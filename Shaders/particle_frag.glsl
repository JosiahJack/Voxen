// particle_frag.glsl - Particle fragment shader
layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; };
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; };
layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; };
layout(std430, binding = 8) buffer TexturePalettes { uint texturePalettes[]; };
layout(std430, binding = 9) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; };
uniform sampler2D uSceneDepth;
uniform vec2 uViewportSize;
uniform int uMode;
uniform float uCutThreshold;
uniform float uSoftness;
uniform float uNearFadeStart;
uniform float uNearFadeRange;
layout(location = 7) uniform int uBlendMode;
layout(location = 8) uniform uint uTexIndex;
layout(location = 9) uniform int uSolidColor;
in vec2 vUV;
in vec4 vColor;
in float vViewDist;
flat in uint vFlags;
out vec4 outColor;
vec4 getParticleTextureColor(ivec2 texCoord, int texSizeX) { uint pixelOffset=textureOffsets[uTexIndex]+uint(texCoord.y)*uint(texSizeX)+uint(texCoord.x); uint localOffset=pixelOffset & 3u; return unpackUnorm4x8(texturePalettes[texturePaletteOffsets[uTexIndex] + (((colors[pixelOffset>>2u])>>(localOffset<<3u))&0xFFu)]); }
float linearizeSceneDepth(vec2 screenUV) { float rawDepth = texture(uSceneDepth, screenUV).r; float ndcZ = rawDepth * 2.0 - 1.0; float near = 0.1; float far = 500.0; float viewZ = (2.0 * near * far) / (far + near - ndcZ * (far - near)); return viewZ; }
void main() {
    ivec2 texSize = textureSizes[uTexIndex]; vec2 uv = vec2(vUV.x, 1.0 - vUV.y); vec4 texColor = getParticleTextureColor(ivec2(clamp(fract(uv), 0.0, 0.99999) * vec2(texSize)), texSize.x);
    if (uBlendMode == 1) { outColor = vec4(texColor.rgb * texColor.a, 0.0); return; } // Additive (brightness-as-alpha, adds to scene)
    if (uBlendMode == 2) { outColor = vec4(texColor.rgb, 1.0); return; } // Multiply (darkens where texture is dark)
    outColor = vec4(texColor.rgb, texColor.a);
}
