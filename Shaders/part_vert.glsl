// part_vert.glsl - Particle Vertex Shader
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in vec4 aColor;
layout(location=3) in float aTexIdx;
layout(location=0) uniform mat4 uViewProj;
out vec2 vUV;
out vec4 vColor;
flat out uint vTex;
void main() { gl_Position=uViewProj*vec4(aPos,1.0); vUV=aUV; vColor=aColor; vTex=uint(aTexIdx); }
