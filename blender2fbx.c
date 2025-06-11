#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blender2fbx.h"

int BlenderConvert() {
    // Command to run Blender in background and run the Python export script
    char command[2048];
    snprintf(command, sizeof(command),"blender ./Models/med1_1.blend --background --python ./blender2fbx.py -- ./Models/med1_1.fbx");
    printf("Converting med1_1.blend: %s\n", command);

    snprintf(command, sizeof(command),"blender ./Models/med1_7.blend --background --python ./blender2fbx.py -- ./Models/med1_7.fbx");
    printf("Converting med1_7.blend: %s\n", command);

    snprintf(command, sizeof(command),"blender ./Models/med1_9.blend --background --python ./blender2fbx.py -- ./Models/med1_9.fbx");
    printf("Converting med1_9.blend: %s\n", command);
    int result = system(command);

    return result == 0 ? 0 : 1;
}
