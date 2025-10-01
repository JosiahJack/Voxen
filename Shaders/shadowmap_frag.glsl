// Shadowmap Fragment Shader
#version 430 core
in vec3 WorldPos;
uniform vec3 lightPos;
uniform int lightIdx;
uniform int face;
layout(std430,  binding =  5) buffer ShadowMaps { float depthData[]; };
const int SHADOW_MAP_SIZE = 256;

void main() {
    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    int index = (lightIdx * 6 * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE) +
                 face * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE +
                 texelCoord.y * SHADOW_MAP_SIZE + texelCoord.x;

    if (depthData[index] <= 0.00001) depthData[index] = 15.36; // Keep GL driver from increasing RAM by 1gb by clearing it here.
    float dist = length(WorldPos - lightPos); // Linear distance in world space
    if (index > 0 && dist < depthData[index]) depthData[index] = dist;
}
