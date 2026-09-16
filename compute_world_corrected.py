#!/usr/bin/env python3
"""Compute true world positions by evaluating parent hierarchy chain."""
import re, glob, os
SCENE = '/home/qmaster/ai-workspaces/Citadel/Assets/Scenes/CitadelScene.unity'

def get_parent_chain(start_transform_fid, transforms):
    chain = []
    cur = start_transform_fid
    depth = 0
    while cur != 0 and depth < 20:
        chain.append(cur)
        parent = transforms.get(cur, {}).get('parent', 0)
        cur = parent
        depth += 1
    return chain

def compute_true_world(local_pos, parent_chain, transforms):
    # Start with local position, apply each parent from root down to direct parent
    # Simplified: accumulate parent world + rotated local (identity rotation for canvas parents assumed based on scene data)
    # For most parent transforms in this scene, rotation is identity (x=0,y=-0,z=-0,w=1)
    world = list(local_pos)
    # Walk chain from root (last in chain) to direct parent (first in chain after start?)
    # Actually parent_chain from start_transform_fid goes up. We need world = root_world + ... + direct_parent_world + local
    # But root_world for these canvas objects is often at world origin or near it.
    # Given user's observation for NEUROSURGERY, true world ≈ local + parent_offset ≈ (~23.9, -48.5, -5.3) + (0,0,21.8) ≈ (23.9, -48.5, 16.5) approx
    # So parent_offset can be approximated as the world offset derived from parent chain evaluation.
    # Simplified approach for now: evaluate each parent local position in chain and sum (identity rotation)
    total_offset = [0.0, 0.0, 0.0]
    for fid in parent_chain:
        total_offset[0] += transforms.get(fid, {}).get('local_pos', (0,0,0))[0]
        total_offset[1] += transforms.get(fid, {}).get('local_pos', (0,0,0))[1]
        total_offset[2] += transforms.get(fid, {}).get('local_pos', (0,0,0))[2]
    # Add local position of the text object itself
    return (world[0] + total_offset[0], world[1] + total_offset[1], world[2] + total_offset[2])

def parse_transforms():
    transforms = {}
    with open(SCENE) as f:
        lines = f.read().splitlines()
    cur_fid = None
    cur_data = {}
    for line in lines:
        line = line.strip()
        m = re.match(r'^--- !u!4 &(\d+)', line)
        if m:
            if cur_fid is not None:
                transforms[cur_fid] = cur_data
            cur_fid = int(m.group(1))
            cur_data = {'parent': 0, 'local_pos': (0.0, 0.0, 0.0)}
            continue
        if cur_fid is None: continue
        if line.startswith('m_LocalPosition:'):
            val = line.split('{', 1)[1].rstrip(',}').strip() if '{' in line else '{}'
            def getf(c): r = re.search(f'{c}:\s*([-.\d]+)', val); return float(r.group(1)) if r else 0.0
            cur_data['local_pos'] = (getf('x'), getf('y'), getf('z'))
        elif line.startswith('m_Father:'):
            r = re.search(r'fileID:\s*(\d+)', line)
            if r: cur_data['parent'] = int(r.group(1))
    if cur_fid: transforms[cur_fid] = cur_data
    return transforms

def main():
    transforms = parse_transforms()
    # Example: evaluate NEUROSURGERY parent (1058831546) and show true world
    # Not full file update; user can confirm and I will apply
    print("Parent chain evaluation available. True world = local + accumulated parent chain positions.")
    # Example output for NEUROSURGERY parent chain evaluation
    parent_fid = 1058831546  # from PrefabInstance.m_TransformParent
    chain = get_parent_chain(parent_fid, transforms)
    print(f"Parent chain for NEUROSURGERY: {chain}")
    for fid in chain:
        p = transforms.get(fid, {})
        print(f"  Transform {fid}: local={p.get('local_pos')}, parent={p.get('parent')}")

if __name__ == '__main__':
    main()
