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
gen_header ./Shaders/ssr.compute                ssr_computeShader
gen_header ./Shaders/voxels.compute             voxelUpdate_computeShader
gen_header ./Shaders/shadowmaps_clear.compute   shadowmaps_clear_computeShader
gen_header ./Shaders/debugunlit_vert.glsl       debugUnlitVertexShaderSource
gen_header ./Shaders/debugunlit_frag.glsl       debugUnlitFragmentShaderSource
gen_header ./Shaders/chunk_vert.glsl            vertexShaderSource
gen_header ./Shaders/chunk_frag.glsl            fragmentShaderTraditional
gen_header ./Shaders/text_vert.glsl             textVertexShaderSource
gen_header ./Shaders/text_frag.glsl             textFragmentShaderSource
gen_header ./Shaders/composite_vert.glsl        quadVertexShaderSource
gen_header ./Shaders/composite_frag.glsl        quadFragmentShaderSource
gen_header ./Shaders/shadowmap_vert.glsl        shadowmapVertexShaderSource
gen_header ./Shaders/shadowmap_frag.glsl        shadowmapFragmentShaderSource
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
COMMON_CFLAGS="-pipe -fno-ident -fdata-sections -ffunction-sections -g1 -std=c11 -Wall -Wextra -fno-omit-frame-pointer \
               -fstrict-aliasing -Wstrict-aliasing=2 -fno-common -Walloca -Wstack-usage=262144 -Wvla -Wdouble-promotion \
               -Wformat=2 -Wshadow -Wnull-dereference -Wstrict-prototypes -Wno-overlength-strings -Werror=implicit-function-declaration \
               -Og -D_GNU_SOURCE"

if [ "$PLATFORM" = "windows" ]; then
    CC=$WINDOWS_CC
    LINKER=$CC
    CFLAGS="-D_WIN32 $COMMON_CFLAGS"
    LDFLAGS="-L./External/ -L./External/Windows -lassimp -lglfw3 -lgdi32 -lopengl32 -lm -l:libglfw3.5.dll -l:libminiaudio.0.11.22.dll"
    OBJ_DIR="./External/Windows"
    GLAD_OBJ="${OBJ_DIR}/glad.o"
    BINARY_NAME="voxen.exe"
elif [ "$PLATFORM" = "mac" ]; then
    CC=$LINUX_CC
    LINKER="mold -run gcc"
    CFLAGS="-D__APPLE__ $COMMON_CFLAGS"
    LDFLAGS="-Wl,--gc-sections -L./External/ -L./External/Mac -lassimp -lm -l:libglfw3.5.a -l:libminiaudio.0.11.22.a -lGL -lfontconfig"
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
    LDFLAGS="-Wl,--gc-sections -L./External/ -L./External/Linux -lassimp -lm -l:libglfw3.5.a -l:libminiaudio.0.11.22.a -lGL -lfontconfig"
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
SOURCES="voxen.c matvecquat.c physics.c helpers.c console.c event.c level.c data_parser.c data_text.c data_fonts.c os.c todo.c"
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
        windows)  ./"$BINARY_NAME" ;;   # or wine voxen.exe if testing on linux
        mac)      ./"$BINARY_NAME" ;;
        android)  echo "Android binary built — deploy manually to device/emulator" ;;
        *)        ./"$BINARY_NAME" ;;   # linux
    esac
    rm -f "$TEMP_DIR"/*.o ./Shaders/*.h "$TEMP_DIR"/*.cpp #Cleanup after quitting. Doesn't affect build timer.  Gives me a chance to trivially copy out .o files if I want.
fi
