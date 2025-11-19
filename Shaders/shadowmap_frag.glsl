// Shadowmap Fragment Shader
#version 430 core
in vec3 FragPos;
layout(std430,  binding = 5) buffer ShadowMaps { uint depthData[]; };
uniform int ssbo_indexBase;
uniform vec3 lightPos;
uniform int shadowmapSize;

void main() {
    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    int ssbo_index = ssbo_indexBase + texelCoord.y * shadowmapSize + texelCoord.x;
    vec3 toLight = lightPos - FragPos;
    float dist = length(toLight);
    uint distInt = uint(dist * 10000.0);
    atomicMin(depthData[ssbo_index], distInt);
}
