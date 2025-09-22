// composite.glsl
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

        // Unrolled loop
        {
            ivec2 samplePixel = pixel + offsets[0];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[0],weights[0],weights[0]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[1];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[1],weights[1],weights[1]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[2];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[2],weights[2],weights[2]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[3];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[3],weights[3],weights[3]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[4];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[4],weights[4],weights[4]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[5];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[5],weights[5],weights[5]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[6];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[6],weights[6],weights[6]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[7];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[7],weights[7],weights[7]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[8];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[8],weights[8],weights[8]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[9];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[9],weights[9],weights[9]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[10];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[10],weights[10],weights[10]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[11];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[11],weights[11],weights[11]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[12];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[12],weights[12],weights[12]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[13];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[13],weights[13],weights[13]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[14];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[14],weights[14],weights[14]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[15];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[15],weights[15],weights[15]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[16];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[16],weights[16],weights[16]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[17];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[17],weights[17],weights[17]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[18];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[18],weights[18],weights[18]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[19];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[19],weights[19],weights[19]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[20];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[20],weights[20],weights[20]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[21];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[21],weights[21],weights[21]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[22];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[22],weights[22],weights[22]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[23];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[23],weights[23],weights[23]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        {
            ivec2 samplePixel = pixel + offsets[24];
            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));
            vec3 sampleWeight = vec3(weights[24],weights[24],weights[24]);
            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 2.0;
        }
        FragColor += reflectionColor;
    } else if (debugView == 7 || debugView == 10) {
        FragColor = imageLoad(outputImage, ivec2(TexCoord * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES)));
    }
}
