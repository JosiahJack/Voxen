from PIL import Image
import numpy as np

# Used to generate the blue noise color buffer for use in shaders
# so as to not balloon the number of unique colors unnecessarily and have bundled with
# the engine instead of requiring mods to include it since I consider it a necessity
# for proper smooth lighting results.

# Load 64x64 RGB image
img = Image.open("bluenoise64.png").convert("RGB")  # Ensure RGB format
pixels = np.array(img).flatten() / 255.0  # Normalize to [0,1], shape=(64*64*3,)

# Write to bluenoise.cginc with 4 RGB triplets per line
with open("bluenoise64.cginc", "w") as f:
    f.write("const float blueNoise[12288] = float[](\n")
    for i in range(0, len(pixels), 12):  # Step by 12 (4 RGB triplets)
        line = pixels[i:i+12]
        f.write("    " + ", ".join([f"{x:.6f}" for x in line]))
        if i + 12 < len(pixels):
            f.write(",\n")
        else:
            f.write("\n")
    f.write(");\n")
