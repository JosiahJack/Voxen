#!/usr/bin/env python3
import re

# Simple parser for Unity scene file
# Look for PrefabInstance sections and extract the m_Name modifications

with open('/home/qmaster/ai-workspaces/Citadel/Assets/Scenes/CitadelScene.unity', 'r', encoding='utf-8', errors='replace') as f:
    content = f.read()

# Find all PrefabInstance sections by looking for "--- !u!1001 &NNN"
# followed by "PrefabInstance:" on the next line

prefab_sections = []
current_section = []
in_prefab = False
section_start_line = 0

lines = content.splitlines()
for i, line in enumerate(lines):
    stripped = line.strip()
    
    # Detect start of PrefabInstance document
    if re.match(r'^---\s*!u!1001\s*&\d+', stripped):
        # Save previous section if any
        if in_prefab and current_section:
            prefab_sections.append((section_start_line, current_section))
        in_prefab = True
        section_start_line = i
        current_section = [line]
    elif in_prefab:
        current_section.append(line)
        # End of section when we hit another document marker or end of file
        if stripped.startswith('---') and not re.match(r'^---\s*!u!1001\s*&\d+', stripped):
            prefab_sections.append((section_start_line, current_section))
            in_prefab = False
            current_section = []

# Save last section
if in_prefab and current_section:
    prefab_sections.append((section_start_line, current_section))

print(f"Found {len(prefab_sections)} PrefabInstance sections")

# Now extract m_Name and m_Text from each section
text_decal_objects = []
stopdss1_objects = []

for section_start, section in prefab_sections:
    # Debug: print first few lines of first section
    if len(prefab_sections) > 0 and section_start == prefab_sections[0][0]:
        print(f"\n=== First section at line {section_start} ===")
        for line in section[:20]:
            print(line)
    
    prefab_name = ""
    text_content = ""
    
    for i in range(len(section)):
        line = section[i].strip()
        
        # Look for propertyPath: m_Name - value is on NEXT line
        if 'propertyPath: m_Name' in line:
            # Get value from next line
            if i + 1 < len(section):
                next_line = section[i + 1].strip()
                match = re.search(r'value:\s*"?([^"\s#]+)', next_line)
                if match:
                    prefab_name = match.group(1).strip()
                    print(f"Line ~{section_start + i + 2}: Found prefab name: '{prefab_name}'")
                    
                    # Check if it's a text_decal variant
                    if 'text_decal' in prefab_name.lower():
                        if 'stopdss1' in prefab_name.lower():
                            stopdss1_objects.append((section_start + i + 2, prefab_name, text_content))
                        else:
                            text_decal_objects.append((section_start + i + 2, prefab_name, text_content))
        
        # Look for propertyPath: m_Text - value is on NEXT line
        elif 'propertyPath: m_Text' in line:
            if i + 1 < len(section):
                next_line = section[i + 1].strip()
                match = re.search(r'value:\s*"?(.+?)(?:\s*#|\s*objectReference|$)', next_line)
                if match:
                    text_content = match.group(1).strip()
                    print(f"Line ~{section_start + i + 2}: Found text content: '{text_content}'")
                    
                    # Update the last prefab_name we found
                    if text_decal_objects:
                        text_decal_objects[-1] = (text_decal_objects[-1][0], text_decal_objects[-1][1], text_content)
                    elif stopdss1_objects:
                        stopdss1_objects[-1] = (stopdss1_objects[-1][0], stopdss1_objects[-1][1], text_content)

print(f"\n=== Summary ===")
print(f"Total text_decal objects found: {len(text_decal_objects)}")
print(f"Total text_decalStopDSS1 objects found: {len(stopdss1_objects)}")

print(f"\n=== First 30 text_decal objects ===")
for line_num, name, text in text_decal_objects[:30]:
    print(f"Line {line_num}: Name='{name}', Text='{text}'")

print(f"\n=== First 20 text_decalStopDSS1 objects ===")
for line_num, name, text in stopdss1_objects[:20]:
    print(f"Line {line_num}: Name='{name}', Text='{text}'")