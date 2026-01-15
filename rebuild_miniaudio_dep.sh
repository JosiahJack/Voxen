#!/bin/bash
set -euo pipefail
echo "Building voxen's heavily reduced miniaudio DLL for Windows..."
x86_64-w64-mingw32-gcc -shared \
    ./External/miniaudio.c \
    -o ./External/Windows/libminiaudio.0.11.22.dll \
    -I./External \
    -D_WIN32 \
    -D MA_DLL \
    -lwinmm -lole32 -ladvapi32 -lgdi32 \
    -Wl,--out-implib,./External/Windows/libminiaudio.dll.a
echo "Done"
