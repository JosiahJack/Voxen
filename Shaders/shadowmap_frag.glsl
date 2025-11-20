// Shadowmap Fragment Shader
#version 430 core
in vec3 FragPos;
layout(std430,  binding = 5) buffer ShadowMaps { uint depthData[]; };
layout(std430, binding = 19) buffer LightIndices { float lights[]; };
uniform int ssbo_indexBase;
uniform uint lightIndex;
uniform int shadowmapSize;

const int LIGHT_DATA_SIZE = 13;
const int LIGHT_DATA_OFFSET_POSX = 0;
const int LIGHT_DATA_OFFSET_POSY = 1;
const int LIGHT_DATA_OFFSET_POSZ = 2;

void main() {
    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    int ssbo_index = ssbo_indexBase + texelCoord.y * shadowmapSize + texelCoord.x;
    uint lightIdx = lightIndex * uint(LIGHT_DATA_SIZE);
    vec3 lightPos = vec3(lights[lightIdx + LIGHT_DATA_OFFSET_POSX], lights[lightIdx + LIGHT_DATA_OFFSET_POSY], lights[lightIdx + LIGHT_DATA_OFFSET_POSZ]);
    vec3 toLight = lightPos - FragPos;
    float dist = length(toLight);
    uint distInt = uint(dist * 100000.0);
    atomicMin(depthData[ssbo_index], distInt);
}
