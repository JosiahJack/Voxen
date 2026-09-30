"""Authoritative Entity field table, extracted from the compiler.

Both Tools/parse_coverage_check.py and Tools/entity_dump_diff.py need to know
Entity's real field names and byte offsets.  Parsing the typedef by regex is
fragile (nested typedefs, arrays, bitfields, comments in the middle of a
declaration), so the table is taken from clang's -fdump-record-layouts
instead: it can never disagree with what the compiler actually does.

Only the AST block is used; the IRgen blocks are skipped.
"""
import os
import re
import subprocess
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PROBE = os.path.join(ROOT, "Tools", "entity_layout.c")
LAYOUT_LINE = re.compile(r"^\s*(\d+) \| (.*)$")
SIZE_LINE = re.compile(r"\|\s*\[sizeof=(\d+), align=(\d+)\]")

# Scalar type -> declared byte width.  The struct's tail padding must not be
# mistaken for the width of the last member (e.g. `u8 puzzleSolved` at +916 in
# a 920-byte struct is one byte wide, not four), so widths come from the
# declared type and only fall back to the inter-field gap when unknown.
SCALARS = {
    "bool": 1, "u8": 1, "i8": 1, "char": 1,
    "u16": 2, "i16": 2, "short": 2,
    "u32": 4, "i32": 4, "int": 4, "float": 4, "enum": 4,
    "u64": 8, "i64": 8, "double": 8, "long": 8, "size_t": 8,
    "V2": 8, "V3": 12, "Color": 16, "Color3": 12, "Quaternion": 16,
    "ColliderType": 1, "AttType": 4, "EmitFlag": 4,
}


def _declared_size(ctype):
    m = re.match(r"^([\w ]+?)\s*\[\s*(\d+)\s*\]$", ctype)
    if m:
        base = SCALARS.get(m.group(1))
        return int(m.group(2)) * base if base else None
    return SCALARS.get(ctype)


def _dump_record_layouts():
    """Return clang's record-layout dump as text."""
    # The -o path is part of zig's cache key, so a fixed output path makes the
    # second run of this function return an empty dump (zig replays the cached
    # object and never invokes clang, which is where -Xclang output comes
    # from).  A fresh temp path each call keeps the probe honest.  -S rather
    # than -c: with -c zig can satisfy the job entirely from cache too.
    out = tempfile.NamedTemporaryFile(suffix=".s", delete=False)
    out.close()
    cmd = [
        "zig", "cc", "-Xclang", "-fdump-record-layouts", "-S",
        PROBE, "-I" + ROOT, "-nostdinc", "-std=c11", "-o", out.name,
    ]
    try:
        p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                           text=True, timeout=300)
        if p.returncode != 0:
            raise SystemExit("layout probe failed:\n" + p.stdout)
        return p.stdout
    finally:
        os.unlink(out.name)


def entity_fields():
    """[(name, offset, size, ctype), ...] in declaration order, plus sizeof."""
    dump = _dump_record_layouts()

    # Entity is `typedef struct {...} Entity;`, so the block header is a bare
    # `0 | Entity` rather than `0 | struct Entity`.
    start = None
    for i, line in enumerate(dump.splitlines()):
        if re.match(r"^\s*0 \| Entity\s*$", line):
            start = i + 1
            break
    if start is None:
        raise SystemExit("no `0 | Entity` record in the layout dump")

    fields = []
    size = None
    for line in dump.splitlines()[start:]:
        if line.startswith("*** Dumping"):  # next record, we already have ours
            break
        m = SIZE_LINE.search(line)
        if m:
            size = int(m.group(1))
            break
        m = LAYOUT_LINE.match(line)
        if not m:
            continue
        rest = m.group(2).strip()
        rest = re.sub(r":\s*\d+$", "", rest)          # strip bitfield width
        parts = rest.split()
        if len(parts) < 2:
            continue                                  # anonymous padding member
        name = parts[-1]
        ctype = " ".join(parts[:-1])
        fields.append((name, int(m.group(1)), ctype))

    if size is None:
        raise SystemExit("layout dump ended without [sizeof=..]")

    out = []
    for i, (name, off, ctype) in enumerate(fields):
        gap = (fields[i + 1][1] if i + 1 < len(fields) else size) - off
        declared = _declared_size(ctype)
        out.append((name, off, declared or gap, ctype))
    out.sort(key=lambda f: f[1])
    return out, size


def field_names():
    return {name for name, _, _, _ in entity_fields()[0]}


if __name__ == "__main__":
    fields, size = entity_fields()
    print("sizeof(Entity) = %d, %d fields" % (size, len(fields)))
    for name, off, n, ctype in fields[:8]:
        print("  +%-4d %-6d %-24s %s" % (off, n, ctype, name))
    print("  ...")
    for name, off, n, ctype in fields[-5:]:
        print("  +%-4d %-6d %-24s %s" % (off, n, ctype, name))