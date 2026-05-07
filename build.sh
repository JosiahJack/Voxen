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
TEMP_DIRGC=temp_build_gc
rm -f ./Shaders/*.h "$TEMP_DIRGC"/*.o
export TMPDIR=/dev/shm
mkdir -p $TEMP_DIRGC
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
    sed 's|//.*||g' "$1" | awk '
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
gh ./Shaders/ssr.compute              ssrComputeSrc
gh ./Shaders/voxels.compute           voxelUpdateComputeSrc
gh ./Shaders/shadowmaps_clear.compute shadowmapsClearComputeSrc
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
#include "ssr.compute.h"
#include "voxels.compute.h"
#include "shadowmaps_clear.compute.h"
EOF

ZIG_LIBS="-L/usr/lib/x86_64-linux-gnu -L/usr/lib64"
LINUX_CC="zig cc -target x86_64-linux-gnu.2.7"
WINDOWS_CC="zig cc -target x86_64-windows-gnu -Wl,--stack,8388608"
COMMON_CFLAGS="-ferror-limit=500 -fno-exceptions -fno-stack-protector -fno-asynchronous-unwind-tables -fno-unwind-tables -Wno-format-nonliteral \
               -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -fvisibility=hidden -pipe -fno-ident -fdata-sections -Wno-int-to-void-pointer-cast \
               -ffunction-sections -ffast-math -std=c11 -Wall -Wextra -Wno-implicit-fallthrough -fdeclspec -fno-rtti \
               -fomit-frame-pointer -g0 -fstrict-aliasing -fcommon -Walloca \
               -Wformat=2 -Wnull-dereference -Wstrict-prototypes -Wno-overlength-strings -fno-math-errno -fno-sanitize=all \
               -fno-trapping-math -fmerge-all-constants -m64 -Os -march=haswell -fstack-usage"
COMMON_LFLAGS="-Wl,-z,now -Wl,-z,relro -Wl,--gc-sections -Wl,--as-needed -Wl,--strip-all -Wl,-z,norelro -Wl,--build-id=none $ZIG_LIBS"
if [ "$PLATFORM" = "windows" ]; then
    CC=$WINDOWS_CC
    LINKER=$CC
    CFLAGS="-D_WIN32 $COMMON_CFLAGS -mno-stack-arg-probe"
    CFLAGSGC="-D_WIN32 $COMMON_CFLAGS -mno-stack-arg-probe"
    LDFLAGS="$COMMON_LFLAGS -L. -lgdi32 -lopengl32 -lole32 -Wl,--out-implib=voxen.lib -Xlinker /pdb:none"
    LDFLAGSGC="$COMMON_LFLAGS -Wl,--allow-shlib-undefined -Wl,--entry,DllMainCRTStartup -Wl,--subsystem,windows -Wl,--no-entry -L. -lvoxen -Xlinker /pdb:none"
    BINARY_NAME="voxen.exe"
    BINARY_NAMEGC="Citadel.dll"
else
    CC=$LINUX_CC
    LINKER=$CC
    CFLAGS="$COMMON_CFLAGS -fno-plt -fno-semantic-interposition -D_GNU_SOURCE -ffreestanding -fno-builtin"
    CFLAGSGC="$COMMON_CFLAGS -fno-plt -fno-semantic-interposition"
    LDFLAGS="$COMMON_LFLAGS -target x86_64-linux-gnu.2.7 -ldl"
    LDFLAGSGC="$COMMON_LFLAGS -target x86_64-linux-gnu.2.7 -nostdlib"
    BINARY_NAME="voxen"
    BINARY_NAMEGC="Citadel.so"
fi

# Engine Build
$CC voxen.c $CFLAGS $LDFLAGS -rdynamic -o $BINARY_NAME

# Game Code Build
export CCGC=$CC
export CFLAGSGC=$CFLAGSGC
SOURCESGC="animation.c ai.c biomonitor.c weapons.c music.c modaudio.c citadel.c entity.c"
export TEMP_DIRGC=temp_build_gc
export SCRIPT_DIR="./Scripts"
printf "%s\n" $SOURCESGC | xargs -P12 -I{} $CCGC -c $SCRIPT_DIR/{} $CFLAGSGC -I. -nostdinc -fPIC -ffreestanding -fno-builtin -Wshadow -o "$TEMP_DIRGC"/{}.o
$LINKER "$TEMP_DIRGC"/*.o $LDFLAGSGC -Wl,-soname,$BINARY_NAMEGC -shared -o $BINARY_NAMEGC
build_end=$(now_ms)
total_build_time=$((build_end - shader_start))
echo "Built engine and mod in ${total_build_time} ms"
if ! $IS_CI; then
    case "$PLATFORM" in
        windows)  strip --strip-all voxen.exe; upx -qqq --best --lzma ./voxen.exe; wine ./voxen.exe ;;
#         windows)  wine ./voxen.exe ;;
        *)        strip --strip-all --strip-unneeded ./voxen; upx -qqq --best --lzma ./voxen; ./voxen ;;   # linux
#         *)        strip --strip-all --strip-unneeded ./voxen; ./voxen ;;   # linux Alternate build methods to be able to look at symbols and debugging
#         *)        ./voxen ;;   # linux
    esac
    rm -f ./Shaders/*.h "$TEMP_DIRGC"/*.o ./voxen.upx ./*.lib #Cleanup after quitting. Doesn't affect build timer.  Gives me a chance to trivially copy out .o files if I want.
fi
