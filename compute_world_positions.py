#!/usr/bin/env python3
"""
Script to compute worldspace positions for text_decal (592) and text_decalStopDSS1 (593)
entities from level data files using Unity scene hierarchy.

Approach:
1. Parse the Unity scene file to extract all GameObjects with their transforms and hierarchy
2. For each text_decal/text_decalStopDSS1 object, compute worldspace position
3. Parse level files and match entries with Unity objects (by text content)
4. Replace anchoredPosition values with computed worldspace positions
"""

import re
import glob
import os
from typing import Dict, List, Tuple, Optional


class TransformInfo:
    """Stores transform information for a Unity object."""
    def __init__(self, file_id: int):
        self.file_id = file_id
        self.name = ""
        self.parent_file_id = 0  # 0 means no parent (root)
        self.local_pos = (0.0, 0.0, 0.0)
        self.local_rot = (0.0, 0.0, 0.0, 1.0)  # x, y, z, w
        self.local_scl = (1.0, 1.0, 1.0)
        self.prefab_name = ""  # From PrefabInstance m_Name modification
        self.text_content = ""  # From m_Text modification
        self.is_text_decal = False
        self.is_text_decal_stopdss1 = False


def parse_unity_scene(scene_path: str) -> Dict[int, TransformInfo]:
    """Parse the Unity scene file using regex-based line scanning."""
    objects = {}
    current_object = None
    current_file_id = None
    in_prefab_instance = False
    prefab_name = ""
    text_content = ""
    is_decal = False
    is_stopdss1 = False
    transform_parent = 0
    
    with open(scene_path, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()
    
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        
        # Detect Transform component header: "--- !u!4 &12345"
        match = re.match(r'^---\s*!u!4\s*&(\d+)', line)
        if match:
            file_id = int(match.group(1))
            current_file_id = file_id
            current_object = TransformInfo(file_id)
            objects[file_id] = current_object
            i += 1
            continue
        
        if current_object is not None:
            # Parse m_Name
            if line.startswith('m_Name:'):
                current_object.name = line[7:].strip()
            
            # Parse m_LocalPosition
            elif line.startswith('m_LocalPosition:'):
                # Next lines: {x: val, y: val, z: val}
                # Check if value is on same line or next
                if '{' in line:
                    pos_str = line[line.index('{'):].strip().rstrip(',')
                    if pos_str.endswith('}'):
                        pos = parse_vector(pos_str)
                        current_object.local_pos = pos
                    else:
                        # Continue reading next line
                        i += 1
                        if i < len(lines):
                            next_line = lines[i].strip()
                            pos_str = next_line.strip().rstrip(',')
                            if pos_str.endswith('}'):
                                pos = parse_vector(pos_str)
                                current_object.local_pos = pos
            
            # Parse m_LocalRotation
            elif line.startswith('m_LocalRotation:'):
                if '{' in line:
                    rot_str = line[line.index('{'):].strip().rstrip(',')
                    if rot_str.endswith('}'):
                        rot = parse_quaternion(rot_str)
                        current_object.local_rot = rot
            
            # Parse m_LocalScale
            elif line.startswith('m_LocalScale:'):
                if '{' in line:
                    scl_str = line[line.index('{'):].strip().rstrip(',')
                    if scl_str.endswith('}'):
                        scl = parse_vector(scl_str)
                        current_object.local_scl = scl
            
            # Parse m_Father (parent reference)
            elif line.startswith('m_Father:'):
                if 'fileID:' in line:
                    match = re.search(r'fileID:\s*(\d+)', line)
                    if match:
                        current_object.parent_file_id = int(match.group(1))
            
            # Parse m_GameObject (to get the GameObject fileID)
            elif line.startswith('m_GameObject:'):
                if 'fileID:' in line:
                    match = re.search(r'fileID:\s*(\d+)', line)
                    if match:
                        pass  # Already have current_object.file_id
            
            # Detect PrefabInstance section (contains modifications)
            elif 'PrefabInstance:' in line:
                in_prefab_instance = True
                prefab_name = ""
                text_content = ""
                transform_parent = 0
            
            elif in_prefab_instance:
                # Look for m_TransformParent
                if 'm_TransformParent:' in line:
                    match = re.search(r'fileID:\s*(\d+)', line)
                    if match:
                        transform_parent = int(match.group(1))
                
                # Look for m_Name modification (the prefab name)
                elif 'm_Name' in line and 'value:' in line:
                    match = re.search(r'value:\s*(\S+)', line)
                    if match:
                        prefab_name = match.group(1).strip('"')
                        # Check if this is a text_decal variant (handle cases like "text_decalStopDSS1" and "text_decalStopDSS1 (2)")
                        if 'text_decal' in prefab_name.lower():
                            current_object.is_text_decal = True
                            if 'stopdss1' in prefab_name.lower().split('text_decal')[-1].lower():
                                current_object.is_text_decal_stopdss1 = True
                        elif 'text_decalStopDSS1' in prefab_name.lower():
                            current_object.is_text_decal = True
                            current_object.is_text_decal_stopdss1 = True
                
                # Look for m_Text modification
                elif 'm_Text' in line and 'value:' in line:
                    match = re.search(r'value:\s*(.+)', line)
                    if match:
                        text_content = match.group(1).strip().strip('"')
                
                # End of PrefabInstance (next --- or end of file)
                elif line.startswith('---') and 'PrefabInstance' not in line:
                    in_prefab_instance = False
                    # Update the object with prefab info
                    if current_object is not None:
                        current_object.prefab_name = prefab_name
                        current_object.text_content = text_content
                        if 'text_decalStopDSS1' in prefab_name:
                            current_object.is_text_decal_stopdss1 = True
                            current_object.is_text_decal = True
                        elif 'text_decal' == prefab_name:
                            current_object.is_text_decal = True
            
            # End of Transform component (next --- with different type)
            elif line.startswith('---') and '!u!4' not in line:
                current_object = None
                current_file_id = None
        
        i += 1
    
    return objects


def parse_vector(s: str) -> Tuple[float, float, float]:
    """Parse a Unity vector string like {x: 1.0, y: 2.0, z: 3.0}."""
    x = re.search(r'x:\s*([-\d.]+)', s)
    y = re.search(r'y:\s*([-\d.]+)', s)
    z = re.search(r'z:\s*([-\d.]+)', s)
    return (
        float(x.group(1)) if x else 0.0,
        float(y.group(1)) if y else 0.0,
        float(z.group(1)) if z else 0.0
    )


def parse_quaternion(s: str) -> Tuple[float, float, float, float]:
    """Parse a Unity quaternion string like {x: 0, y: 0, z: 0, w: 1}."""
    x = re.search(r'x:\s*([-\d.]+)', s)
    y = re.search(r'y:\s*([-\d.]+)', s)
    z = re.search(r'z:\s*([-\d.]+)', s)
    w = re.search(r'w:\s*([-\d.]+)', s)
    return (
        float(x.group(1)) if x else 0.0,
        float(y.group(1)) if y else 0.0,
        float(z.group(1)) if z else 0.0,
        float(w.group(1)) if w else 1.0
    )


def find_object_by_id(file_id: int, objects: Dict[int, TransformInfo]) -> Optional[TransformInfo]:
    """Find an object by its fileID."""
    return objects.get(file_id)


def compute_world_position(file_id: int, objects: Dict[int, TransformInfo], depth=0) -> Optional[Tuple[float, float, float]]:
    """Compute worldspace position by traversing parent hierarchy."""
    if depth > 100:  # Safety limit
        return (0.0, 0.0, 0.0)
    
    obj = find_object_by_id(file_id, objects)
    if obj is None:
        return (0.0, 0.0, 0.0)
    
    if obj.parent_file_id == 0:
        # Root object - world position equals local position
        return obj.local_pos
    else:
        # Recursively get parent's world position
        parent_pos = compute_world_position(obj.parent_file_id, objects, depth + 1)
        if parent_pos is None:
            return obj.local_pos
        # Simple addition (not accounting for rotation/scale for now)
        return (
            parent_pos[0] + obj.local_pos[0],
            parent_pos[1] + obj.local_pos[1],
            parent_pos[2] + obj.local_pos[2]
        )


def parse_level_line(line: str) -> Dict[str, str]:
    """Parse a level file line into key-value pairs."""
    line = line.strip()
    if not line.startswith('constIndex:'):
        return {}
    
    result = {}
    parts = line.split('|')
    for part in parts:
        if ':' in part:
            key, value = part.split(':', 1)
            result[key] = value
    
    return result


def update_level_file(filepath: str, objects: Dict[int, TransformInfo]) -> int:
    """Update a level file with computed worldspace positions. Returns number of changes."""
    
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    changes = 0
    updated_lines = []
    
    for i, line in enumerate(lines):
        line = line.rstrip('\n')
        
        # Check if this is a text_decal (592) or text_decalStopDSS1 (593) entity
        if line.startswith('constIndex:592|') or line.startswith('constIndex:593|'):
            # Parse the level entry
            entry = parse_level_line(line)
            
            # Get the text content from level entry
            text_content = entry.get('text', '')
            
            # Find matching object in Unity scene by text content
            world_pos = None
            matched_obj = None
            
            for file_id, obj in objects.items():
                if obj.is_text_decal or obj.is_text_decal_stopdss1:
                    # Match by text content
                    if text_content and obj.text_content and text_content.lower() == obj.text_content.lower():
                        world_pos = compute_world_position(file_id, objects)
                        matched_obj = obj
                        break
            
            # If no text match, try matching by anchoredPosition (from level) with RectTransform anchored position
            if world_pos is None:
                # Parse anchoredPosition from level line
                level_anchored_x = float(entry.get('anchoredPosition.x', 0))
                level_anchored_y = float(entry.get('anchoredPosition.y', 0))
                
                for file_id, obj in objects.items():
                    if obj.is_text_decal or obj.is_text_decal_stopdss1:
                        # Check if this object's text matches at all
                        if obj.text_content and obj.text_content.lower() == text_content.lower():
                            world_pos = compute_world_position(file_id, objects)
                            matched_obj = obj
                            break
            
            if world_pos is not None:
                # Update anchoredPosition values with worldspace position
                # Replace anchoredPosition.x/y/z with world coordinates
                new_line = re.sub(
                    r'anchoredPosition\.x:[^|]*',
                    f'anchoredPosition.x:{world_pos[0]}',
                    line
                )
                new_line = re.sub(
                    r'anchoredPosition\.y:[^|]*',
                    f'anchoredPosition.y:{world_pos[1]}',
                    new_line
                )
                new_line = re.sub(
                    r'anchoredPosition\.z:[^|]*',
                    f'anchoredPosition.z:{world_pos[2]}',
                    new_line
                )
                
                # Also update anchoredPosition3D if present
                new_line = re.sub(
                    r'anchoredPosition3D\.x:[^|]*',
                    f'anchoredPosition3D.x:{world_pos[0]}',
                    new_line
                )
                new_line = re.sub(
                    r'anchoredPosition3D\.y:[^|]*',
                    f'anchoredPosition3D.y:{world_pos[1]}',
                    new_line
                )
                new_line = re.sub(
                    r'anchoredPosition3D\.z:[^|]*',
                    f'anchoredPosition3D.z:{world_pos[2]}',
                    new_line
                )
                
                updated_lines.append(new_line + '\n')
                changes += 1
                print(f"  L{i+1}: {matched_obj.prefab_name} '{matched_obj.text_content}' -> world pos ({world_pos[0]:.4f}, {world_pos[1]:.4f}, {world_pos[2]:.4f})")
            else:
                updated_lines.append(line + '\n')
                print(f"  L{i+1}: No matching Unity entity found for '{text_content}'")
        else:
            updated_lines.append(line + '\n')
    
    # Write back if changes were made
    if changes > 0:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.writelines(updated_lines)
        print(f"  Saved {changes} changes to {os.path.basename(filepath)}")
    
    return changes


def main():
    """Main function."""
    print("=" * 60)
    print("Computing worldspace positions for text_decal entities")
    print("=" * 60)
    
    # Parse Unity scene
    scene_path = '/home/qmaster/ai-workspaces/Citadel/Assets/Scenes/CitadelScene.unity'
    print(f"\nParsing Unity scene: {scene_path}")
    objects = parse_unity_scene(scene_path)
    
    # Count found decal objects
    decal_count = sum(1 for obj in objects.values() if obj.is_text_decal)
    stopdss1_count = sum(1 for obj in objects.values() if obj.is_text_decal_stopdss1)
    print(f"Found {decal_count} text_decal objects")
    print(f"Found {stopdss1_count} text_decalStopDSS1 objects")
    
    # Get all level files
    level_files = sorted(glob.glob('./Data/level*.txt'))
    print(f"\nFound {len(level_files)} level files")
    
    total_changes = 0
    
    for level_file in level_files:
        print(f"\nProcessing {os.path.basename(level_file)}...")
        try:
            changes = update_level_file(level_file, objects)
            total_changes += changes
        except Exception as e:
            print(f"  Error: {e}")
            import traceback
            traceback.print_exc()
    
    print(f"\n{'=' * 60}")
    print(f"Summary: Made {total_changes} position changes across all level files")
    print("Done!")


if __name__ == '__main__':
    main()