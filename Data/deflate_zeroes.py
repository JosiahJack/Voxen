import sys

def remove_default_scale_in_place(file_path):
    target = "|localRotation.x:0|localRotation.y:0|localRotation.z:0|localRotation.w:1"

    try:
        with open(file_path, "r") as infile:
            content = infile.read()
    except FileNotFoundError:
        print(f"Skipping {file_path} - File not found.")
        return

    # ONLY replace the exact scale string
    if target in content:
        optimized_content = content.replace(target, "")

        with open(file_path, "w") as outfile:
            outfile.write(optimized_content)
        print(f"Removed default scales from: {file_path}")
    else:
        print(f"No default scales found in: {file_path}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(
            "Usage: python3 remove_scales.py <file1.txt> <file2.txt> ..."
        )
        sys.exit(1)

    for path in sys.argv[1:]:
        remove_default_scale_in_place(path)

    print("Done!")
    
