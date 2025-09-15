#!/bin/bash
clear
TEMP_DIR=temp_build
mkdir -p $TEMP_DIR
rm -f "$TEMP_DIR"/*.o

# Convert shaders into string headers
sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' Shaders/deferred_lighting.compute \
    | sed '1i const char* deferredLighting_computeShader =' \
    | sed '$a ;' \
    > deferred_lighting.compute.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' Shaders/ssr.compute \
    | sed '1i const char* ssr_computeShader =' \
    | sed '$a ;' \
    > ssr.compute.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' Shaders/chunk_vert.glsl \
    | sed '1i const char* vertexShaderSource =' \
    | sed '$a ;' \
    > chunk_vert.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' Shaders/chunk_frag.glsl \
    | sed '1i const char* fragmentShaderTraditional =' \
    | sed '$a ;' \
    > chunk_frag.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' Shaders/text_vert.glsl \
    | sed '1i const char* textVertexShaderSource =' \
    | sed '$a ;' \
    > text_vert.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' Shaders/text_frag.glsl \
    | sed '1i const char* textFragmentShaderSource =' \
    | sed '$a ;' \
    > text_frag.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' Shaders/composite_vert.glsl \
    | sed '1i const char* quadVertexShaderSource =' \
    | sed '$a ;' \
    > composite_vert.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' Shaders/composite_frag.glsl \
    | sed '1i const char* quadFragmentShaderSource =' \
    | sed '$a ;' \
    > composite_frag.glsl.h

CC=gcc
CFLAGS="-fopenmp -std=c11 -Wall -Wextra -O3 -D_POSIX_C_SOURCE=199309L"
MINIAUDIO_CFLAGS="-std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=199309L -DNDEBUG"
LDFLAGS="-L./External -l:libassimp.6.0.2.a -lz -lstdc++ -static-libstdc++ -lSDL2 -lSDL2_ttf -lGLEW -lGL -lm -lrt -lenet -lpthread -fopenmp -s"
SOURCES="voxen.c data_textures.c data_parser.c audio.c dynamic_culling.c miniaudio.c"

echo "Compiling voxen..."

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
        exit 1
    fi
done

# Link object files
$CC $TEMP_DIR/*.o -o voxen $LDFLAGS
if [ $? -ne 0 ]; then
    echo "ERROR: Linking failed."
    exit 1
fi

rm -f "$TEMP_DIR"/*.o
rm -f ./deferred_lighting.compute.h
rm -f ./ssr.compute.h
rm -f ./chunk_vert.glsl.h
rm -f ./chunk_frag.glsl.h
rm -f ./text_vert.glsl.h
rm -f ./text_frag.glsl.h
rm -f ./composite_vert.glsl.h
rm -f ./composite_frag.glsl.h
echo "Build complete."

if [ $# -eq 0 ] || [ "$1" != "ci" ]; then
    ./voxen
fi
