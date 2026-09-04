// trail_frag.glsl - Trail fragment shader (uses premultiplied alpha)
uniform sampler2DArray uParticleTextures;
uniform int uMode;

in vec2 vUV;
in vec4 vColor;

out vec4 outColor;

void main() {
    vec4 texColor = texture(uParticleTextures, vec3(vUV.x, vUV.y, 0.0));
    vec4 color = texColor * vColor;

    if (uMode == 0) {
        outColor = vec4(color.rgb, 1.0);
        return;
    }

    outColor = vec4(color.rgb * color.a, color.a);
}