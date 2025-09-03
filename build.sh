#!/bin/bash
clear
CC=gcc
CFLAGS="-std=c11 -Wall -Wextra -O3 -D_POSIX_C_SOURCE=199309L"
MINIAUDIO_CFLAGS="-std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=199309L -DNDEBUG"
LDFLAGS="-lSDL2 -lSDL2_ttf -lGLEW -lGL -lm -lrt -lassimp -lenet -lpthread -s"
SOURCES="voxen.c data_textures.c data_parser.c audio.c dynamic_culling.c"
MINIAUDIO="miniaudio.c"

TEMP_DIR=temp_build
mkdir -p $TEMP_DIR
shopt -s extglob
rm -f "$TEMP_DIR"/!(*miniaudio).o 2>/dev/null
shopt -u extglob

echo "Compiling voxen..."

# Compile miniaudio separately if missing or outdated
MINIAUDIO_OBJ="$TEMP_DIR/miniaudio.o"
if [ ! -f "$MINIAUDIO_OBJ" ] || [ "$MINIAUDIO" -nt "$MINIAUDIO_OBJ" ]; then
    $CC -c "$MINIAUDIO" $MINIAUDIO_CFLAGS -o "$MINIAUDIO_OBJ"
    if [ $? -ne 0 ]; then
        echo "ERROR: miniaudio compilation failed."
        exit 1
    fi
fi

# Compile other sources in parallel
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

# Clean up object files except miniaudio.o
shopt -s extglob
rm -f "$TEMP_DIR"/!(*miniaudio).o 2>/dev/null
shopt -u extglob
echo "Build complete."
./voxen
