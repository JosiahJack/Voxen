#!/bin/bash
# gen_sfx_table.sh - batch consumer for voxout.
#
# Runs voxout over a set of .wav files, in parallel, and collects the results
# into:
#   <out>/sfx_presets.h   C table of accepted presets + the sounds[] -> preset map
#   <out>/sfx_report.tsv  one row per file: floor/dist/gap/complexity/time
#   Tools/voxoutputs/     A/B wavs for listening (original, 0.3s gap, fitted)
#
# voxout is run once per file in C format: its stdout is exactly one comment
# line of metrics plus one struct initializer, so the report and the preset
# table come from the same run.  (Re-running to get the other format made the
# two disagree, because the fits depend on the time budget.)
#
# Usage:
#   ./Tools/gen_sfx_table.sh                        # all wavs, default budget
#   ./Tools/gen_sfx_table.sh --budget 8 -j 12       # fast triage pass
#   ./Tools/gen_sfx_table.sh --budget 300 --only-failed
#   ./Tools/gen_sfx_table.sh --tier loose --no-ab
#   ./Tools/gen_sfx_table.sh --only "hud buttons weapons"
set -uo pipefail
cd "$(dirname "$0")/.."
ROOT="$PWD"

TOOL="$ROOT/Tools/voxout.py"
OUT="$ROOT/sfx_out"
ABDIR="$ROOT/Tools/voxoutputs"
BUDGET=45
TIER="strict"
# Leave one core free so the machine stays responsive, and pin each worker to
# a single BLAS thread so -j N really is N processes rather than N times the
# thread count.  Override with -j when you want the whole machine.
NCPU="$(nproc 2>/dev/null || echo 2)"
JOBS=$(( NCPU - 1 )); [[ "$JOBS" -lt 1 ]] && JOBS=1
MAKE_AB=1
NPS_SWEEP=0
RESUME=0
ONLY_FAILED=""
ONLY_DIRS=""
SCAN_ROOT="$ROOT/Audio"

usage() { sed -n '2,/^set /p' "$0" | sed 's/^# \{0,1\}//'; exit 0; }

while [[ $# -gt 0 ]]; do
    case "$1" in
        --budget)     BUDGET="$2"; shift 2 ;;
        --tier)       TIER="$2"; shift 2 ;;
        --jobs|-j)    JOBS="$2"; shift 2 ;;
        --out)        OUT="$2"; shift 2 ;;
        --ab-dir)     ABDIR="$2"; shift 2 ;;
        --no-ab)      MAKE_AB=0; shift ;;
        --resume)     RESUME=1; shift ;;
        --ab)         MAKE_AB=1; shift ;;
        --nps-sweep)  NPS_SWEEP=1; shift ;;
        --only-failed) ONLY_FAILED="$2"; shift 2 ;;
        --only)       ONLY_DIRS="$2"; shift 2 ;;
        --scan-root)  SCAN_ROOT="$2"; shift 2 ;;
        -h|--help)    usage ;;
        *) echo "unknown option: $1" >&2; usage ;;
    esac
done

RAW="$OUT/work/raw"
mkdir -p "$OUT" "$ABDIR" "$RAW"
# One run at a time per output dir: a previous aborted run left children
# appending to the shared index and double-counted every file.  flock is used
# rather than pgrep because pgrep matches this script's own command line.
LOCK="$OUT/.lock"
exec 9>"$LOCK"
if ! flock -n 9; then
    echo "gen_sfx_table: another run holds $LOCK; refusing to share $RAW" >&2
    exit 1
fi
REPORT="$OUT/sfx_report.tsv"
if [[ "${RESUME:-0}" != "1" ]]; then
    rm -f "$RAW"/*.c "$RAW"/*.err 2>/dev/null
fi
: > "$RAW/index.txt"

# --- build the file list -----------------------------------------------------
LIST="$OUT/work/files.txt"
: > "$LIST"
if [[ -n "$ONLY_FAILED" ]]; then
    if [[ ! -f "$REPORT" ]]; then
        echo "gen_sfx_table: no previous report at $REPORT" >&2; exit 1
    fi
    awk -F'\t' 'NR==1{for(i=1;i<=NF;i++)h[$i]=i; next}
                  $h["status"]!="pass"{print $h["path"]}' "$REPORT" >> "$LIST"
    echo "gen_sfx_table: retrying $(wc -l < "$LIST") previously-failed files"
else
    if [[ -n "$ONLY_DIRS" ]]; then
        for d in $ONLY_DIRS; do find "$SCAN_ROOT/$d" -name '*.wav' 2>/dev/null; done >> "$LIST"
    else
        find "$SCAN_ROOT" -name '*.wav' 2>/dev/null >> "$LIST"
    fi
fi
sort -u -o "$LIST" "$LIST"
TOTAL=$(wc -l < "$LIST")
[[ "$TOTAL" -eq 0 ]] && { echo "gen_sfx_table: no wav files found" >&2; exit 1; }
echo "gen_sfx_table: $TOTAL files, budget ${BUDGET}s, tier $TIER, -j$JOBS"
echo "gen_sfx_table: A/B wavs -> $ABDIR"

# $TOOL is passed in rather than inherited: export -f carries the function but
# not the script's own variables, so python3 would get an empty path.
# $3 (work dir) and $4 (ab dir) are separate: the per-file output must land in
# the work dir or the collector finds nothing.
run_one() {
    local tool="$1" wav="$2" work="$3" abdir="$4" budget="$5" tier="$6"
    local ab="$7" sweep="$8"
    local key; key=$(printf '%s' "$wav" | md5sum | cut -c1-16)
    # Several material folders reuse basenames (impact_soda.wav, impact_rifle1.wav,
    # ...), so key the A/B filename on the parent folder too or they collide and
    # silently overwrite each other.
    local dir; dir=$(basename "$(dirname "$wav")")
    local stem; stem="${dir}_$(basename "$wav" .wav)"
    local args=(--budget "$budget" --tier "$tier" --format c --name "$stem")
    # voxout prefixes the A/B name with ok_/REJECT_ itself once it knows
    [[ "$ab" == "1" ]] && args+=(--ab "$abdir/$stem.wav")
    [[ "$sweep" == "1" ]] && args+=(--nps-sweep)
    if [[ -s "$work/$key.c" ]]; then
        printf '%s\t%s\n' "$key" "$wav" >> "$work/index.txt"
        return 0                     # resume: already fitted
    fi
    OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 MKL_NUM_THREADS=1 \
        NUMEXPR_NUM_THREADS=1 VECLIB_MAXIMUM_THREADS=1 \
        python3 "$tool" "$wav" "${args[@]}" > "$work/$key.c" 2>"$work/$key.err"
    printf '%s\t%s\n' "$key" "$wav" >> "$work/index.txt"
}
export -f run_one

START=$(date +%s)
mapfile -t FILES < "$LIST"
printf '%s\n' "${FILES[@]}" | xargs -P "$JOBS" -I{} bash -c \
    'run_one "$@"' _ "$TOOL" {} "$RAW" "$ABDIR" "$BUDGET" "$TIER" "$MAKE_AB" "$NPS_SWEEP"
END=$(date +%s)
echo "gen_sfx_table: fitting done in $((END-START))s"

# --- report + preset table, both from the single pass per file ---------------
python3 - "$ROOT" "$OUT" "$RAW" "$REPORT" <<'PYEOF'
import glob, os, re, sys
root, out, raw, report = sys.argv[1:5]

FIELDS = ("path name tier status nps dur floor dist gap nHarm nBand odd tilt "
          "drive clipfrac seconds reason").split()
tok = re.compile(r"(\w+)=([^\s]+)")

rows, presets = [], []
for key, wav in (l.rstrip("\n").split("\t") for l in open(os.path.join(raw, "index.txt"))):
    f = os.path.join(raw, key + ".c")
    if not os.path.exists(f):
        continue
    text = open(f).read()
    comment = next((l for l in text.splitlines() if l.startswith("/* voxout")), "")
    init = next((l for l in text.splitlines() if l.startswith("{")), "")
    kv = dict(tok.findall(comment))
    r = {k: kv.get(k, "") for k in FIELDS}
    r["path"] = wav
    rows.append(r)
    if kv.get("status") == "pass" and init:
        rel = os.path.relpath(wav, os.path.join(root, "Audio")).replace(os.sep, "/")
        presets.append((rel[:-4] if rel.endswith(".wav") else rel, init))

with open(report, "w") as fh:
    fh.write("\t".join(FIELDS) + "\n")
    for r in sorted(rows, key=lambda x: x["path"]):
        fh.write("\t".join(str(r[k]) for k in FIELDS) + "\n")

# --- map sounds[] index -> preset ordinal -----------------------------------
src = open(os.path.join(root, "audio.c"), encoding="utf-8", errors="replace").read()
i = src.index("const char* sounds[SOUNDS_COUNT]")
j = src.index("};", i)
sounds = {int(n): p for p, n in re.findall(r'"([^"]*)"\s*/\*(\d+)\*/', src[i:j])}
lut = {rel: n for n, (rel, _init) in enumerate(sorted(presets))}

smap = [lut.get(sounds.get(n, ""), -1) for n in range(max(sounds) + 1 if sounds else 0)]
total = max(len(sounds), 670)
smap += [-1] * (total - len(smap))

out_path = os.path.join(out, "sfx_presets.h")
with open(out_path, "w") as fh:
    fh.write("/* GENERATED by Tools/gen_sfx_table.sh -- do not edit.\n"
             " * %d of %d sounds[] entries have a synth preset; the rest (-1)\n"
             " * fall back to play_wav() and keep their .wav.\n"
             " * Regenerate: ./Tools/gen_sfx_table.sh\n */\n"
             % (len(presets), total))
    fh.write("#pragma once\n#include \"common.h\"\n\n")
    fh.write("#define SFX_TABLE_COUNT %d\n" % len(presets))
    fh.write("#define SFX_SOUND_COUNT %d\n\n" % total)
    fh.write("static const SfxDef sfxPresets[SFX_TABLE_COUNT] = {\n")
    for rel, init in sorted(presets):
        fh.write("    %s /* %s */,\n" % (init, rel))
    fh.write("};\n\n")
    fh.write("/* sounds[] index -> sfxPresets[] index, -1 = keep the .wav */\n")
    fh.write("static const i16 sfxForSound[SFX_SOUND_COUNT] = {\n")
    for k in range(0, len(smap), 24):
        fh.write("    " + ",".join(str(v) for v in smap[k:k + 24]) + ",\n")
    fh.write("};\n")

print("gen_sfx_table: wrote %s (%d presets, %d/%d sounds[] mapped)" %
      (out_path, len(presets), sum(1 for v in smap if v >= 0), len(smap)))

# --- summary ----------------------------------------------------------------
import statistics
d = sorted(float(r["dist"]) for r in rows if r["dist"])
ok = [r for r in rows if r["status"] == "pass"]
bad = [r for r in rows if r["status"] != "pass"]
tot = sum(float(r["seconds"]) for r in rows if r["seconds"])
print("\n=== %d files, %d accepted, %d rejected, %.0fs cpu ===" %
      (len(rows), len(ok), len(bad), tot))
if d:
    n = len(d)
    print("dist (dB):  min %.2f  p25 %.2f  median %.2f  p75 %.2f  p90 %.2f  max %.2f"
          % (d[0], d[n//4], statistics.median(d), d[3*n//4], d[int(.9*n)], d[-1]))
import collections
for fld in ("nHarm", "nBand"):
    c = collections.Counter(int(r[fld] or 0) for r in ok)
    if c:
        print("accepted %-6s %s" % (fld, dict(sorted(c.items()))))
if bad:
    bad.sort(key=lambda r: -float(r["dist"] or 0))
    print("\nworst rejects:")
    for r in bad[:10]:
        print("   %-44s dist=%s" % (os.path.basename(r["path"]), r["dist"]))
PYEOF
