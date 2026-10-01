#!/usr/bin/env python3
"""
bridgewall - find the prop_bridgewall1 scene instances that carry a material
override, so they can be repointed at Voxen's prop_bridgewall2 (856, red
citmat2_1) / prop_bridgewall3 (857, blue citmat1_3).

The level-data exporter drops m_Materials overrides, so a bridgewall that Unity
renders red arrives in Voxen as the default-textured prop_bridgewall1 (537).
The override only survives in the scene file, as a PrefabInstance
m_Modifications entry.

    python3 Tools/bridgewall.py
"""
import os
import re
import sys
from collections import Counter, defaultdict

CITADEL = "/home/qmaster/ai-workspaces/Citadel/Assets"
SCENE = f"{CITADEL}/Scenes/CitadelScene.unity"
VOXEN = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

PREFAB_GUID = "a0694cfe8a834ae4db420d9cdbde6bac"   # prop_bridgewall1.prefab
MAT = {
    "02962ceef1b0aea45ab05fcb8586291a": "citmat2_1",  # red
    "45ad6a50ffd11644998aaa8f3364f66c": "citmat1_3",  # blue
    "3195c3e6d92b62c478ce0735ad52d559": "seat1_7",    # prefab default
}
# Every citmat guid, so unknown overrides are still reported rather than dropped.
MAT_RE = re.compile(r"objectReference: \{fileID: \d+, guid: ([0-9a-f]{32}), type: 2\}")

POS_RE = {
    a: re.compile(rf"propertyPath: m_LocalPosition\.{a}\n\s*value: (-?[\d.eE+]+)")
    for a in "xyz"
}


def blocks():
    with open(SCENE, "r", errors="replace") as fh:
        buf = []
        for line in fh:
            if line.startswith("--- !u!1001"):
                if buf:
                    yield "".join(buf)
                buf = [line]
            elif buf:
                buf.append(line)
        if buf:
            yield "".join(buf)


def main():
    total = 0
    kinds = Counter()
    by_mat = defaultdict(list)
    for b in blocks():
        if f"guid: {PREFAB_GUID}" not in b:
            continue
        total += 1
        # Material overrides are the m_Modifications entries whose propertyPath
        # is m_Materials.Array.data[N]; the scene also stores light/probe
        # references as objectReference, so key off the propertyPath line above.
        mats = []
        for mt in re.finditer(
                r"propertyPath: (m_Materials\.Array\.data\[\d+\])\n"
                r"\s*value:[^\n]*\n"
                r"\s*objectReference: \{fileID: \d+, guid: ([0-9a-f]{32})", b):
            mats.append(mt.group(2))
        if not mats:
            kinds["(no override)"] += 1
            continue
        names = tuple(sorted(MAT.get(g, g[:8]) for g in mats))
        kinds[names] += 1
        pos = []
        for a, rx in POS_RE.items():
            m = rx.search(b)
            pos.append(float(m.group(1)) if m else None)
        by_mat[names].append(pos)

    print(f"prop_bridgewall1 PrefabInstances in scene: {total}")
    print()
    for k, n in kinds.most_common():
        print(f"  {n:4d}  {k}")
    print()
    for names, poss in sorted(by_mat.items(), key=lambda kv: -len(kv[1])):
        print(f"--- {names}: {len(poss)} instances ---")
        for p in poss[:8]:
            print("    " + "  ".join("?" if v is None else f"{v:10.4f}" for v in p))
        if len(poss) > 8:
            print(f"    ... and {len(poss) - 8} more")


if __name__ == "__main__":
    main()
