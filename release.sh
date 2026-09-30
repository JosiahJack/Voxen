#!/bin/sh
# release.sh - package the already-built release binaries into a 7z archive.
# Bump VERSION manually for each release.
# Stages the payload in a folder in the parent directory (../) named the same as
# the final .7z, copies over the release folders/files (excluding .blend sources
# and save *.bin files), forces Config.ini to 1366x768 + the code defaults, then
# zips that entire folder up as a 7z with max LZMA compression.
# The finished archive is placed in the parent directory of the repo (../) by
# default, or $1 to override.
set -e
cd "$(dirname "$0")"

VERSION="0.99.94"
ARCHIVE="Citadel_v${VERSION}_x64.7z"
ARCHIVE_BASE="${ARCHIVE%.7z}"
OUTDIR="${1:-..}"

command -v 7z >/dev/null 2>&1 || { echo "7z not found: sudo apt install p7zip-full"; exit 1; }

# Sanity check the binaries we are about to ship.
case "$(head -c 2 voxen 2>/dev/null)" in
    "\x7fE") : ;; # ELF Linux binary
    *) echo "release.sh: voxen is not a Linux ELF binary" >&2; exit 1 ;;
esac
case "$(head -c 2 voxen.exe 2>/dev/null)" in
    "MZ") : ;; # PE Windows binary
    *) echo "release.sh: voxen.exe is not a Windows PE binary" >&2; exit 1 ;;
esac

TMPD="$(mktemp -d)"
trap 'rm -rf "$TMPD"' EXIT INT TERM
STAGE="$TMPD/stage"
mkdir -p "$STAGE" "$STAGE/Data" "$STAGE/Models"
# Data: copy everything except save files (*.bin)
(cd Data && tar cf - --exclude='*.bin' .) | (cd "$STAGE/Data" && tar xf -)
cp -r Fonts Textures Audio "$STAGE/"
# Models: copy everything except .blend source files
(cd Models && tar cf - --exclude='*.blend' .) | (cd "$STAGE/Models" && tar xf -)
# Ship the binaries under their release names.
cp voxen  "$STAGE/Citadel"
cp voxen.exe "$STAGE/Citadel.exe"
cp LICENSE "$STAGE/"
# Config.ini: force 1366x768 and the code defaults from winput.c/voxen.c
sed -i \
    -e 's/^ResolutionWidth = .*/ResolutionWidth = 1366/' \
    -e 's/^ResolutionHeight = .*/ResolutionHeight = 768/' \
    -e 's/^AA = .*/AA = 0/' \
    -e 's/^Shadows = .*/Shadows = 0/' \
    -e 's/^SSR = .*/SSR = 0/' \
    -e 's/^ModelDetail = .*/ModelDetail = 0/' \
    -e 's/^GI = .*/GI = 0/' \
    -e 's/^Reverb = .*/Reverb = 0/' \
    -e 's/^VolumeMusic = .*/VolumeMusic = 25/' \
    -e 's/^MouseSensitivity = .*/MouseSensitivity = 10/' \
    "$STAGE/Data/Config.ini"

# Stage the payload folder in the parent dir, named like the archive (without .7z)
mkdir -p "$OUTDIR"
rm -rf "$OUTDIR/$ARCHIVE_BASE"
cp -r "$STAGE" "$OUTDIR/$ARCHIVE_BASE"
# Zip the staged folder up as 7z with max LZMA compression
(cd "$OUTDIR" && 7z a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on "$TMPD/$ARCHIVE" "$ARCHIVE_BASE" >/dev/null)
mv "$TMPD/$ARCHIVE" "$OUTDIR/$ARCHIVE"
echo "wrote $OUTDIR/$ARCHIVE"
echo "staged folder: $OUTDIR/$ARCHIVE_BASE"