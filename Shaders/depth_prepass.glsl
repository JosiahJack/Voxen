// depth_prepass.glsl: Renders all opaque objects prior to main forward+ pass
#version 430 core
#extension GL_ARB_shading_language_packing : require
#extension GL_ARB_shader_image_load_store : enable
layout(early_fragment_tests) in;

layout(location = 0) uniform uint instanceIndex;
layout(location = 2) uniform mat4 viewProjection;
layout(location = 17) uniform uint unlit;

void main() {}
