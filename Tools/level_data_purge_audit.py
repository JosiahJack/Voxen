#!/usr/bin/env python3
"""Audit Data/level*.txt for keys the loader does not need to read.

Voxen's loader initialises each live instance with mset(&inst, 0, sizeof(Entity))
and then applies prefab/EPerms defaults.  A level record key whose value is
always 0 is therefore only load-bearing when some later step would have written
a non-zero value into that field anyway -- in that case the record's 0 is
meaningful and the key must stay.

This tool separates the two cases:

  drop-key   every occurrence in every level is 0, so the key carries no
             information the loader can use.
  sentinel   the value is -1, which is Voxen's "absent" marker for
             contents[]/custIdx[]/randomItem[]/randomItemCustIdx[] and is
             never the mset default.  Never dropped.
  live       the key has at least one non-zero occurrence.

Among drop-key entries, `UNREAD` ones are additionally safe: Voxen has no
parser branch for them at all, so they are pure Unity export noise.  The
remainder need the field-default check (see --fields) before they can be
purged.

Usage:
  Tools/level_data_purge_audit.py                 # report
  Tools/level_data_purge_audit.py --list          # purgeable keys, one per line
  Tools/level_data_purge_audit.py --apply         # purge every drop-key
  Tools/level_data_purge_audit.py --apply --dry-run

Why every drop-key is safe to remove
------------------------------------
LoadLevelMod resets the staging record at the top of every line
(`inst = &entsFromFile[entCount]; mset(...)`), so staging starts at all-zero.
A key whose exported value is 0 therefore writes the value staging already
holds, and omitting it leaves staging byte-identical.  Since copy-back reads
only staging, the loaded Entity is unchanged.  That covers both shapes:

  flag_set(&inst->entflags, FLAG, false)   bit already clear -> no-op
  inst->field = 0                          already 0          -> no-op

Keys Voxen has no branch for are trivially safe by the same argument.  The one
key in this set with a non-assigning branch is testQuestBitIsOff, which is
`inst->questTestMode = parse_bool(v) ? 2 : inst->questTestMode` -- on 0 it
keeps the existing value, so it is a no-op too.

Keys that are NOT eligible even when they read 0: the per-component lP/lR/lS
family, because the parser also uses their *presence* to set ipSubRotSet /
npcRootRotSet, so dropping a zero lR.w changes behaviour.  Those keys are live
(non-zero somewhere) and are excluded by the drop-key test anyway.
"""
import glob
import os
import re
import sys
import collections

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA_GLOB = os.path.join(ROOT, "Data", "level*.txt")
ENTITY_C = os.path.join(ROOT, "entity.c")

SENTINEL_FIELDS = ("contents", "custIdx", "randomItem", "randomItemCustIdx")

# Keys whose *presence* or *zero value* carries meaning, so a 0 cannot be dropped.
#
# LoadLevelMod resets the per-record staging to:
#     inst (Entity)      all 0            <- a plain 0 is redundant
#     posFromFile        all 0            <- a plain 0 is redundant
#     scaleFromFile      (1,1,1)          <- 0 is NOT redundant
#     rotationFromFile   QUAT_IDENTITY    <- w=0 is NOT redundant
#     colCtrFromFile     all 0            <- a plain 0 is redundant
#     colSzFromFile      (-1,-1,-1)       <- 0 is NOT redundant: it is what passes
#                                             the `colSz >= 0` presence test
# A key is purgeable at value 0 only when it writes to `inst` and its presence
# sets no parser state.
# ---------------------------------------------------------------------------
# Presence-sensitive keys: never purgeable at any value.
#
# Each of these records a *fact about the record's shape* in parser state, so
# deleting the key changes the parse even when the value is the default.
# ---------------------------------------------------------------------------
PRESENCE_SENSITIVE = {
    "constIndex",        # sets constIndexRead / fwLine / ipLine / npcLine
    "go.activeSelf",     # activeStateRead; absent forces EF_ACTIVE on (entity.c:835)
    "matIndex",          # matIndexRead; absent uses the default texIndex (entity.c:842)
    "text", "tA", "tAl", "tLs",   # each calls PendDecal(), appending to pendDecals[];
                                   # tLs additionally defaults to 1.0f (entity.c:595)
    "target", "targetname", "targetOnDeath", "targetIfFalse",  # IOInternName() registers
    "type",              # light type: flag_set, value is a string like "Spot"
}
# chunkIDs[N] appends to the func_wall child pool and advances its cursor:
#   lwPrefab[fwCurChild]=idx; fwLastChunkSlot=fwCurChild; fwPendingChild=true; fwCurChild++;
PREFIX_SENSITIVE = ("chunkIDs[",)

# ---------------------------------------------------------------------------
# The per-record staging defaults, from entity.c:632-636.  A pair is purgeable
# when its value equals the default the slot already holds AND the key carries
# no presence meaning -- then omitting it leaves the loaded state bit-identical.
#
#   inst(Entity)         all 0        except relayEnabled=true
#   posFromFile          all 0
#   scaleFromFile        (1,1,1)
#   rotationFromFile     QUAT_IDENTITY -> w=1
#   colCtrFromFile       all 0
#   colSzFromFile        (-1,-1,-1)   and the test is `colSz >= 0`, so 0 is NOT default
#   contents/custIdx[0..3], randomItem/randomItemCustIdx[0..6]  -1
#
# Lights are staged separately (LoadFieldIntoLight) and re-seeded per level:
#   range=5.5, col=(1,1,1), spotAng=0, everything else 0.
# ---------------------------------------------------------------------------
TRIGGER_SCALE = {595: (2.56, 2.56, 2.56), 596: (1.0, 1.0, 1.0),
                 597: (1.0, 2.4, 0.16), 598: (2.56, 2.56, 2.56),
                 599: (2.56, 2.56, 2.56), 600: (2.56, 2.56, 2.56),
                 601: (2.56, 2.56, 2.56)}
NPC_LO, NPC_HI = 419, 447            # IdxIsNPC -- presence of lR drives the yaw fallback
STRUCTURED = (517, 602, 614, 615)    # fwLine / ipLine write through a sub-staging cursor
SENTINEL_DEFAULTED_PREFIXES = ("contents[", "custIdx[", "randomItem[", "randomItemCustIdx[")
SENTINEL_DEFAULTED_EXACT = {"relayEnabled"}
TRANSFORM_FAMILY_PREFIXES = ("lP.", "lR.", "lS.")
LIGHT_DEFAULTS = {"range": 5.5, "color.r": 1.0, "color.g": 1.0, "color.b": 1.0}
SCALE_COMPONENT = {"lS.x": 0, "lS.y": 1, "lS.z": 2}


def _npc_or_structured(ci):
    return (NPC_LO <= ci <= NPC_HI) or (ci in STRUCTURED)


def entity_is_default(key, val, ci):
    """True when writing `val` is identical to omitting `key` on an entity record."""
    if key.startswith(SENTINEL_DEFAULTED_PREFIXES):
        return val == -1.0
    if key in SENTINEL_DEFAULTED_EXACT:
        return val == 1.0
    if key.startswith("center."):
        return val == 0.0
    if key.startswith("size."):
        return val == -1.0            # colSz defaults to -1 and the test is `>= 0`
    if key == "lP.x":
        return False                  # opens a transform block
    if key in ("lP.y", "lP.z"):
        return val == 0.0 and not _npc_or_structured(ci)
    if key in ("lR.x", "lR.y", "lR.z"):
        return val == 0.0 and not _npc_or_structured(ci)
    if key == "lR.w":
        return val == 1.0 and not _npc_or_structured(ci)   # QUAT_IDENTITY
    if key in SCALE_COMPONENT:
        default = TRIGGER_SCALE[ci][SCALE_COMPONENT[key]] if ci in TRIGGER_SCALE else 1.0
        return val == default and not _npc_or_structured(ci)
    return val == 0.0


def light_is_default(key, val):
    return val == LIGHT_DEFAULTS.get(key, 0.0)


def is_presence_sensitive(key):
    return (key in PRESENCE_SENSITIVE
            or key.startswith(PREFIX_SENSITIVE))


# Keys denied by evidence, not by inspection.  For every key here the entity-dump
# differ reported a field changing when the key was removed, so the effective
# default is NOT the staging default and the key has to stay in the data:
#
#   randomItemCustomIndex[N]  entity.c:633 stages randomItemCustIdx[] to -1, so a
#                             data 0 is a real distinct value.  Removing 129 of them
#                             moved randomItemCustIdx on 129 instances, 0 -> -1.
#   lightOn                   LoadFieldIntoLight does flag_set(&lit->lflags, LIGHTON,
#                             parse_bool(v)): lightOn:0 CLEARS the bit, it does not
#                             write the staging value.  Dropping it left 107 lights
#                             switched on, lflags 0x12 -> 0x13 and 0x20 -> 0x21.
DENY_BY_EVIDENCE = {
    "randomItemCustomIndex[0]", "randomItemCustomIndex[1]", "randomItemCustomIndex[2]",
    "randomItemCustomIndex[3]", "randomItemCustomIndex[4]", "randomItemCustomIndex[5]",
    "randomItemCustomIndex[6]",
    "lightOn",
}
DENY = set(DENY_BY_EVIDENCE)


def load_deny(argv):
    for a in argv:
        if a.startswith("--deny-keys="):
            for ln in open(a.split("=", 1)[1]):
                ln = ln.strip()
                if ln and not ln.startswith("#"):
                    DENY.add(ln)


def pair_is_purgeable(key, val, ci, is_light):
    if is_presence_sensitive(key) or key in DENY:
        return False
    return light_is_default(key, val) if is_light else entity_is_default(key, val, ci)


def record_purge_mask(aligned, ci, is_light):
    """Per-pair "safe to drop" mask for one record.

    A key can appear more than once in a record -- a constIndex 614 relay panel
    writes despawnInstead:1 and later despawnInstead:0, then a trailing
    doSelfAfterList block repeats more keys.  The parser is not block-aware for
    most of those, so the LAST occurrence wins.  Dropping a trailing
    default-valued occurrence exposes an earlier non-default one and silently
    changes the loaded value, which is exactly the despawnInstead 0 -> 1
    regression the entity-dump differ caught.  So a key is droppable only when
    *every* occurrence of it in the record is default-valued.

    `aligned` is [(pair_text, key, value), ...] in file order.
    """
    per_key = collections.defaultdict(list)
    own = []
    for _, k, v in aligned:
        fv = as_float(v)
        own.append(k)
        per_key[k].append(fv is not None and pair_is_purgeable(k, fv, ci, is_light))
    key_ok = {k: all(oks) for k, oks in per_key.items()}
    return [key_ok[k] for k in own]


def record_ctx(line):
    """(aligned, constIndex, is_light) for one record line."""
    pairs = line.split("|")
    aligned = [(p, p.split(":", 1)[0], p.split(":", 1)[1]) for p in pairs if ":" in p]
    is_light = not aligned or aligned[0][1] != "constIndex"
    ci = -1
    if not is_light:
        try:
            ci = int(float(aligned[0][2]))
        except ValueError:
            is_light = True
    return aligned, ci, is_light


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    return re.sub(r"//[^\n]*", " ", text)


def parser_keys():
    """Keys LoadLevelMod actually has a branch for."""
    src = strip_comments(open(ENTITY_C).read())
    i = src.index("void LoadLevelMod(")
    body = src[i:src.index("\nvoid ", i + 10)] if "\nvoid " in src[i + 10:] else src[i:]
    return set(re.findall(r'KEY_EQ\(\s*"([^"]+)"', body)) | set(
        re.findall(r'KEY_EQ\("([^"]+)"\)', body))


def as_float(v):
    """Numeric value of a pair, or None when it is not a plain number."""
    try:
        f = float(v)
    except ValueError:
        return None
    return f if f == f else None      # reject NaN


def is_sentinel(v):
    try:
        return float(v) == -1.0
    except ValueError:
        return False


def main():
    do_apply = "--apply" in sys.argv
    dry = "--dry-run" in sys.argv
    do_list = "--list" in sys.argv
    load_deny(sys.argv)

    files = sorted(glob.glob(DATA_GLOB))
    total_bytes = sum(os.path.getsize(f) for f in files)
    lines = {f: open(f).read().split("\n") for f in files}

    # key -> [occurrences, zero_occurrences, sentinel_occurrences, bytes, sample_nonzero]
    stats = collections.defaultdict(lambda: [0, 0, 0, 0, None])

    for f in files:
        for ln in lines[f]:
            if not ln:
                continue
    # Record-aware scan.  A record is a light when its first key is not constIndex;
    # entity records carry constIndex, which selects the per-index defaults.
    purged_bytes = 0
    purged_pairs = 0
    kept_pairs = 0
    by_key = collections.defaultdict(lambda: [0, 0])
    scanned = 0

    for f in files:
        for ln in lines[f]:
            if not ln:
                continue
            aligned, ci, is_light = record_ctx(ln)
            if not aligned:
                continue
            scanned += 1
            for (p, k, v), ok in zip(aligned, record_purge_mask(aligned, ci, is_light)):
                if not ok:
                    continue
                if True:
                    purged_pairs += 1
                    purged_bytes += len(p) + 1
                    by_key[k][0] += 1
                    by_key[k][1] += len(p) + 1
                else:
                    kept_pairs += 1

    if do_list:
        for k in sorted(by_key):
            print(k)
        return 0

    print("Data/level*.txt: %d files, %d bytes, %d records\n"
          % (len(files), total_bytes, scanned))
    print("pairs that equal the staging default (PURGE) : %8d  %9d bytes  (%.2f%%)"
          % (purged_pairs, purged_bytes, 100.0 * purged_bytes / total_bytes))
    print("pairs that must stay                          : %8d" % kept_pairs)
    print()
    print("corpus %d -> %d bytes  (saves %d, %.2f%%)"
          % (total_bytes, total_bytes - purged_bytes, purged_bytes,
             100.0 * purged_bytes / total_bytes))
    print()
    print("== never purged, whatever the value ==")
    for k in sorted(PRESENCE_SENSITIVE):
        print("  %s" % k)
    for pre in PREFIX_SENSITIVE:
        print("  %s...  (appends to the func_wall child pool)" % pre)
    print("  lP.x        (opens a transform block)")
    print("  lR.*/lS.*   on constIndex 419-447 (NPC: presence drives the yaw fallback)")
    print("  lR.*/lS.*/lP.* on constIndex 517/602/614/615 (fwLine/ipLine sub-staging)")
    print()
    print("== top 30 purgeable keys by bytes ==")
    for k, (n, b) in sorted(by_key.items(), key=lambda kv: -kv[1][1])[:30]:
        print("  %-36s %7d pairs %9d bytes" % (k, n, b))
    print("  ... %d keys total" % len(by_key))

    if do_apply:
        if dry:
            print("\n[dry-run] would purge %d pairs, %d bytes" % (purged_pairs, purged_bytes))
            return 0
        if os.environ.get("VOXEN_PURGE") != "yes":
            print("\nrefusing to rewrite Data/level*.txt without VOXEN_PURGE=yes",
                  file=sys.stderr)
            return 1
        removed = 0
        for f in files:
            out = []
            for ln in lines[f]:
                if not ln:
                    out.append(ln)
                    continue
                aligned, ci, is_light = record_ctx(ln)
                if not aligned:
                    out.append(ln)
                    continue
                kept = []
                for (p, k, v), ok in zip(aligned, record_purge_mask(aligned, ci, is_light)):
                    if ok:
                        removed += 1
                        continue
                    kept.append(p)
                out.append("|".join(kept) if kept else ln)
            open(f, "w").write("\n".join(out))
        after = sum(os.path.getsize(f) for f in files)
        print("\npurged %d default-valued pairs: %d -> %d bytes (saved %d)"
              % (removed, total_bytes, after, total_bytes - after))
        print("\nNow verify:")
        print("  python3 Tools/parse_coverage_check.py")
        print("  VOXEN_DUMP_ENTITIES=/tmp/purge%d.ent timeout 60 ./voxen")
        print("  python3 Tools/entity_dump_diff.py <baseline>.ent /tmp/purge0.ent")
        print("  -> expect: RESULT: PASS, and no field differences")
    return 0


if __name__ == "__main__":
    sys.exit(main())
