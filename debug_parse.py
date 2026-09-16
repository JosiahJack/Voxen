#!/usr/bin/env python3
import re

scene_path = '/home/qmaster/ai-workspaces/Citadel/Assets/Scenes/CitadelScene.unity'

with open(scene_path, 'r', encoding='utf-8', errors='replace') as f:
    lines = f.readlines()

prefab_names = []
in_prefab = False
for i, line in enumerate(lines):
    line_strip = line.strip()
    if 'PrefabInstance:' in line_strip:
        in_prefab = True
        prefab_name = ""
        text_content = ""
        # Look backwards a bit for the Transform fileID
        # For simplicity, just capture the name
    if in_prefab:
        if 'm_Name' in line_strip and 'value:' in line_strip:
            # Extract the value
            match = re.search(r'value:\s*"?([^"\s]+)"?', line_strip)
            if match:
                prefab_name = match.group(1)
                print(f"Line {i+1}: Found prefab name: '{prefab_name}'")
                prefab_names.append(prefab_name)
        elif line_strip.startswith('---') and 'PrefabInstance' not in line_strip:
            in_prefab = False

print(f"\nFound {len(prefab_names)} prefab names")
print("First 20:", prefab_names[:20])

# Check for text_decal variants
text_decal_names = [n for n in prefab_names if 'text_decal' in n.lower()]
print(f"\nFound {len(text_decal_names)} text_decal variants:")
for name in text_decal_names[:30]:
    print(f"  '{name}'")