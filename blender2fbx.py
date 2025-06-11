import bpy
import sys

# Get the output .fbx path from command line args
argv = sys.argv
argv = argv[argv.index("--") + 1:]  # only get args after '--'

if len(argv) < 1:
    print("No output path provided for FBX export.")
    sys.exit(1)

output_path = argv[0]

# Export the scene to FBX
bpy.ops.export_scene.fbx(filepath=output_path)
