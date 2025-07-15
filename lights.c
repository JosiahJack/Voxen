#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "render.h"
#include "matrix.h"
#include "constants.h"
#include "lights.h"
#include "quaternion.h"

float spotAngTypes[8] = { 0.0f, 30.0f, 45.0f, 60.0f, 75.0f, 90.0f, 135.0f, 151.7f }; // What?  I only have 6 spot lights and half are 151.7 and other half are 135.

float lights[LIGHT_COUNT * LIGHT_DATA_SIZE];
bool lightDirty[LIGHT_COUNT] = { [0 ... LIGHT_COUNT-1] = true };
float lightsRangeSquared[LIGHT_COUNT];

void UpdateLightVolumes(void) {

}
