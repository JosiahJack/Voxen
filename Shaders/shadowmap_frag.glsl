// Shadowmap Fragment Shader
#version 430 core
in vec3 WorldPos;
uniform vec3 lightPos;
uniform int lightIdx;
uniform int face;
layout(std430,  binding =  5) buffer ShadowMaps { uint depthData[]; };
const int SHADOW_MAP_SIZE = 256;

void main() {
    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    int ssbo_index = (lightIdx * 393216/*6 * SHADOW_MAP_SIZE * SHADOW_MAP_SIZE*/) + // Slight performance boost hardcoding it
                 face * 65536/*SHADOW_MAP_SIZE * SHADOW_MAP_SIZE*/ +
                 texelCoord.y * 256/*SHADOW_MAP_SIZE*/ + texelCoord.x;

    int packedIndex = ssbo_index >> 1;           // two values per uint
    bool upper      = false;//(ssbo_index & 1) == 1;
    uint oldVal     = depthData[packedIndex];
    uint stored     = upper ? (oldVal >> 16) & 0xFFFFu : oldVal & 0xFFFFu;
    if (stored == 0) stored = 0xFFFFu; // Keep GL driver from increasing RAM by 1gb by clearing it here instead of on CPU call.

    float dist = length(WorldPos - lightPos);
    dist = clamp(dist, 0.0, 15.36);              // clip to max distance
    uint q = uint((dist / 15.36) * 65535.0 + 0.5);
    if (q < stored || stored == 0xFFFFu) {            // closer or uninitialized
        if (upper) oldVal = (oldVal & 0x0000FFFFu) | (q << 16);
        else       oldVal = (oldVal & 0xFFFF0000u) | q;

        depthData[packedIndex] = oldVal;
    }
}
