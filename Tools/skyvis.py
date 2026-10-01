#!/usr/bin/env python3
"""
skyvis - decode Data/worldcellskyvis_<level>.png and report which cells the engine
will treat as sky-visible, cross-referenced against level-data placements.

Voxen decodes a 64x64 palette PNG where the palette is 3 states:
  red   (r>127, g<127, b<127) -> no sky, no sun
  blue  (r<=127,g<=127,b>127) -> sky + sun
  white/black                 -> sun only
and maps pixel (x, y) to cell (x, 63-y) because the PNG is top-left origin and
the cell grid is bottom-left origin.

    python3 Tools/skyvis.py 10 --at 93
"""
import argparse
import math
import os
import struct
import zlib
from collections import defaultdict

VOXEN = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# voxen.c:780 levMins
LEV_MINS = [(-37.3600, -52.7600), (-53.8000, -64.0800), (-46.12, -56.34),
            (-51.266, -51.246), (-29.462, -53.7872), (-47.3622, -55.04),
            (-65.94, -71.6833), (-66.8989, -82.0144), (-43.7456, -43.9872),
            (-51.5039, -69.0306), (-24.0994, -39.7972), (-27.1772, -28.3394),
            (-18.05, -30.50), (-64.000, -60.120)]
CELLSZ = 2.56
CELLXHALF = CELLSZ * 0.5
WORLDX = WORLDZ = 64


def load_png(path):
    d = open(path, "rb").read()
    off, idat, plte = 8, b"", b""
    w = h = bd = ct = 0
    while off < len(d):
        ln, typ = struct.unpack(">I4s", d[off:off + 8])
        body = d[off + 8:off + 8 + ln]
        if typ == b"IHDR":
            w, h, bd, ct = struct.unpack(">IIBB", body[:10])
        elif typ == b"PLTE":
            plte = body
        elif typ == b"IDAT":
            idat += body
        elif typ == b"IEND":
            break
        off += 12 + ln
    assert ct in (0, 3), f"unsupported PNG colortype {ct}"
    raw = zlib.decompress(idat)
    bpp_bits = bd
    row_bytes = (w * bpp_bits + 7) // 8
    mask = (1 << bd) - 1
    pal = [tuple(plte[i:i + 3]) for i in range(0, len(plte), 3)]
    px, prev, pos = [], bytearray(row_bytes), 0
    for _ in range(h):
        f = raw[pos]; pos += 1
        line = bytearray(raw[pos:pos + row_bytes]); pos += row_bytes
        for i in range(row_bytes):
            a = line[i - bpp_bits // 8] if i >= bpp_bits // 8 else 0
            b = prev[i]
            c = prev[i - bpp_bits // 8] if i >= bpp_bits // 8 else 0
            if f == 1: line[i] = (line[i] + a) & 0xFF
            elif f == 2: line[i] = (line[i] + b) & 0xFF
            elif f == 3: line[i] = (line[i] + (a + b) // 2) & 0xFF
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pr) & 0xFF
        row = []
        for x in range(w):
            if bd == 8:
                row.append(line[x])
            else:
                bit = x * bd
                byte = line[bit >> 3]
                shift = 8 - bd - (bit & 7)
                row.append((byte >> shift) & mask)
        if ct == 3:
            px.append([pal[i] for i in row])
        else:
            px.append([(g, g, g) for g in row])  # greyscale: nonzero == open
        prev = line
    return w, h, px


def cell_state(r, g, b):
    """Mirror of culling.c:44."""
    if r > 127 and g < 127 and b < 127:
        return "RED"      # no sky, no sun
    if r <= 127 and g <= 127 and b > 127:
        return "BLUE"     # sky + sun
    return "SUN"          # sun only


def grid(level):
    w, h, px = load_png(os.path.join(VOXEN, f"Data/worldcellskyvis_{level}.png"))
    assert (w, h) == (WORLDX, WORLDZ), f"expected 64x64, got {w}x{h}"
    # PIXEL_IDX(x,z) = (x + ((WORLDZ-1-z) * WORLDX)) * 4
    return {(x, z): cell_state(*px[WORLDZ - 1 - z][x])
            for x in range(WORLDX) for z in range(WORLDZ)}


def cell_of(level, x, z):
    wmin_x, wmin_z = LEV_MINS[level]
    cx = min(max(int(math.floor((x - wmin_x + CELLXHALF) / CELLSZ)), 0), WORLDX - 1)
    cz = min(max(int(math.floor((z - wmin_z + CELLXHALF) / CELLSZ)), 0), WORLDZ - 1)
    return cx, cz


def placements(level, only=None):
    out = []
    path = os.path.join(VOXEN, f"Data/level{level}.txt")
    for line in open(path):
        if not line.startswith("constIndex:"):
            continue
        p = line.rstrip("\n").split("|")
        ci = int(p[0].split(":")[1])
        if only is not None and ci not in only:
            continue
        kv = dict(x.split(":", 1) for x in p[1:] if ":" in x)
        if "lP.x" not in kv:
            continue
        out.append((ci, float(kv["lP.x"]), float(kv["lP.y"]), float(kv["lP.z"])))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("level", type=int)
    ap.add_argument("--at", type=int, action="append",
                    help="constIndex to report cells for (repeatable)")
    ap.add_argument("--map", action="store_true", help="dump the whole grid")
    args = ap.parse_args()

    g = grid(args.level)
    tally = defaultdict(int)
    for s in g.values():
        tally[s] += 1
    print(f"level {args.level} skyvis: " +
          "  ".join(f"{k}={tally[k]}" for k in ("BLUE", "SUN", "RED")))

    if args.map:
        for z in range(WORLDZ - 1, -1, -1):
            row = "".join({"BLUE": "B", "SUN": ".", "RED": "#"}[g[(x, z)]]
                          for x in range(WORLDX))
            print(f"{z:2d} {row}")
        print("   " + "".join(str(x // 10 % 10) for x in range(WORLDX)))
        print("   " + "".join(str(x % 10) for x in range(WORLDX)))

    for ci in (args.at or []):
        cells = defaultdict(list)
        for c, x, y, z in placements(args.level, {ci}):
            cells[cell_of(args.level, x, z)].append((x, y, z))
        print(f"\nconstIndex {ci}: {sum(len(v) for v in cells.values())} instances "
              f"across {len(cells)} cells")
        st = defaultdict(int)
        for (cx, cz), v in sorted(cells.items()):
            st[g[(cx, cz)]] += 1
        print("  " + "  ".join(f"{k}={v}" for k, v in sorted(st.items())))
        for (cx, cz), v in sorted(cells.items()):
            if g[(cx, cz)] == "BLUE":
                continue
            print(f"  cell ({cx:2d},{cz:2d}) = {g[(cx,cz)]:4s} "
                  f"{len(v):3d} inst  e.g. {tuple(round(q,2) for q in v[0])}")


if __name__ == "__main__":
    main()
