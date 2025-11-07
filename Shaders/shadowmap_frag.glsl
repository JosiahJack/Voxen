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
    float dist = dot(toLight,toLight); // Actually faster for performance to use length... likely due to shadows mostly being static so read speed is more crucial (or the bias math)
    uint distInt = uint(dist * 10.0);
    atomicMin(depthData[ssbo_index], distInt);
}
