#!/bin/bash
set -euo pipefail
PLATFORM="linux"
IS_CI=false
while [[ $# -gt 0 ]]; do
    case "$1" in
        win|windows)
            PLATFORM="windows"
            shift
            ;;
        mac|macintosh)
            PLATFORM="mac"
            shift
            ;;
        android)
            PLATFORM="android"
            shift
            ;;
        ci)
            IS_CI=true
            shift
            ;;
        *)
            echo "Unknown argument: $1" >&2
            echo "Usage: $0 [platform] [ci]"
            echo "  platform: linux (default), win/windows, mac, android"
            echo "  ci:       suppress clear & auto-run"
            exit 1
            ;;
    esac
done

if ! $IS_CI; then
    clear
fi

need_rebuild() {
    local src="$1"
    local obj="$2"
    [[ ! -f "$obj" ]] && return 0
    [[ "$src" -nt "$obj" ]] && return 0
    return 1
}

TEMP_DIR=temp_build
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

LINUX_CC="gcc"
WINDOWS_CC="x86_64-w64-mingw32-gcc"
ANDROID_CC="aarch64-linux-android24-clang"
MAC_CC="gcc"
COMMON_CFLAGS=" -I./External/ -pipe -fno-ident -fdata-sections -ffunction-sections -g1 -std=c11 -Wall -Wextra \
               -fno-omit-frame-pointer -fstrict-aliasing -fno-common -Walloca -Wstack-usage=262144 \
               -Wdouble-promotion -Wformat=2 -Wnull-dereference -Wstrict-prototypes -Wno-overlength-strings \
               -Werror=implicit-function-declaration -Og -D_GNU_SOURCE"

if [ "$PLATFORM" = "windows" ]; then
    CC=$WINDOWS_CC
    LINKER=$CC
    CFLAGS="-D_WIN32 $COMMON_CFLAGS -fno-ident -fno-asynchronous-unwind-tables -mno-stack-arg-probe"
    LDFLAGS="-L./ -L./External/ -L./External/Windows -l:assimp-vc143-mt.dll -lopengl32 -lm -l:glfw3.dll -l:libminiaudio.dll.a -static-libgcc -flto=auto"
    OBJ_DIR="./External/Windows"
    GLAD_OBJ="${OBJ_DIR}/glad.o"
    BINARY_NAME="voxen.exe"
elif [ "$PLATFORM" = "mac" ]; then
    CC=$LINUX_CC
    LINKER=$LINUX_CC
    CFLAGS="-D__APPLE__ $COMMON_CFLAGS"
    LDFLAGS="-L./External/ -L./External/Mac -lassimp -lm -l:libglfw3.a -l:libminiaudio.0.11.22.a -lGL"
    OBJ_DIR="./External/Mac"
    GLAD_OBJ="${OBJ_DIR}/glad.o"
    BINARY_NAME="voxen_mac"
elif [ "$PLATFORM" = "android" ]; then
    CC=$ANDROID_CC
    LINKER=$CC
    CFLAGS="-D__ANDROID__ -fPIC $COMMON_CFLAGS"
    LDFLAGS="-L./External/ -L./External/Android -landroid -llog -lGLESv3 -lEGL -lm"
    OBJ_DIR="./External/Android"
    GLAD_OBJ="${OBJ_DIR}/glad.o"
    BINARY_NAME="voxen_android"
else
    CC=$LINUX_CC
    LINKER="mold -run gcc"
    CFLAGS="-march=haswell -mtune=haswell $COMMON_CFLAGS"
    LDFLAGS="-flto -Wl,--gc-sections -L./External/Linux -lassimp -lm -l:libglfw3.5.a -l:libminiaudio.0.11.22.a -lGL"
    OBJ_DIR="./External/Linux"
    GLAD_OBJ="${OBJ_DIR}/glad/glad.o"
    BINARY_NAME="voxen"
fi

# Build infrequent fliers
if need_rebuild "./External/glad/glad.c" "$GLAD_OBJ"; then
    echo "Rebuilding glad.o for ${PLATFORM}..."
    mkdir -p "$(dirname "$GLAD_OBJ")"
    $CC -c "./External/glad/glad.c" $CFLAGS -o "$GLAD_OBJ"
fi

cp $GLAD_OBJ "$TEMP_DIR/glad.o"
export CC=$CC
export CFLAGS=$CFLAGS
SOURCES="voxen.c physics.c helpers.c init.c menu.c animation.c console.c biomonitor.c level.c data_parser.c data_text.c data_fonts.c data_models.c dynamic_culling.c todo.c input.c"
export TEMP_DIR=temp_build
printf "%s\n" $SOURCES | xargs -P12 -I{} $CC -c {} $CFLAGS -o "$TEMP_DIR"/{}.o

$LINKER "$TEMP_DIR"/*.o $LDFLAGS -o $BINARY_NAME $LDFLAGS
link_status=$?
if [ $link_status -ne 0 ]; then
    echo "ERROR: Linking failed."
    exit 1
fi

build_end=$(now_ms)
total_build_time=$((build_end - shader_start))
echo "Build completed in ${total_build_time} ms"
if ! $IS_CI; then
    case "$PLATFORM" in
        windows)  strip --strip-all voxen.exe; upx -qqq --best --lzma ./voxen.exe; WINEPATH="External/Windows" wine ./voxen.exe ;;   # or wine voxen.exe if testing on linux
        mac)      ./voxen.app ;;
        android)  java -jar bundletool.jar build-apks --bundle=voxen.aab --output=voxen.app;;
        *)        strip --strip-all voxen; upx -qqq --best --lzma ./voxen; ./voxen ;;   # linux
#         *)        ./voxen ;;   # linux
    esac
    rm -f "$TEMP_DIR"/*.o ./Shaders/*.h "$TEMP_DIR"/*.cpp #Cleanup after quitting. Doesn't affect build timer.  Gives me a chance to trivially copy out .o files if I want.
fi
