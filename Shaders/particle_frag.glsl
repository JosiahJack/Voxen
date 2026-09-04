// particle_frag.glsl - Particle fragment shader
layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; };
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; };
layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; };
layout(std430, binding = 8) buffer TexturePalettes { uint texturePalettes[]; };
layout(std430, binding = 9) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; };
layout(location = 10) uniform sampler2D uSceneDepth;
layout(location = 11) uniform vec2 uViewportSize;
layout(location = 12) uniform float uNear;
layout(location = 13) uniform float uFar;
layout(location = 14) uniform float uSoftness;
uniform int uMode;
uniform float uCutThreshold;
uniform float uNearFadeStart;
uniform float uNearFadeRange;
layout(location = 7) uniform int uBlendMode;
layout(location = 8) uniform uint uTexIndex;
layout(location = 9) uniform int uSolidColor;
in vec2 vUV;
in vec4 vColor;
in float vViewDist;
flat in uint vFlags;
flat in uint vTexIndex;
out vec4 outColor;
vec4 getParticleTextureColor(ivec2 texCoord, int texSizeX) { uint pixelOffset=textureOffsets[vTexIndex]+uint(texCoord.y)*uint(texSizeX)+uint(texCoord.x); uint localOffset=pixelOffset & 3u; return unpackUnorm4x8(texturePalettes[texturePaletteOffsets[vTexIndex] + (((colors[pixelOffset>>2u])>>(localOffset<<3u))&0xFFu)]); }
float linearizeSceneDepth(vec2 screenUV) { float rawDepth = texture(uSceneDepth, screenUV).r; float ndcZ = rawDepth * 2.0 - 1.0; return (2.0 * uNear * uFar) / (uFar + uNear - ndcZ * (uFar - uNear)); }
void main() {
    ivec2 texSize = textureSizes[vTexIndex]; vec2 uv = vec2(vUV.x, 1.0 - vUV.y); vec4 texColor = getParticleTextureColor(ivec2(clamp(fract(uv), 0.0, 0.99999) * vec2(texSize)), texSize.x);
    float fade = 1.0; if ((vFlags & 2u) != 0u) { float sceneDist = linearizeSceneDepth(gl_FragCoord.xy / uViewportSize); fade = clamp((sceneDist - vViewDist) / max(uSoftness, 0.0001f), 0.0, 1.0); }
    if (uBlendMode == 1) { outColor = vec4(texColor.rgb * texColor.a * vColor.rgb * vColor.a * fade, 0.0); return; } // Additive (brightness-as-alpha, adds to scene)
    if (uBlendMode == 2) { outColor = vec4(mix(texColor.rgb * vColor.rgb, vec3(1.0), 1.0 - fade), 1.0); return; } // Multiply (soft-fades toward no darkening)
    outColor = texColor * vColor * fade;
}
