#!/bin/bash
clear
TEMP_DIR=temp_build
mkdir -p $TEMP_DIR
rm -f "$TEMP_DIR"/*.o

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

# List shaders and their C variable names
gen_header ./Shaders/ssr.compute                ssr_computeShader
gen_header ./Shaders/chunk_vert.glsl            vertexShaderSource
gen_header ./Shaders/chunk_frag.glsl            fragmentShaderTraditional
gen_header ./Shaders/text_vert.glsl             textVertexShaderSource
gen_header ./Shaders/text_frag.glsl             textFragmentShaderSource
gen_header ./Shaders/composite_vert.glsl        quadVertexShaderSource
gen_header ./Shaders/composite_frag.glsl        quadFragmentShaderSource
gen_header ./Shaders/shadowmap_vert.glsl        shadowmapVertexShaderSource
gen_header ./Shaders/shadowmap_frag.glsl        shadowmapFragmentShaderSource
shader_end=$(now_ms)
build_start=$(now_ms)
echo "Shaders converted to string constants in $((shader_end - shader_start)) ms"

CC=gcc
CFLAGS="-fopenmp -std=c11 -Wall -Wextra -O3 -D_POSIX_C_SOURCE=199309L"
LDFLAGS="-L./External -l:libassimp.6.0.2.a -lz -lstdc++ -static-libstdc++ -l:libglfw3.4.a -l:libminiaudio.0.11.22.a -lGLEW -lGL -lm -lrt -lenet -lpthread -fopenmp"
SOURCES="voxen.c data_textures.c data_parser.c physics.c dynamic_culling.c citadel_playermovement.c"

# Compile sources in parallel
pids=()
for src in $SOURCES; do
    obj="$TEMP_DIR/${src%.c}.o"
    $CC -c "$src" $CFLAGS -o "$obj" &
    pids+=($!)
done

# Wait for all compilations to complete
for pid in "${pids[@]}"; do
    wait $pid
    if [ $? -ne 0 ]; then
        echo "ERROR: Compilation failed for one or more source files."
        rm -f ./Shaders/*.h
        rm -f "$TEMP_DIR"/*.o
        exit 1
    fi
done

# Link object files
link_start=$(now_ms)
$CC $TEMP_DIR/*.o -o voxen $LDFLAGS
if [ $? -ne 0 ]; then
    echo "ERROR: Linking failed."
    exit 1
fi
link_end=$(now_ms)
echo "Linking completed in $((link_end - link_start)) ms"
rm -f "$TEMP_DIR"/*.o
rm -f ./Shaders/*.h
build_end=$(now_ms)
echo "Build completed in $((build_end - build_start)) ms"
if [ $# -eq 0 ] || [ "$1" != "ci" ]; then
    ./voxen
fi
