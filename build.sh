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
TEMP_DIR=temp_build
TEMP_DIRGC=temp_build_gc
rm -f "$TEMP_DIR"/*.o ./Shaders/*.h "$TEMP_DIRGC"/*.o
export TMPDIR=/dev/shm
mkdir -p $TEMP_DIR
now_ms() { date +%s%3N; }
shader_start=$(now_ms)
if [ $# -eq 0 ] || [ "${1:-}" != "ci" ]; then
    CSV="build_history.csv"
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
gen_header() {
    local infile="$1"
    local varname="$2"
    local outfile="$infile.h"
    sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' "$infile" \
        | sed "1i static const char* $varname =" \
        | sed '$a ;' \
        > "$outfile"
}

# Shaders and their C variable names
gen_header ./Shaders/ssr.compute                ssrComputeSrc
gen_header ./Shaders/voxels.compute             voxelUpdateComputeSrc
gen_header ./Shaders/shadowmaps_clear.compute   shadowmapsClearComputeSrc
gen_header ./Shaders/depth_prepass_vert.glsl    depthPrepassVertSrc
gen_header ./Shaders/depth_prepass.glsl         depthPrepassFragSrc
gen_header ./Shaders/debugunlit_vert.glsl       debugUnlitVertSrc
gen_header ./Shaders/debugunlit_frag.glsl       debugUnlitFragSrc
gen_header ./Shaders/chunk_vert.glsl            vertSrc
gen_header ./Shaders/chunk_frag.glsl            fragSrc
gen_header ./Shaders/text_vert.glsl             textVertSrc
gen_header ./Shaders/text_frag.glsl             textFragSrc
gen_header ./Shaders/composite_vert.glsl        quadVertSrc
gen_header ./Shaders/composite_frag.glsl        quadFragSrc
gen_header ./Shaders/shadowmap_vert.glsl        shadowmapVertSrc
gen_header ./Shaders/shadowmap_frag.glsl        shadowmapFragSrc
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
#include "shadowmap_vert.glsl.h"
#include "shadowmap_frag.glsl.h"
#include "composite_vert.glsl.h"
#include "composite_frag.glsl.h"
#include "ssr.compute.h"
#include "voxels.compute.h"
#include "shadowmaps_clear.compute.h"
#include "bluenoise64.cginc"
EOF

ZIG_LIBS="-L/usr/lib/x86_64-linux-gnu -L/usr/lib64"
LINUX_CC="zig cc -target x86_64-linux-gnu.2.7"
WINDOWS_CC="zig cc -target x86_64-windows-gnu -Wl,--stack,8388608"
COMMON_CFLAGS="-fno-exceptions -fno-stack-protector -fno-asynchronous-unwind-tables -fno-unwind-tables -Wno-format-nonliteral \
               -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -fvisibility=hidden -pipe -fno-ident -fdata-sections -Wno-int-to-void-pointer-cast \
               -ffunction-sections -ffast-math -std=c11 -Wall -Wextra -Wno-implicit-fallthrough -fdeclspec \
               -fomit-frame-pointer -g1 -fstrict-aliasing -fcommon -Walloca -DMA_USE_STDINT \
               -Wformat=2 -Wnull-dereference -Wstrict-prototypes -Wno-overlength-strings -fno-math-errno -fno-sanitize=all \
               -fno-trapping-math -fmerge-all-constants -m64 -O3 -march=haswell -fstack-usage"
COMMON_LFLAGS="-Wl,--sort-common -Wl,-z,now -Wl,-z,relro $ZIG_LIBS"

# Game Code Build 14.4kb
if [ "$PLATFORM" = "windows" ]; then
    CC=$WINDOWS_CC
    LINKERGC=$CC
    CFLAGSGC="-D_WIN32 $COMMON_CFLAGS -mno-stack-arg-probe"
    LDFLAGSGC="$COMMON_LFLAGS -Wl,--allow-shlib-undefined -Wl,--entry,DllMainCRTStartup -Wl,--subsystem,windows -Wl,--no-entry"
    BINARY_NAMEGC="Citadel.dll"
else
    CC=$LINUX_CC
    LINKERGC=$CC
    CFLAGSGC="$COMMON_CFLAGS -fno-plt -fno-semantic-interposition"
    LDFLAGSGC="$COMMON_LFLAGS -target x86_64-linux-gnu.2.7 -nostdlib"
    BINARY_NAMEGC="Citadel.so"
fi

export CCGC=$CC
export CFLAGSGC=$CFLAGSGC
SOURCESGC="init.c modinput.c modphysics.c ai.c biomonitor.c weapons.c music.c modaudio.c"
export TEMP_DIRGC=temp_build_gc
export SCRIPT_DIR="./Scripts"
printf "%s\n" $SOURCESGC | xargs -P12 -I{} $CCGC -c $SCRIPT_DIR/{} $CFLAGSGC -I. -nostdinc -fPIC -ffreestanding -fno-builtin -Wshadow -o "$TEMP_DIRGC"/{}.o
$LINKERGC "$TEMP_DIRGC"/*.o $LDFLAGSGC -Wl,-soname,$BINARY_NAMEGC -shared -o $BINARY_NAMEGC
link_status=$?
if [ $link_status -ne 0 ]; then
    echo "ERROR: Linking failed."
    exit 1
else
    echo "Built mod gamecode successfully."
fi

# Engine Build 214.5kb
if [ "$PLATFORM" = "windows" ]; then
    CC=$WINDOWS_CC
    LINKER=$CC
    CFLAGS="-D_WIN32 $COMMON_CFLAGS -mno-stack-arg-probe"
    LDFLAGS="$COMMON_LFLAGS -L. -lopengl32 -lglfw3"
    BINARY_NAME="voxen.exe"
else
    CC=$LINUX_CC
    LINKER=$CC
    CFLAGS="$COMMON_CFLAGS -fno-plt -fno-semantic-interposition"
    LDFLAGS="$COMMON_LFLAGS -target x86_64-linux-gnu.2.7 -pthread -lglfw -lGL "
    BINARY_NAME="voxen"
fi

export CC=$CC
export CFLAGS=$CFLAGS
SOURCES="voxen.c physics.c helpers.c audio.c animation.c console.c level.c data_parser.c \
         data_text.c data_fonts.c data_models.c dynamic_culling.c data_textures.c glad.c \
         input.c miniaudio.c menu.c"

export TEMP_DIR=temp_build
printf "%s\n" $SOURCES | xargs -P12 -I{} $CC -c {} $CFLAGS -o "$TEMP_DIR"/{}.o
$LINKER "$TEMP_DIR"/*.o $LDFLAGS -rdynamic -lm -o $BINARY_NAME
link_status=$?
if [ $link_status -ne 0 ]; then
    echo "ERROR: Linking failed."
    exit 1
fi

build_end=$(now_ms)
total_build_time=$((build_end - shader_start))
echo "Built engine and mod in ${total_build_time} ms"
if ! $IS_CI; then
    case "$PLATFORM" in
        windows)  strip --strip-all voxen.exe; upx -qqq --best --lzma ./voxen.exe; wine ./voxen.exe ;;
#         *)        strip --strip-all --strip-unneeded ./voxen; upx -qqq --best --lzma ./voxen; ./voxen ;;   # linux
        *)        ./voxen ;;   # linux
    esac
    rm -f "$TEMP_DIR"/*.o ./Shaders/*.h "$TEMP_DIRGC"/*.o #Cleanup after quitting. Doesn't affect build timer.  Gives me a chance to trivially copy out .o files if I want.
fi
