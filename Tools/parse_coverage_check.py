#!/usr/bin/env python3
"""Parse-coverage check for the Voxen level loader.

Two independent holes are checked, both of which silently discard level data:

  (1) parse -> copy.  LoadLevelMod parses each record into a staging `Entity`
      (entsFromFile[]) and then propagates it to the live instance with a
      hand-written list of `par->X = src->X`.  Any field the parser writes that
      the copy-back omits is parsed and then thrown away.  This is what dropped
      gridCells, wireCurL/R, logIndex, teleportID and 18 others.

  (2) data -> parse.  Any key present in Data/level*.txt that Voxen reads but
      the parser has no branch for.  This is what dropped teleportID and
      targetDestinationID, which Unity's TeleportTouch.Save() emits.

Usage:  Tools/parse_coverage_check.py [--verbose]
Exit 0 when clean, 1 when a hole is found.
"""
import glob
import os
import re
import sys
import collections

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from entity_layout import entity_fields

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ENTITY_C = os.path.join(ROOT, "entity.c")
DATA_GLOB = os.path.join(ROOT, "Data", "level*.txt")

# LoadLevelMod's line range, discovered by brace matching on the function.
FUNC = "LoadLevelMod"


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return re.sub(r"//[^\n]*", " ", text)


def func_body(text, name):
    start = text.index("void " + name + "(")
    brace = text.index("{", start)
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[brace : i + 1], text[:start].count("\n") + 1
    raise SystemExit("unbalanced braces in " + name)


def main():
    verbose = "--verbose" in sys.argv
    src = open(ENTITY_C, encoding="utf-8", errors="replace").read()
    body, lineno = func_body(src, FUNC)
    body = strip_comments(body)
    off = lineno

    # The parse loop writes the staging record through `inst`, and two
    # post-loop fixups go through entsFromFile[entCount] directly.
    staged = set(re.findall(r"\binst->(\w+)", body))
    staged |= set(re.findall(r"\bentsFromFile\[entCount\]\.(\w+)", body))

    # The propagation list reads src-> and writes par->.
    carried = set(re.findall(r"\bpar->(\w+)", body))

    # `index` is passed to AddInstance rather than copied; `entflags` rides the
    # bitwise OR with its own EF_ACTIVE restore.  Both are handled, not dropped.
    handled_elsewhere = {"index", "entflags"}

    # (1) parse -> copy
    dropped = sorted(
        (staged - carried - handled_elsewhere),
        key=lambda f: f.lower(),
    )

    # (2) data -> parse.  Keys the parser reads live in the KEY_EQ chain; the
    # light record has its own map in LoadFieldIntoLight.
    keyeq = set(re.findall(r'KEY_EQ\("([^"]+)"\)', src))
    keyeq |= {
        "lP.x", "lP.y", "lP.z", "lR.x", "lR.y", "lR.z", "lR.w",
        "lS.x", "lS.y", "lS.z",
    }
    # Keys whose suffix after the dot names a member of some *other* Unity
    # object, not of Entity.  `deathBurst.activeSelf` is a GameObject bool --
    # Entity's deathBurst is the u16 child-instance index.  `clip.name` is an
    # AnimationClip asset name -- Entity's clip is the u8 door-ajar animation
    # index.  Neither maps to the Entity field the prefix happens to name.
    SUBOBJECT_MEMBERS = {
        "activeSelf", "activeInHierarchy", "childCount", "name", "transform",
        "gameObject", "localPosition", "localScale", "tag", "layer",
    }
    light_map = set(re.findall(r'\{"([\w.]+)"', func_body(src, "LoadFieldIntoLight")[0]))
    light_map |= {
        "intensity", "type", "lightOn", "lerpOn", "targetname",
        "intervalSteps[", "intervalStepisLerping[", "currentStep", "lerpValue",
    }

    prefixes = ("grid[", "currentPositions", "chunkIDs[", "intervalStep")

    def is_handled(k):
        return k in keyeq or any(k.startswith(p) for p in prefixes)

    counts = collections.Counter()
    values = collections.defaultdict(set)
    for path in glob.glob(DATA_GLOB):
        with open(path, encoding="utf-8", errors="replace") as fh:
            for line in fh:
                for kv in line.rstrip("\n").split("|"):
                    if ":" not in kv:
                        continue
                    k, _, v = kv.partition(":")
                    counts[k] += 1
                    if len(values[k]) < 16:
                        values[k].add(v)

    # A key Voxen consumes if some non-entity.c source mentions the identifier.
    others = "".join(
        open(p, encoding="utf-8", errors="replace").read()
        for p in glob.glob(os.path.join(ROOT, "*.c"))
        if os.path.basename(p) != "entity.c"
    )
    others += open(os.path.join(ROOT, "common.h"), encoding="utf-8", errors="replace").read()
    assert others, "failed to read engine sources"

    unparsed_live = []
    efields = {name for name, _, _, _ in entity_fields()[0]}
    for k, n in counts.most_common():
        if is_handled(k) or k in light_map:
            continue
        base = re.split(r"[.\[]", k)[0]
        suffix = k.rsplit(".", 1)[-1]
        if "." in k and suffix in SUBOBJECT_MEMBERS:
            continue  # same name, different Unity object -- not an Entity field
        # Voxen only cares if the identifier names an Entity field it actually
        # reads.  Membership in the compiler-extracted field list is the right
        # test; a bare text search would flag every generic token in the tree
        # (time, force, distance, direction -- mostly SoA arrays and locals) and
        # would miss nothing it should catch.
        if base not in efields:
            continue  # Unity GameObject/AnimationClip plumbing Voxen does not model
        if base in keyeq:
            continue  # parser has a branch for this key under its bare name
        if len(values[k]) == 1 and next(iter(values[k])) in ("0", "0.0", ".0"):
            kind = "always-zero (dropping is a no-op)"
        else:
            kind = "LIVE DATA -- add a parse branch"
        unparsed_live.append((n, k, kind))

    ok = True
    print("=" * 78)
    print("parse -> copy:  staging fields the copy-back does not carry")
    print("=" * 78)
    if dropped:
        ok = False
        for f in dropped:
            print("  MISSING  par->%s  (parser writes inst->%s)" % (f, f))
    else:
        print("  clean -- every inst-> field reaches the live instance")

    print()
    print("=" * 78)
    print("data -> parse:  keys in Data/level*.txt that Voxen reads but never parses")
    print("=" * 78)
    live = [x for x in unparsed_live if x[2].startswith("LIVE")]
    zero = [x for x in unparsed_live if not x[2].startswith("LIVE")]
    if live:
        ok = False
        print("  %d key(s) carrying data the engine expects but never receives:" % len(live))
        for n, k, kind in live:
            print("    %-36s %6d occurrences  %s" % (k, n, kind))
            if verbose:
                print("        values: %s" % sorted(values[k])[:12])
    else:
        print("  clean -- no key Voxen reads is left unparsed")
    if zero:
        print("  (%d further key(s) name a field Voxen reads but are 0 in every record," % len(zero))
        print("   so the omission is a no-op today: %s)" % ", ".join(k for _, k, _ in zero[:12]))
        if len(zero) > 12:
            print("    ... and %d more" % (len(zero) - 12))

    print()
    print("RESULT:", "PASS" if ok else "FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
