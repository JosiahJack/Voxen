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

/*    float oldVal = depthData[ssbo_index];
    if (oldVal <= 0.0001) oldVal = 15.36; */// Keep GL driver from increasing RAM by 1gb by clearing it here instead of on CPU call.
    vec3 toLight = lightPos - FragPos;
    float dist = length(toLight);
    dist = clamp(dist, 0.0, 15.36); // clip to max distance
    uint distInt = uint(dist * 10000.0);
    atomicMin(depthData[ssbo_index], distInt);
//     if (dist < oldVal || oldVal >= 15.3599) depthData[ssbo_index] = dist;
}
