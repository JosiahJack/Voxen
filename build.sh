#!/bin/bash
clear
TEMP_DIR=temp_build
mkdir -p $TEMP_DIR
rm -f "$TEMP_DIR"/*.o

echo "Compiling voxen..."
now_ms() { date +%s%3N; }
# shader_start=$(now_ms)

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
gen_header ./Shaders/shadowmaps_clear.compute   shadowmaps_clear_computeShader
gen_header ./Shaders/chunk_vert.glsl            vertexShaderSource
gen_header ./Shaders/chunk_frag.glsl            fragmentShaderTraditional
gen_header ./Shaders/text_vert.glsl             textVertexShaderSource
gen_header ./Shaders/text_frag.glsl             textFragmentShaderSource
gen_header ./Shaders/composite_vert.glsl        quadVertexShaderSource
gen_header ./Shaders/composite_frag.glsl        quadFragmentShaderSource
gen_header ./Shaders/shadowmap_vert.glsl        shadowmapVertexShaderSource
gen_header ./Shaders/shadowmap_frag.glsl        shadowmapFragmentShaderSource
# shader_end=$(now_ms)
build_start=$(now_ms)
# echo "Shaders converted to string constants in $((shader_end - shader_start)) ms"

CC=gcc
CFLAGS="-fopenmp -std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=199309L"
LDFLAGS="-flto -L./External -l:libassimp.6.0.2.a -lz -lstdc++ -static-libstdc++ -l:libglfw3.4.a -l:libminiaudio.0.11.22.a -lGLEW -lGL -lm -lfontconfig -fopenmp"
SOURCES="voxen.c data_parser.c physics.c data_fonts.c matvecquat.c audio.c helpers.c event.c data_models.c data_textures.c"

# Array to store PIDs and source names
declare -a pids
declare -a sources

# SEQUENTIAL BUILD
#========================================================
# voxenc_time=$(now_ms)
# gcc -c voxen.c $CFLAGS -o "$TEMP_DIR"/voxen.o
# voxenc_end=$(now_ms)
# echo "voxen.c: $((voxenc_end - voxenc_time)) ms"
# 
# data_parserc_time=$(now_ms)
# gcc -c data_parser.c $CFLAGS -o "$TEMP_DIR"/data_parser.o
# data_parserc_end=$(now_ms)
# echo "data_parser.c: $((data_parserc_end - data_parserc_time)) ms"
# 
# data_modelsc_time=$(now_ms)
# gcc -c data_models.c $CFLAGS -o "$TEMP_DIR"/data_models.o
# data_modelsc_end=$(now_ms)
# echo "data_models.c: $((data_modelsc_end - data_modelsc_time)) ms"
# 
# data_texturesc_time=$(now_ms)
# gcc -c data_textures.c $CFLAGS -o "$TEMP_DIR"/data_textures.o
# data_texturesc_end=$(now_ms)
# echo "data_textures.c: $((data_texturesc_end - data_texturesc_time)) ms"
# 
# physicsc_time=$(now_ms)
# gcc -c physics.c $CFLAGS -o "$TEMP_DIR"/physics.o
# physicsc_end=$(now_ms)
# echo "physics.c: $((physicsc_end - physicsc_time)) ms"
# 
# data_fontsc_time=$(now_ms)
# gcc -c data_fonts.c $CFLAGS -o "$TEMP_DIR"/data_fonts.o
# data_fontsc_end=$(now_ms)
# echo "data_fonts.c: $((data_fontsc_end - data_fontsc_time)) ms"
# 
# audioc_time=$(now_ms)
# gcc -c audio.c $CFLAGS -o "$TEMP_DIR"/audio.o
# audioc_end=$(now_ms)
# echo "audio.c: $((audioc_end - audioc_time)) ms"
# 
# helpersc_time=$(now_ms)
# gcc -c helpers.c $CFLAGS -o "$TEMP_DIR"/helpers.o
# helpersc_end=$(now_ms)
# echo "helpers.c: $((helpersc_end - helpersc_time)) ms"
# 
# eventc_time=$(now_ms)
# gcc -c event.c $CFLAGS -o "$TEMP_DIR"/event.o
# eventc_end=$(now_ms)
# echo "event.c: $((eventc_end - eventc_time)) ms"

# matvecquatc_time=$(now_ms)
# gcc -c matvecquat.c $CFLAGS -o "$TEMP_DIR"/matvecquat.o
# matvecquatc_end=$(now_ms)
# echo "matvecquat.c: $((eventc_end - matvecquatc_time)) ms"
#========================================================
# PARALLEL BUILD
#========================================================
for src in $SOURCES; do
    obj="$TEMP_DIR/${src%.c}.o"
    start_time=$(now_ms)
    $CC -c "$src" $CFLAGS -o "$obj" &
    pids+=($!)
    sources+=("$src|$obj|$start_time")
done

# Wait and report per-file time
failed=false

for i in "${!pids[@]}"; do
    pid=${pids[$i]}
    IFS='|' read -r src obj start_time <<< "${sources[$i]}"
    wait "$pid"
    status=$?
    end_time=$(now_ms)
    elapsed=$((end_time - start_time))
    if [ $status -ne 0 ]; then
        echo "ERROR: Compilation failed for $src"
        failed=true
    fi
done

if $failed; then
    echo "ERROR: One or more source files failed to compile."
    rm -f ./Shaders/*.h
    rm -f "$TEMP_DIR"/*.o
    exit 1
fi

#========================================================

# === Linking ===
link_start=$(now_ms)
$CC "$TEMP_DIR"/*.o -o voxen $LDFLAGS
link_status=$?
link_end=$(now_ms)
link_time=$((link_end - link_start))

if [ $link_status -ne 0 ]; then
    echo "ERROR: Linking failed."
    exit 1
fi

# echo "Linking completed in ${link_time} ms"

# === Cleanup ===
rm -f "$TEMP_DIR"/*.o
rm -f ./Shaders/*.h

# === Final Timing ===
build_end=$(now_ms)
total_build_time=$((build_end - build_start))
echo "Build completed in ${total_build_time} ms"

# === Run (unless in CI) ===
if [ $# -eq 0 ] || [ "$1" != "ci" ]; then
    ./voxen
fi
