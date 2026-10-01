#!/usr/bin/env python3
"""
constmap - cross-check the Citadel constIndex table against Voxen's EDefs.

    python3 Tools/constmap.py names          # print constIndex -> prefab, both sources
    python3 Tools/constmap.py diff           # print indices where the names disagree
    python3 Tools/constmap.py levels         # per-level placed-count comparison
"""
import os
import re
import sys
from collections import Counter, defaultdict

CITADEL = "/home/qmaster/ai-workspaces/Citadel"
PREFAB_DATA = os.path.join(CITADEL, "ConversionData/Prefabs")
STREAM = os.path.join(CITADEL, "Assets/StreamingAssets")
VOXEN = "/home/qmaster/ai-workspaces/Voxen"

# prefab name -> constIndex, from ConversionData/Prefabs/<name>.txt headers
RE_CONST = re.compile(r"^# constIndex: (\d+)\s*$")


def citadel_names():
    names = {}
    dupes = []
    for fn in sorted(os.listdir(PREFAB_DATA)):
        if not fn.endswith(".txt"):
            continue
        prefab = fn[:-4]
        with open(os.path.join(PREFAB_DATA, fn)) as fh:
            for line in fh:
                m = RE_CONST.match(line)
                if m:
                    ci = int(m.group(1))
                    if ci in names and names[ci] != prefab:
                        dupes.append((ci, names[ci], prefab))
                    names[ci] = prefab
                    break
    return names, dupes


def voxen_names():
    """Parse the /*N name*/[N]={...} comments in entity.c's EDefs table."""
    src = open(os.path.join(VOXEN, "entity.c")).read()
    out = {}
    for comment, name, idx, _model in re.findall(
            r"/\*(\d+) ([^*]*)\*/\[(\d+)\]=\{(\d+),", src):
        out[int(idx)] = name.strip()
    return out


def _positions(rec):
    """(x, y, z) from either key scheme: Citadel uses localPosition.*, Voxen lP.*."""
    p = ("localPosition." if "localPosition.x" in rec else "lP.")
    return tuple(round(float(rec[p + a]), 3) for a in "xyz")


def stream_records(level):
    """All placed records in a Citadel level, from every StreamingAssets file.

    The name field is optional: the geometry/dynamics exports carry
    "constIndex:162|chunk_med1_7 (25)|localPosition..." but the staticobjects
    export starts straight at "constIndex:537|localPosition...", so the
    position keys are read out of every segment rather than from a fixed index.
    """
    recs = []
    for fn in os.listdir(STREAM):
        m = re.match(r"CitadelScene_\w+_level(\d+)\.txt$", fn)
        if not m or int(m.group(1)) != level:
            continue
        with open(os.path.join(STREAM, fn)) as fh:
            for line in fh:
                if not line.startswith("constIndex:"):
                    continue
                parts = line.rstrip("\n").split("|")
                rec = {"constIndex": int(parts[0].split(":")[1])}
                for seg in parts[1:]:
                    if ":" not in seg:
                        continue
                    k, _, v = seg.partition(":")
                    if k == "constIndex":
                        continue
                    if k == "prefab":
                        rec["prefab"] = v.split(" (")[0]
                    elif k.startswith("localPosition."):
                        rec[k] = v
                    elif k.startswith("localRotation."):
                        rec[k] = v
                    elif k.startswith("localScale."):
                        rec[k] = v
                if "prefab" not in rec and len(parts) > 1 and ":" not in parts[1]:
                    rec["prefab"] = parts[1].split(" (")[0]
                if "localPosition.x" in rec:
                    recs.append(rec)
    return recs


def voxen_records(level):
    recs = []
    with open(os.path.join(VOXEN, f"Data/level{level}.txt")) as fh:
        for line in fh:
            if not line.startswith("constIndex:"):
                continue
            parts = line.rstrip("\n").split("|")
            rec = {"constIndex": int(parts[0].split(":")[1])}
            for kv in parts[1:]:
                k, _, v = kv.partition(":")
                rec[k] = v
            recs.append(rec)
    return recs


def cmd_names():
    cn, dupes = citadel_names()
    vn = voxen_names()
    for ci in sorted(set(cn) | set(vn)):
        a, b = cn.get(ci, "-"), vn.get(ci, "-")
        mark = " " if a == b else "*"
        print(f"{mark}{ci:4d}  Citadel={a:38s} Voxen={b}")
    if dupes:
        print("\n# duplicate constIndex headers in ConversionData:", file=sys.stderr)
        for ci, a, b in dupes:
            print(f"  {ci}: {a} vs {b}", file=sys.stderr)


def cmd_diff():
    cn, _ = citadel_names()
    vn = voxen_names()
    n = 0
    for ci in sorted(set(cn) | set(vn)):
        a, b = cn.get(ci), vn.get(ci)
        if a != b:
            n += 1
            print(f"{ci:4d}  Citadel={a or '-':38s} Voxen={b or '-'}")
    print(f"\n{n} mismatched indices of {len(set(cn) | set(vn))}")


def _exact_offset(vox, cit):
    """Translation explaining the most Voxen positions, plus that count.

    Nearest-neighbour is unusable here: the props sit on a 1.28 lattice, so ties
    are everywhere.  Brute-forcing every (vox, cit) pair and keeping the offset
    that matches the most records is exact and cheap at these set sizes.
    """
    if not vox or not cit:
        return None, 0
    cs = {_positions(r) for r in cit}
    best = (0, None)
    for a in vox:
        for b in cit:
            o = tuple(round(_positions(a)[i] - _positions(b)[i], 3) for i in range(3))
            n = sum(1 for r in vox
                    if tuple(round(_positions(r)[i] - o[i], 3) for i in range(3)) in cs)
            if n > best[0]:
                best = (n, o)
    return best[1], best[0]


def cmd_screens():
    """Per-level chunk_screen reconciliation, with console proximity.

    prop_console02 carries a child GameObject named "screen", so its console
    panels plausibly account for chunk_screen records that Citadel does not
    export.  This reports matched/extra/missing and how far each stray sits
    from the nearest console, which is what settles whether that holds.
    """
    cn, _ = citadel_names()
    for level in range(14):
        sv = [r for r in stream_records(level) if r["constIndex"] == 279]
        vv = [r for r in voxen_records(level) if r["constIndex"] == 279]
        if len(sv) == len(vv):
            continue
        consoles = [_positions(r) for r in stream_records(level) if r["constIndex"] == 526]
        off, matched = _exact_offset(vv, sv)
        if off is None:
            print(f"level {level:2d}: Voxen={len(vv)} Citadel={len(sv)}  (one side empty)")
            continue
        cs = {_positions(r) for r in sv}
        extra = [tuple(round(_positions(r)[i] - off[i], 3) for i in range(3))
                 for r in vv
                 if tuple(round(_positions(r)[i] - off[i], 3) for i in range(3)) not in cs]
        missing = sorted(cs - {tuple(round(_positions(r)[i] - off[i], 3) for i in range(3))
                               for r in vv})
        near = [e for e in extra
                if consoles and min(sum((e[i] - q[i]) ** 2 for i in range(3)) ** 0.5
                                   for q in consoles) < 1.0]
        print(f"level {level:2d}: Voxen={len(vv):3d} Citadel={len(sv):3d} "
              f"matched={matched:3d} extra={len(extra)} missing={len(missing)} "
              f"consoles={len(consoles)} extras_within_1m_of_console={len(near)}")
        for e in extra:
            print(f"        extra   {e}")
        for mpos in missing:
            print(f"        missing {mpos}")


# prefabs whose Citadel instances have no Voxen counterpart by design, so the
# levels report does not keep flagging them.
IGNORED_PREFIXES = ("prop_vending",)


def cmd_levels():
    cn, _ = citadel_names()
    for level in range(14):
        sv = Counter(r["constIndex"] for r in stream_records(level))
        vv = Counter(r["constIndex"] for r in voxen_records(level))
        diffs = []
        for ci in sorted(set(sv) | set(vv), key=lambda v: int(v)):
            nm = cn.get(ci, "?")
            if nm.startswith(IGNORED_PREFIXES):
                continue
            if sv.get(ci, 0) != vv.get(ci, 0):
                diffs.append((ci, nm, sv.get(ci, 0), vv.get(ci, 0)))
        if diffs:
            print(f"--- level {level}: {len(diffs)} constIndex count differences")
            for ci, nm, a, b in diffs:
                print(f"    {ci:4d} {nm:38s} Citadel={a:5d} Voxen={b:5d}")


if __name__ == "__main__":
    {"names": cmd_names, "diff": cmd_diff, "levels": cmd_levels,
     "screens": cmd_screens}[sys.argv[1]]()