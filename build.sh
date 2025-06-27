#!/bin/bash
gcc -o voxen \
    voxen.c cli_args.c event.c quaternion.c matrix.c input.c data_models.c \
    data_textures.c render.c network.c lights.c text.c shaders.c voxel.c \
    image_effects.c debug.c \
    -std=c11 -Wall -Wextra -O3 -g -D_POSIX_C_SOURCE=199309L -lSDL2 -lSDL2_ttf \
    -lSDL2_image -lGLEW -lGL -lm -lrt -lassimp -lenet
