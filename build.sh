#!/bin/bash
clear
CC=gcc
CFLAGS="-std=c11 -Wall -Wextra -O3 -g -D_POSIX_C_SOURCE=199309L"
MINIAUDIO_CFLAGS="-std=c11 -Wall -Wextra -O2 -g -D_POSIX_C_SOURCE=199309L -DNDEBUG"
LDFLAGS="-lSDL2 -lSDL2_ttf -lGLEW -lGL -lm -lrt -lassimp -lenet -lpthread -s"
SOURCES="voxen.c cli_args.c event.c quaternion.c matrix.c input.c data_models.c \
         data_textures.c network.c text.c shaders.c voxel.c data_parser.c levels.c \
         instance.c data_entities.c audio.c"
#         instance.c data_entities.c audio.c \
#         miniaudio.c"

TEMP_DIR=temp_build
mkdir -p $TEMP_DIR
for src in $SOURCES; do
    if [ "$src" != "miniaudio.c" ]; then
        rm -f "$TEMP_DIR/${src%.c}.o"
    fi
done
echo "Compiling voxen..."

# Compile sources in parallel to temporary object files
pids=()
for src in $SOURCES; do
    obj="$TEMP_DIR/${src%.c}.o"
    # Always compile non-miniaudio sources; compile miniaudio.c only if it’s newer
    if [ "$src" != "miniaudio.c" ] || [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
        #echo "Compiling $src..."
        if [ "$src" = "miniaudio.c" ]; then
            $CC -c $src $MINIAUDIO_CFLAGS -o $obj &
        else
            $CC -c $src $CFLAGS -o $obj &
        fi
        pids+=($!) # Store process ID
    fi
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

# Clean up object files except miniaudio.o
for src in $SOURCES; do
    if [ "$src" != "miniaudio.c" ]; then
        rm -f "$TEMP_DIR/${src%.c}.o"
    fi
done
echo "Build complete."
./voxen
