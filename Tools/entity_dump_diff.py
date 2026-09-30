"""Diff two Voxen entity dumps produced by VOXEN_DUMP_ENTITIES.

    VOXEN_DUMP_ENTITIES=/tmp/a.ent ./voxen     # build A
    VOXEN_DUMP_ENTITIES=/tmp/b.ent ./voxen     # build B
    Tools/entity_dump_diff.py /tmp/a.ent /tmp/b.ent

Raw Entity structs are dumped by entity.c so this sees every field, including
ones neither tool was told about; the field table comes from the compiler via
entity_layout, so a struct change cannot desync the decode.

Pointer-typed members are excluded: they hold addresses whose values vary
between runs and would drown the signal in noise.  Everything else is compared
byte for byte and reported by field name, so a loader regression shows up as
"level 6, field direction, 78 instances" rather than a wall of hex.

Model bounds are derived from the files in Models/, so re-exporting a mesh
between the two runs changes entity state and will show up here.  Check asset
mtimes before believing a modelBounds/shadRadius/radius difference.

Exit 0 when identical, 1 when they differ.
"""
import os
import struct
import sys
import collections

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from entity_layout import entity_fields

MAGIC = b"VOXENED1"
INT_TYPES = {
    "bool": (1, False), "u8": (1, False), "i8": (1, True),
    "u16": (2, False), "i16": (2, True),
    "u32": (4, False), "i32": (4, True), "int": (4, True),
    "u64": (8, False), "i64": (8, True), "size_t": (8, False),
}
FLOAT_TYPES = {"float": ("f", 4), "double": ("d", 8)}

# Fields whose value varies between two runs of the *same* build, so they cannot
# participate in a build-vs-build comparison.  Each entry records why.
NONDETERMINISTIC = {
    "aiThinkFinished": "ai.c:203 sets it to pauseRelativeTime + AI_TICK_TIME + "
                       "random_range(0.0f, 1.0f) -- wall clock plus RNG",
    "tickFinished": "citadel.c:231 sets it to pauseRelativeTime + 0.05 + random_range()",
    "intervalFinished": "same pause-relative-time-plus-RNG idiom as tickFinished",
    "contents": "AI inventory slots, filled during AI init (ai.c:588); -1 when empty",
}

# Fields where only specific bits vary between runs.  Masked out of the
# comparison rather than skipping the whole field, so the remaining flags stay
# under test.
NONDETERMINISTIC_BITS = {
    "entflags": 0x1,  # EF_ACTIVE: physics.c:39 CyberMineInitBeforeLoad deactivates each
                     # cybermine (constIndex 480) when random_range() beats the
                     # difficulty threshold, so every run leaves a different subset
                     # of the mines inactive.
}

MASK_WHY = {
    "entflags": "EF_ACTIVE -- physics.c:39 CyberMineInitBeforeLoad deactivates each "
                "cybermine (constIndex 480) when random_range() beats the difficulty "
                "threshold, so every run leaves a different subset inactive",
}


def is_pointer(ctype):
    return "*" in ctype


def decode(raw, ctype):
    """Best-effort human-readable value for one field's bytes."""
    ctype = ctype.replace(" ", "")
    if ctype in INT_TYPES:
        width, signed = INT_TYPES[ctype]
        return int.from_bytes(raw[:width], "little", signed=signed)
    if ctype in FLOAT_TYPES:
        fmt, width = FLOAT_TYPES[ctype]
        return round(struct.unpack("<" + fmt, raw[:width])[0], 5)
    m = ctype.split("[")
    if len(m) == 2 and m[1].endswith("]"):
        base, count = m[0], int(m[1][:-1])
        n = count * INT_TYPES.get(base, (1, False))[0]
        return raw[:n].hex()
    if ctype in ("V3", "Color3", "Quaternion", "Color", "V2"):
        n = len(raw) // 4
        vals = struct.unpack("<%df" % n, raw[: n * 4])
        return "(" + ",".join("%g" % v for v in vals) + ")"
    return raw.hex()


MAX_LEVELS = 14
LIGHT_COUNT = 2048


def read_dump(path, fields):
    data = open(path, "rb").read()
    if data[:8] != MAGIC:
        raise SystemExit("%s: not a Voxen entity dump" % path)
    esz, nlev = struct.unpack_from("<II", data, 8)
    off = 16
    levels = {}
    for _ in range(nlev):
        lev, = struct.unpack_from("<I", data, off)
        count, = struct.unpack_from("<H", data, off + 4)
        off += 8
        blob = data[off:off + count * esz]
        off += count * esz
        ents = []
        for i in range(count):
            base = i * esz
            ents.append({
                f[0]: blob[base + f[1]: base + f[1] + f[2]] for f in fields
            })
        levels[lev] = ents

    # Trailing sections: the same loader writes lights, light positions, light
    # animations and decal staging, and level-data purging touches all of them, so
    # the differ compares them byte-for-byte.  Identical bytes is a stronger
    # statement than a per-field comparison, so no field decoding is needed here.
    tail = {}

    def take(name, esz_bytes, count=None, count_fmt="<I"):
        nonlocal off
        if count is None:
            count, = struct.unpack_from(count_fmt, data, off)
            off += struct.calcsize(count_fmt)
        blob = data[off:off + esz_bytes * count]
        off += esz_bytes * count
        tail[name] = blob

    lsz, = struct.unpack_from("<I", data, off); off += 4
    take("lights", lsz, nlev * LIGHT_COUNT)
    v3sz, = struct.unpack_from("<I", data, off); off += 4
    take("lightpos", v3sz, nlev * LIGHT_COUNT)
    lasz, = struct.unpack_from("<I", data, off); off += 4
    nlanim, = struct.unpack_from("<I", data, off); off += 4
    take("lanims", lasz, nlanim)
    dsz, = struct.unpack_from("<I", data, off); off += 4
    take("decalStyles", dsz, None, "<H")
    tsz, = struct.unpack_from("<I", data, off); off += 4
    take("decalText", tsz, None, "<H")

    if off != len(data):
        raise SystemExit("%s: %d trailing bytes" % (path, len(data) - off))
    return esz, levels, tail


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    a_path, b_path = sys.argv[1], sys.argv[2]

    all_fields, sizeof = entity_fields()
    fields = [f for f in all_fields
              if not is_pointer(f[3]) and f[0] not in NONDETERMINISTIC]
    print("Entity: %d bytes, %d fields compared" % (sizeof, len(fields)))
    for name, why in sorted(NONDETERMINISTIC.items()):
        print("  ignoring %-16s %s" % (name, why))
    for name, mask in sorted(NONDETERMINISTIC_BITS.items()):
        print("  ignoring %-16s bits 0x%x -- %s"
              % (name, mask, MASK_WHY[name]))
    for name, off, size, ctype in all_fields:
        if is_pointer(ctype):
            print("  ignoring %-16s pointer-typed (address varies per run)" % name)

    _, A, tailA = read_dump(a_path, fields)
    _, B, tailB = read_dump(b_path, fields)

    all_lev = sorted(set(A) | set(B))
    field_diffs = collections.Counter()
    field_examples = {}
    count_notes = {}

    for lev in all_lev:
        ea, eb = A.get(lev, []), B.get(lev, [])
        na, nb = len(ea), len(eb)
        if na != nb:
            count_notes[lev] = "instance count %d -> %d (%+d)" % (na, nb, nb - na)
        for i in range(min(na, nb)):
            for name, off, size, ctype in fields:
                va, vb = ea[i][name], eb[i][name]
                mask = NONDETERMINISTIC_BITS.get(name)
                if mask is not None and size <= 4:
                    u = struct.unpack("<I", (va + b"\0" * 4)[:4])[0]
                    v = struct.unpack("<I", (vb + b"\0" * 4)[:4])[0]
                    va = struct.pack("<I", u & ~mask)
                    vb = struct.pack("<I", v & ~mask)
                if va == vb:
                    continue
                key = (name, ctype)
                field_diffs[key] += 1
                field_examples.setdefault(key, (lev, i, decode(va, ctype), decode(vb, ctype)))

    print("instances: %d -> %d" % (sum(len(v) for v in A.values()),
                                   sum(len(v) for v in B.values())))
    for lev, note in count_notes.items():
        print("  level %-2d %s" % (lev, note))

    tail_diffs = []
    for name in sorted(tailA):
        ba, bb = tailA[name], tailB[name]
        if ba == bb:
            continue
        n = sum(1 for x, y in zip(ba, bb) if x != y) + abs(len(ba) - len(bb))
        first = next((i for i, (x, y) in enumerate(zip(ba, bb)) if x != y), None)
        tail_diffs.append((name, n, len(ba), first))

    if not field_diffs and not tail_diffs:
        print("\nno field differences")
        print("lights/lightpos/lanims/decalStyles/decalText: byte-identical")
        return 0

    if tail_diffs:
        print("\ntrailing sections that differ (byte comparison):")
        for name, n, total, first in tail_diffs:
            print("  %-14s %6d of %d bytes differ, first at offset %s"
                  % (name, n, total, first))

    if not field_diffs:
        print("\nRESULT: DIFFER (only in lights/decals, %d bytes)"
              % sum(d[1] for d in tail_diffs))
        return 1

    print("\nfields that differ:")
    for (name, ctype), count in field_diffs.most_common():
        lev, idx, va, vb = field_examples[(name, ctype)]
        print("  %-24s %-10s %6d instance(s)   e.g. level %d #%d: %s -> %s"
              % (name, ctype, count, lev, idx, va, vb))
    print("\nRESULT: DIFFER (%d field differences)" % sum(field_diffs.values()))
    return 1


if __name__ == "__main__":
    sys.exit(main())