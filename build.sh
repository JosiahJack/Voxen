gcc -o voxen voxen.c quaternion.c matrix.c -std=c11 -Wall -Wextra -O3 -g -D_POSIX_C_SOURCE=199309L -lSDL2 -lSDL2_ttf -lSDL2_image -lGLEW -lGL -lm -lrt
