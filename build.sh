#!/bin/bash
set -euo pipefail
clear
TEMP_DIR=temp_build
export TMPDIR=/dev/shm
mkdir -p $TEMP_DIR
rm -f "$TEMP_DIR"/*.o ./Shaders/*.h

echo "Compiling voxen..."
now_ms() { date +%s%3N; }
shader_start=$(now_ms)

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
gen_header ./Shaders/shadowmaps_clear.compute   shadowmaps_clear_computeShader
gen_header ./Shaders/chunk_vert.glsl            vertexShaderSource
gen_header ./Shaders/chunk_frag.glsl            fragmentShaderTraditional
gen_header ./Shaders/text_vert.glsl             textVertexShaderSource
gen_header ./Shaders/text_frag.glsl             textFragmentShaderSource
gen_header ./Shaders/composite_vert.glsl        quadVertexShaderSource
gen_header ./Shaders/composite_frag.glsl        quadFragmentShaderSource
gen_header ./Shaders/shadowmap_vert.glsl        shadowmapVertexShaderSource
gen_header ./Shaders/shadowmap_frag.glsl        shadowmapFragmentShaderSource

export CC="gcc"
CC=gcc
export LD=mold
CFLAGS="-flto -pipe -fno-ident -fno-asynchronous-unwind-tables -fstack-protector-all -fdata-sections -ffunction-sections -g0  -std=c11 -Wall -Wextra -Og"
# LDFLAGS="-fuse-ld=mold -Wl,--gc-sections -flto -L./External -l:libz.a -static-libstdc++ -static-libgcc -l:libglfw3.5.a -l:libminiaudio.0.11.22.a -ffast-math -lGL -lfontconfig" # Uncomment for compiling with FONT_GEN set to regenerate font atlases
LDFLAGS="  -fuse-ld=mold -Wl,--gc-sections -flto -L./External -l:libz.a -static-libstdc++ -static-libgcc -l:libglfw3.5.a -l:libminiaudio.0.11.22.a -ffast-math -lGL"
SOURCES="voxen.c data_parser.c physics.c matvecquat.c audio.c helpers.c console.c event.c data_text.c entity.c data_textures.c data_models.c data_fonts.c glad.c"
export CC=gcc
export CFLAGS="-pipe -fno-ident -fno-asynchronous-unwind-tables -fstack-protector-all -g0 -std=c11 -Wall -Wextra -Og"
export TEMP_DIR=temp_build
printf "%s\n" $SOURCES | xargs -P12 -I{} $CC -c {} $CFLAGS -o "$TEMP_DIR"/{}.o
cp ./External/assimp/*.o "$TEMP_DIR"/
g++ "$TEMP_DIR"/*.o -o voxen $LDFLAGS #g++ for linker to fix compile issues manually linking in Assimp .o files
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
