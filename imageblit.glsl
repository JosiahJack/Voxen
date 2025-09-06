// imageblit.glsl
// Full screen quad unlit textured for presenting image buffers such as results
// from compute shaders, image effects, post-processing, etc..
const char *quadVertexShaderSource =
    "#version 450 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\n";

const char* quadFragmentShaderSource =
    "#version 450 core\n"
    "in vec2 TexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D tex;\n"
    "uniform int debugView;\n"
    "uniform int debugValue;\n"
    "uniform uint screenWidth;\n"
    "uniform uint screenHeight;\n"
    "layout(rgba8, binding = 4) uniform image2D outputImage;\n"
    "layout(rgba32f, binding = 2) readonly uniform image2D inputNormals;\n"
    "const int SSR_RES = 4;\n"

    "vec4 unpackColor32(uint color) {\n"
    "    return vec4(float((color >> 24) & 0xFF) / 255.0,\n"  // r
    "                float((color >> 16) & 0xFF) / 255.0,\n"  // g
    "                float((color >>  8) & 0xFF) / 255.0,\n"  // b
    "                float((color      ) & 0xFF) / 255.0);\n" // a
    "}\n"

    "void main() {\n"
    "    FragColor = texture(tex, TexCoord);\n"
    "    vec4 normalPack = imageLoad(inputNormals, ivec2(TexCoord.x,TexCoord.y));\n"
    "    vec4 specColor = unpackColor32(floatBitsToUint(normalPack.a));\n"
    "    if (debugView == 0) {\n"
    "        ivec2 pixel = ivec2(TexCoord * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES));\n"
    "        vec4 reflectionColor = vec4(0.0);\n"
    "        ivec2 offsets[25] = ivec2[](\n"
    "            ivec2(-2, -2), ivec2(-1, -2), ivec2(0, -2), ivec2(1, -2), ivec2(2,-2),\n"
    "            ivec2(-2, -1), ivec2(-1, -1), ivec2(0, -1), ivec2(1, -1), ivec2(2,-1),\n"
    "            ivec2(-2, 0), ivec2(-1, 0), ivec2(0, 0), ivec2(1, 0), ivec2(2,0),\n"
    "            ivec2(-2, 1), ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1), ivec2(2, 1),\n"
    "            ivec2(-2, 2), ivec2(-1, 2), ivec2(0, 2), ivec2(1, 2), ivec2(2, 2)\n"
    "        );\n"

    "    float weights[25] = float[](\n"
    "        0.00390625, 0.015625, 0.0234375, 0.015625, 0.00390625,\n"
    "        0.015625,   0.0625,   0.09375,   0.0625,   0.015625,\n"
    "        0.0234375,  0.09375,  0.140625,  0.09375,  0.0234375,\n"
    "        0.015625,   0.0625,   0.09375,   0.0625,   0.015625,\n"
    "        0.00390625, 0.015625, 0.0234375, 0.015625, 0.00390625\n"
    "    );\n"

    "        for (int i = 0; i < 9; ++i) {\n"
    "            ivec2 samplePixel = pixel + offsets[i];\n"
    "            samplePixel = clamp(samplePixel, ivec2(0), ivec2(int(screenWidth/SSR_RES)-1, int(screenHeight/SSR_RES)-1));\n"
    "            vec3 sampleWeight = vec3(weights[i],weights[i],weights[i]);\n"// * mix(vec3(0.08), vec3(0.02), specColor.rgb);\n"
    "            reflectionColor.rgb += imageLoad(outputImage, samplePixel).rgb * sampleWeight * 8.0;\n"
    "        }\n"
    "        FragColor += reflectionColor;\n"
    "    } else if (debugView == 7 || debugView == 10) {\n"
    "        FragColor = imageLoad(outputImage, ivec2(TexCoord * vec2(screenWidth/SSR_RES, screenHeight/SSR_RES)));\n"
    "    }\n"
    "}\n";
