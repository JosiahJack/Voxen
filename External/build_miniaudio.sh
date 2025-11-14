#!/bin/bash
set -euo pipefail
gcc -O2 -c miniaudio.c
ar rcs libminiaudio.0.11.22.a miniaudio.o
