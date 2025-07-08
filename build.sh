#!/bin/bash
CC=gcc
CFLAGS="-std=c11 -Wall -Wextra -O3 -g -D_POSIX_C_SOURCE=199309L"
MINIAUDIO_CFLAGS="-std=c11 -Wall -Wextra -O2 -g -D_POSIX_C_SOURCE=199309L -DNDEBUG"
LDFLAGS="-lSDL2 -lSDL2_ttf -lGLEW -lGL -lm -lrt -lassimp -lenet -lpthread"
SOURCES="voxen.c cli_args.c event.c quaternion.c matrix.c input.c data_models.c \
         data_textures.c render.c network.c lights.c text.c shaders.c voxel.c \
         image_effects.c data_parser.c levels.c instance.c \
         data_entities.c audio.c"

TEMP_DIR=temp_build
#if [ "$1" = "clean" ]; then
#    rm ./$TEMP_DIR/*.o
#    echo "Cleaned build artifacts."
#    exit 0
#fi

mkdir -p $TEMP_DIR
rm -f ./$TEMP_DIR/*.o

# Compile sources in parallel to temporary object files
pids=()
for src in $SOURCES; do
    obj="$TEMP_DIR/${src%.c}.o"
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
        echo "Compiling $src..."
        if [ "$src" = "audio.c" ]; then
            $CC -c $src $MINIAUDIO_CFLAGS -o $obj &
        else
            $CC -c $src $CFLAGS -o $obj &
        fi

        pids+=($!) # Store process ID
    fi
done

for pid in "${pids[@]}"; do
    wait $pid
    if [ $? -ne 0 ]; then
        echo "ERROR: Compilation failed for one or more source files."
        exit 1
    fi
done

wait
$CC $TEMP_DIR/*.o -o voxen $LDFLAGS
rm ./$TEMP_DIR/*.o
echo "Build complete."
