#!/bin/bash
set -euo pipefail; PLATFORM="linux"; IS_CI=false
while [[ $# -gt 0 ]]; do
    case "$1" in
        win|windows) PLATFORM="windows" ;;
        ci) IS_CI=true ;;
        *) echo "Unknown: $1" >&2; exit 1 ;;
    esac
    shift
done
$IS_CI || clear
mkdir -p ./temp_build
TEMP_DIR=temp_build
rm -f ./Shaders/*.h "$TEMP_DIR"/*.o
export TMPDIR=/dev/shm
now_ms() { date +%s%3N; }
shader_start=$(now_ms)
if [ $# -eq 0 ] || [ "${1:-}" != "ci" ]; then
    CSV="builds.csv"
    TODAY=$(date '+%Y-%m-%d')
    if [ ! -f "$CSV" ]; then
        printf "date,builds_today\n" > "$CSV"
    fi
    sed -i 's/\r$//' "$CSV" 2>/dev/null || true
    TODAY_COUNT=0
    if grep -q "^$TODAY," "$CSV" 2>/dev/null; then
        TODAY_COUNT=$(grep "^$TODAY," "$CSV" | cut -d',' -f2 | tr -cd '0-9')
        TODAY_COUNT=${TODAY_COUNT:-0}
    fi
    TODAY_COUNT=$((TODAY_COUNT + 1))
    grep -v "^$TODAY," "$CSV" > "${CSV}.tmp" 2>/dev/null || true
    echo "$TODAY,$TODAY_COUNT" >> "${CSV}.tmp"
    mv "${CSV}.tmp" "$CSV"
    echo "Compiling voxen, total iterations today $TODAY_COUNT ($TODAY)..."
else
    echo "Compiling voxen (CI build)..."
fi

# Convert shaders into string headers
gh() {
    local VER="#version 430 core\\n"
    if [[ "$PLATFORM" == "android" ]]; then
        VER="#version 310 es\\nprecision mediump float;\\nprecision mediump sampler2D;\\n"
    fi
    (echo -e "$VER"; cat "$1") | sed 's|//.*||g' | awk '
    /^\s*#/ {gsub(/^\s+|\s+$/,"");printf"%s\\n",$0;next}
    {gsub(/\/\/.*/,"");gsub(/[ \t\r\n]+/," ");gsub(/^[ \t]+|[ \t]+$/,"");if(NF==0)next
    gsub(/ *\* */,"*");gsub(/ *\/ */,"/");gsub(/ *\+ */,"+");gsub(/ *- */,"-")
    gsub(/ *< */,"<");gsub(/ *> */ ,">");gsub(/ *== */,"==");gsub(/ *!= */,"!=")
    gsub(/ *<= */,"<=");gsub(/ *>= */ ,">=");gsub(/ *= */,"=");gsub(/ *, */ ,",")
    gsub(/ *; */ ,";");gsub(/ *\{ */,"{");gsub(/ *\} */,"}");gsub(/ *\(/ ,"(")
    gsub(/ *\) */ ,")");gsub(/ *\[/ ,"[" );gsub(/ *\] */ ,"]");gsub(/ *\. */ ,".")
    printf"%s",$0} END{print""}' | tr -d '\n' | sed 's/"/\\"/g' | \
    sed "s|^|static const char* $2 = \"|;s|$|\";|" > "${1%}.h"
}

# Shaders and their C variable names
gh ./Shaders/ssr.compute              ssrCSSrc
gh ./Shaders/voxels.compute           voxUpdCSSrc
gh ./Shaders/shadowmaps_clear.compute shadClearCSSrc
gh ./Shaders/depth_prepass_vert.glsl  depthPrepassVertSrc
gh ./Shaders/depth_prepass.glsl       depthPrepassFragSrc
gh ./Shaders/debugunlit_vert.glsl     debugUnlitVertSrc
gh ./Shaders/debugunlit_frag.glsl     debugUnlitFragSrc
gh ./Shaders/chunk_vert.glsl          vertSrc
gh ./Shaders/chunk_frag.glsl          fragSrc
gh ./Shaders/ui_vert.glsl             vertUISrc
gh ./Shaders/ui_frag.glsl             fragUISrc
gh ./Shaders/text_vert.glsl           textVertSrc
gh ./Shaders/text_frag.glsl           textFragSrc
gh ./Shaders/composite_vert.glsl      quadVertSrc
gh ./Shaders/composite_frag.glsl      quadFragSrc
gh ./Shaders/shadowmap_vert.glsl      shadowmapVertSrc
gh ./Shaders/shadowmap_frag.glsl      shadowmapFragSrc
gh ./Shaders/particle_vert.glsl       particleVertSrc
gh ./Shaders/particle_frag.glsl       particleFragSrc
gh ./Shaders/trail_vert.glsl          trailVertSrc
gh ./Shaders/trail_frag.glsl          trailFragSrc
cat > Shaders/shaders.h <<'EOF'
#pragma once
#include "text_vert.glsl.h"
#include "text_frag.glsl.h"
#include "depth_prepass_vert.glsl.h"
#include "depth_prepass.glsl.h"
#include "debugunlit_vert.glsl.h"
#include "debugunlit_frag.glsl.h"
#include "chunk_vert.glsl.h"
#include "chunk_frag.glsl.h"
#include "ui_vert.glsl.h"
#include "ui_frag.glsl.h"
#include "shadowmap_vert.glsl.h"
#include "shadowmap_frag.glsl.h"
#include "composite_vert.glsl.h"
#include "composite_frag.glsl.h"
#include "particle_vert.glsl.h"
#include "particle_frag.glsl.h"
#include "trail_vert.glsl.h"
#include "trail_frag.glsl.h"
#include "ssr.compute.h"
#include "voxels.compute.h"
#include "shadowmaps_clear.compute.h"
EOF
LINUX_CC="zig cc"
WINDOWS_CC="zig cc -target x86_64-windows-gnu -Wframe-larger-than=65536"
COMMON_CFLAGS="-ferror-limit=500 -fno-stack-protector -fno-unwind-tables -Wno-format-nonliteral -fvisibility=hidden -pipe -fno-ident -fdata-sections -Wno-int-to-void-pointer-cast \
               -Wshadow -ffunction-sections -ffast-math -std=c11 -Wall -Wextra -Wno-implicit-fallthrough -Wno-switch -fdeclspec -fomit-frame-pointer -g0 -fvisibility=hidden \
               -Wno-overlength-strings -fno-math-errno -fno-sanitize=all -m64 -Os -march=haswell -mf16c -mavx -Wbool-conversion -Wno-empty-body -nostdinc -ftrivial-auto-var-init=zero"
COMMON_LFLAGS="-Wl,-z,relro,-z,now,--gc-sections,--as-needed,--build-id=none"
if [ "$PLATFORM" = "windows" ]; then
    CC=$WINDOWS_CC
    LINKER=$CC
    CFLAGS="-D_WIN32 $COMMON_CFLAGS -fno-strict-aliasing -Wl,-Bstatic -lmingw32 -lmingwex"
    LDFLAGS="$COMMON_LFLAGS -L. -lgdi32 -lole32 -static-libgcc"
    BINARY_NAME="voxen.exe"
else
    CC=$LINUX_CC
    LINKER=$CC
    CFLAGS="$COMMON_CFLAGS -fstrict-aliasing -fmerge-all-constants -fno-plt -fno-semantic-interposition -fno-builtin"
    LDFLAGS="$COMMON_LFLAGS -target x86_64-linux-gnu.2.7 -ldl"
    BINARY_NAME="voxen"
fi
export CC=$CC
export CFLAGS=$CFLAGS
SOURCES="voxen.c physics.c entity.c lib.c citadel.c ai.c weapons.c text.c audio.c textures.c models.c biomonitor.c culling.c particles.c" #synth.c is in audio.c
SIZEOPT="models.c text.c textures.c lib.c audio.c"
export TEMP_DIR=temp_build
printf "%s\n" $SOURCES | xargs -P12 -I{} sh -c "F=\$(echo '$SIZEOPT' | tr ' ' '\\n' | grep -qx \$(basename {}) && echo -Oz || echo ''); $CC -c {} $CFLAGS \$F -o $TEMP_DIR/\$(basename {}).o"
$LINKER "$TEMP_DIR"/*.o $LDFLAGS -o $BINARY_NAME
build_end=$(now_ms)
total_build_time=$((build_end - shader_start))
echo "Built engine as game in ${total_build_time} ms"
if ! $IS_CI; then
    case "$PLATFORM" in
        windows)  strip --strip-all voxen.exe; upx -qqq --best --lzma ./voxen.exe; wine ./voxen.exe ;;
#         windows)  wine ./voxen.exe ;;
#         *)        strip --strip-all --strip-unneeded ./voxen; upx -qqq --best --lzma ./voxen; ./voxen ;;   # linux
        *)        ./voxen ;;   # linux
    esac
    rm -f ./Shaders/*.h ./voxen.upx ./voxen.pdb #Cleanup after quitting. Doesn't affect build timer.  Gives me a chance to trivially copy out .o files if I want.
    rm -r ./temp_build
fi
