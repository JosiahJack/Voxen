// imageblit.glsl
// Full screen quad unlit textured for presenting image buffers such as results
// from compute shaders, image effects, post-processing, etc..
#version 450 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D tex;
uniform int debugView;
uniform int debugValue;
uniform uint screenWidth;
uniform uint screenHeight;
layout(rgba8, binding = 4) uniform image2D outputImage;
const int SSR_RES = 4;

vec4 unpackColor32(uint color) {
    return vec4(float((color >> 24) & 0xFF) / 255.0,  // r
                float((color >> 16) & 0xFF) / 255.0,  // g
                float((color >>  8) & 0xFF) / 255.0,  // b
                float((color      ) & 0xFF) / 255.0); // a
}

void main() {
    FragColor = texture(tex, TexCoord);
    if (debugValue > 0) return;

    if (debugView == 0) {
        ivec2 pixel = ivec2(TexCoord * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES));
        vec4 reflectionColor = vec4(0.0);
        ivec2 offsets[25] = ivec2[](
            ivec2(-2, -2), ivec2(-1, -2), ivec2(0, -2), ivec2(1, -2), ivec2(2,-2),
            ivec2(-2, -1), ivec2(-1, -1), ivec2(0, -1), ivec2(1, -1), ivec2(2,-1),
            ivec2(-2, 0), ivec2(-1, 0), ivec2(0, 0), ivec2(1, 0), ivec2(2,0),
            ivec2(-2, 1), ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1), ivec2(2, 1),
            ivec2(-2, 2), ivec2(-1, 2), ivec2(0, 2), ivec2(1, 2), ivec2(2, 2)
        );

    float weights[25] = float[](
        0.00390625, 0.015625, 0.0234375, 0.015625, 0.00390625,
        0.015625,   0.0625,   0.09375,   0.0625,   0.015625,
        0.0234375,  0.09375,  0.140625,  0.09375,  0.0234375,
        0.015625,   0.0625,   0.09375,   0.0625,   0.015625,
        0.00390625, 0.015625, 0.0234375, 0.015625, 0.00390625
    );

        for (int i = 0; i < 9; ++i) {
            ivec2 samplePixel = pixel + offsets[i];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[i],weights[i],weights[i]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 6.0;
        }
        FragColor += reflectionColor;
    } else if (debugView == 7 || debugView == 10) {
        FragColor = imageLoad(outputImage, ivec2(TexCoord * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES)));
    }
}
