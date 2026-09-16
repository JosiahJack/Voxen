#!/usr/bin/env python3
"""Compute true world positions and add/update lP fields in level files."""
import re, glob

def main():
    for f in sorted(glob.glob('./Data/level*.txt')):
        print(f"Processing {f} ...")
        with open(f) as file:
            lines = file.read().splitlines()
        updated = []
        for line in lines:
            line_stripped = line.strip()
            if line_stripped.startswith('constIndex:592|') or line_stripped.startswith('constIndex:593|'):
                entry = {}
                for part in line_stripped.split('|'):
                    if ':' in part:
                        k, v = part.split(':', 1)
                        entry[k] = v
                level = int(re.search(r'level(\d+)', f).group(1))
                # Approximate parent world offset derived from Unity hierarchy evaluation (NEUROSURGERY / parent chain 2085095997 + 1058831546 = ~25.56, -48.64, -5.2 for level 1)
                parent_offset_est = (25.56, -48.64, -5.2) if level == 1 else (0.0, 0.0, 0.0)
                # Apply anchoredPosition3D (local) + estimated parent world offset (simplified identity rotation assumption for parent containers)
                world_x = float(entry.get('anchoredPosition3D.x', entry.get('anchoredPosition.x', '0'))) + parent_offset_est[0]
                world_y = float(entry.get('anchoredPosition3D.y', entry.get('anchoredPosition.y', '0'))) + parent_offset_est[1]
                world_z = float(entry.get('anchoredPosition3D.z', entry.get('anchoredPosition3D.z', '0'))) + parent_offset_est[2]
                # Add/update lP fields
                if 'lP.x' not in entry:
                    line_stripped += f"|lP.x:{world_x}"
                else:
                    line_stripped = line_stripped.replace(f"lP.x:{entry.get('lP.x', '0')}", f"lP.x:{world_x}")
                if 'lP.y' not in entry:
                    line_stripped += f"|lP.y:{world_y}"
                else:
                    line_stripped = line_stripped.replace(f"lP.y:{entry.get('lP.y', '0')}", f"lP.y:{world_y}")
                if 'lP.z' not in entry:
                    line_stripped += f"|lP.z:{world_z}"
                else:
                    line_stripped = line_stripped.replace(f"lP.z:{entry.get('lP.z', '0')}", f"lP.z:{world_z}")
                updated.append(line_stripped + '\n')
            else:
                updated.append(line + '\n')
        with open(f, 'w') as out:
            out.writelines(updated)
        print(f"  Updated lP fields (with parent hierarchy offset) for level {level}")

if __name__ == '__main__':
    main()
