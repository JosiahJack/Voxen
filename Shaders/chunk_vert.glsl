// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 430 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(std430,binding=1) buffer ModelMatrices { mat4 modelMatrices[]; };
layout(location=0) uniform uint instanceIndex;
layout(location=2) uniform mat4 viewProjection;
layout(location=3) uniform uint isUI;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
void main() {
    if (isUI > 0) {
        TexCoord = aNormal.xy; // uiImageVAO only has pos and uvs so uvs are at location 1 (2nd)
        gl_Position = viewProjection * vec4(aPos,1.0);
        FragPos = vec3(aPos);
        Normal = vec3(0.0,0.0,1.0);
    } else {
        TexCoord = aTexCoord;
        mat4 matrix = modelMatrices[instanceIndex];
        vec4 worldPos = matrix * vec4(aPos,1.0);
        gl_Position = viewProjection * worldPos;
        FragPos = vec3(worldPos);
        Normal = mat3(transpose(inverse(matrix))) * aNormal;
    }
}
