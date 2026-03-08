#!/bin/bash
set -euo pipefail
# Static bundle
gcc -O2 -c -ffreestanding -fno-exceptions -fno-stack-protector -fno-builtin -fdata-sections -ffunction-sections -ffast-math -g1 -std=c11 -Wall -Wextra -fno-omit-frame-pointer -fstrict-aliasing -fno-common -Wno-unused-function -Wno-implicit-fallthrough -Walloca -Wstack-usage=262144 -Wformat=2 -Wnull-dereference -Wstrict-prototypes -Wno-overlength-strings -Wno-implicit-function-declaration miniaudio.c
ar rcs libminiaudio.0.11.22.a miniaudio.o

# # Dynamic .so
# gcc -O2 -fPIC -c miniaudio.c -o miniaudio.o
# gcc -shared -Wl,-soname,libminiaudio.0.11.22.so -o libminiaudio.0.11.22.so miniaudio.o # -lpthread -lm -ldl
