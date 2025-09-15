#!/bin/bash
clear
TEMP_DIR=temp_build
mkdir -p $TEMP_DIR
rm -f "$TEMP_DIR"/*.o

# Convert shaders into string headers
sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' ./Shaders/deferred_lighting.compute \
    | sed '1i const char* deferredLighting_computeShader =' \
    | sed '$a ;' \
    > ./Shaders/deferred_lighting.compute.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' ./Shaders/ssr.compute \
    | sed '1i const char* ssr_computeShader =' \
    | sed '$a ;' \
    > ./Shaders/ssr.compute.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' ./Shaders/chunk_vert.glsl \
    | sed '1i const char* vertexShaderSource =' \
    | sed '$a ;' \
    > ./Shaders/chunk_vert.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' ./Shaders/chunk_frag.glsl \
    | sed '1i const char* fragmentShaderTraditional =' \
    | sed '$a ;' \
    > ./Shaders/chunk_frag.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' ./Shaders/text_vert.glsl \
    | sed '1i const char* textVertexShaderSource =' \
    | sed '$a ;' \
    > ./Shaders/text_vert.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' ./Shaders/text_frag.glsl \
    | sed '1i const char* textFragmentShaderSource =' \
    | sed '$a ;' \
    > ./Shaders/text_frag.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' ./Shaders/composite_vert.glsl \
    | sed '1i const char* quadVertexShaderSource =' \
    | sed '$a ;' \
    > ./Shaders/composite_vert.glsl.h

sed 's/"/\\"/g; s/^/"/; s/$/\\n"/' ./Shaders/composite_frag.glsl \
    | sed '1i const char* quadFragmentShaderSource =' \
    | sed '$a ;' \
    > ./Shaders/composite_frag.glsl.h

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
rm -f ./Shaders/deferred_lighting.compute.h
rm -f ./Shaders/ssr.compute.h
rm -f ./Shaders/chunk_vert.glsl.h
rm -f ./Shaders/chunk_frag.glsl.h
rm -f ./Shaders/text_vert.glsl.h
rm -f ./Shaders/text_frag.glsl.h
rm -f ./Shaders/composite_vert.glsl.h
rm -f ./Shaders/composite_frag.glsl.h
echo "Build complete."

if [ $# -eq 0 ] || [ "$1" != "ci" ]; then
    ./voxen
fi
