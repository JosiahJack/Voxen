// chunk.glsl: Generic shader for unlit textured surfaces (all world geometry, items,
// enemies, doors, etc., without transparency for first pass prior to lighting.
#version 430 core
#extension GL_ARB_shading_language_packing : require
#extension GL_ARB_shader_image_load_store : enable

layout(location = 0) uniform uint instanceIndex;
layout(location = 1) uniform uint texIndex;
layout(location = 2) uniform mat4 viewProjection;

void main() {}
