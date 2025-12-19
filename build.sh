#!/bin/bash
set -euo pipefail
clear
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
        | sed "1i const char* $varname =" \
        | sed '$a ;' \
        > "$outfile"
}

# Shaders and their C variable names
gen_header ./Shaders/ssr.compute                ssr_computeShader
gen_header ./Shaders/voxels.compute             voxelUpdate_computeShader
gen_header ./Shaders/shadowmaps_clear.compute   shadowmaps_clear_computeShader
gen_header ./Shaders/chunk_vert.glsl            vertexShaderSource
gen_header ./Shaders/chunk_frag.glsl            fragmentShaderTraditional
gen_header ./Shaders/text_vert.glsl             textVertexShaderSource
gen_header ./Shaders/text_frag.glsl             textFragmentShaderSource
gen_header ./Shaders/composite_vert.glsl        quadVertexShaderSource
gen_header ./Shaders/composite_frag.glsl        quadFragmentShaderSource
gen_header ./Shaders/shadowmap_vert.glsl        shadowmapVertexShaderSource
gen_header ./Shaders/shadowmap_frag.glsl        shadowmapFragmentShaderSource

CC=gcc
export CC=$CC
CFLAGS="-pipe -fno-ident -fno-asynchronous-unwind-tables -fstack-protector-all -fdata-sections -ffunction-sections -g0 -fstrict-aliasing -Wstrict-aliasing=2 -fno-common -Walloca -Wstack-usage=262144 -Wvla -std=c11 -Wall -Wextra -Wdouble-promotion -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS -Wformat=2 -Wshadow -Wnull-dereference -Wsuggest-attribute=pure -Wstrict-prototypes -Wno-overlength-strings -Og -D_GNU_SOURCE"
# LDFLAGS="-fuse-ld=mold -Wl,--gc-sections -flto -L./External -l:libz.a -static-libstdc++ -static-libgcc -l:libglfw3.5.a -l:libminiaudio.0.11.22.a -ffast-math -lGL -lfontconfig" # Uncomment for compiling with FONT_GEN set to regenerate font atlases
LDFLAGS="  -fuse-ld=mold -Wl,--gc-sections -flto -L./External -l:libz.a -static-libstdc++ -static-libgcc -l:libglfw3.5.a -l:libminiaudio.0.11.22.a -ffast-math -lGL"
SOURCES="voxen.c data_parser.c physics.c matvecquat.c audio.c helpers.c console.c event.c hardware.c data_text.c entity.c data_textures.c data_fonts.c glad.c os.c todo.c"
export CFLAGS=$CFLAGS
export TEMP_DIR=temp_build
printf "%s\n" $SOURCES | xargs -P12 -I{} $CC -c {} $CFLAGS -o "$TEMP_DIR"/{}.o
cp ./External/assimp/*.o "$TEMP_DIR"/
mold -run g++ "$TEMP_DIR"/*.o -o voxen $LDFLAGS #g++ for linker to fix compile issues manually linking in Assimp .o files
link_status=$?
if [ $link_status -ne 0 ]; then
    echo "ERROR: Linking failed."
    exit 1
fi

rm -f "$TEMP_DIR"/*.o ./Shaders/*.h
build_end=$(now_ms)
total_build_time=$((build_end - shader_start))
echo "Build completed in ${total_build_time} ms"
if [ $# -eq 0 ] || [ "$1" != "ci" ]; then
    ./voxen
fi
