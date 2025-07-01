#!/bin/bash

# Compiler and flags
CC=gcc
CFLAGS="-std=c11 -Wall -Wextra -O3 -g -D_POSIX_C_SOURCE=199309L"
MINIAUDIO_CFLAGS="-std=c11 -Wall -Wextra -O2 -g -D_POSIX_C_SOURCE=199309L -DNDEBUG"
LDFLAGS="-lSDL2 -lSDL2_ttf -lGLEW -lGL -lm -lrt -lassimp -lenet -lpthread"
SOURCES="voxen.c cli_args.c event.c quaternion.c matrix.c input.c data_models.c \
         data_textures.c render.c network.c lights.c text.c shaders.c voxel.c \
         image_effects.c debug.c data_definitions.c audio.c"

if [ "$1" = "clean" ]; then
    rm -rf $TEMP_DIR voxen
    echo "Cleaned build artifacts."
    exit 0
fi

# Fixed temporary directory for object files
TEMP_DIR=voxen_build_temp

# Create temporary directory if it doesn't exist
mkdir -p $TEMP_DIR

# Compile sources in parallel to temporary object files
for src in $SOURCES; do
    obj="$TEMP_DIR/${src%.c}.o"
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
        echo "Compiling $src..."
        if [ "$src" = "audio.c" ]; then
            $CC -c $src $MINIAUDIO_CFLAGS -o $obj &
        else
            $CC -c $src $CFLAGS -o $obj &
        fi
    fi
done

wait
$CC $TEMP_DIR/*.o -o voxen $LDFLAGS
echo "Build complete."
