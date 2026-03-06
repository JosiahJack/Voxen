#!/bin/bash
set -euo pipefail
# Static bundle
gcc -O2 -c miniaudio.c
ar rcs libminiaudio.0.11.22.a miniaudio.o

# # Dynamic .so
# gcc -O2 -fPIC -c miniaudio.c -o miniaudio.o
# gcc -shared -Wl,-soname,libminiaudio.0.11.22.so -o libminiaudio.0.11.22.so miniaudio.o # -lpthread -lm -ldl
