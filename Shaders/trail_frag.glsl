// trail_frag.glsl - Trail fragment shader (SSBO palette texture lookup)
layout(std430, binding = 12) buffer ColorBuffer { uint colors[]; };
layout(std430, binding = 14) buffer TextureOffsets { uint textureOffsets[]; };
layout(std430, binding = 15) buffer TextureSizes { ivec2 textureSizes[]; };
layout(std430, binding = 8) buffer TexturePalettes { uint texturePalettes[]; };
layout(std430, binding = 9) buffer TexturePaletteOffsets { uint texturePaletteOffsets[]; };
in vec2 vUV;
in vec4 vColor;
flat in uint vTexIndex; flat in uint vBlendMode;
out vec4 outColor;
vec4 getTrailTextureColor(ivec2 texCoord, int texSizeX) { uint pixelOffset=textureOffsets[vTexIndex]+uint(texCoord.y)*uint(texSizeX)+uint(texCoord.x); uint localOffset=pixelOffset & 3u; return unpackUnorm4x8(texturePalettes[texturePaletteOffsets[vTexIndex] + (((colors[pixelOffset>>2u])>>(localOffset<<3u))&0xFFu)]); }
void main() {
    ivec2 texSize = textureSizes[vTexIndex]; vec2 uv = vec2(vUV.x, 1.0 - vUV.y); vec4 texColor = getTrailTextureColor(ivec2(clamp(fract(uv), 0.0, 0.99999) * vec2(texSize)), texSize.x);
    // Premultiplied output throughout: the trail batch draws with glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA), so
    // rgb is already scaled by its own alpha.  For the default mode this is algebraically identical to the old
    // straight-alpha texColor*vColor under glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA).
    if (vBlendMode == 1u) { outColor = vec4(texColor.rgb * vColor.rgb * vColor.a, texColor.a * vColor.a); return; } // Additive (brightness-as-alpha, adds to scene)
    if (vBlendMode == 3u) { float blackAsAlpha = max(max(texColor.r, texColor.g), texColor.b); outColor = vec4(texColor.rgb * vColor.rgb, blackAsAlpha * vColor.a); return; }
    outColor = vec4(texColor.rgb * vColor.rgb * vColor.a, texColor.a * vColor.a);
}
