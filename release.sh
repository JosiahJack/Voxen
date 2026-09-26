#!/bin/sh
# release.sh - build Linux+Windows release binaries and package the release archive.
# Bump VERSION manually for each release.
# Stages the payload in a tmp dir; the finished archive is placed in the
# parent directory of the repo (../) by default, or $1 to override.
set -e
cd "$(dirname "$0")"

VERSION="0.99.94"
ARCHIVE="Citadel_v${VERSION}_x64.7z"
OUTDIR="${1:-..}"

command -v 7z >/dev/null 2>&1 || { echo "7z not found: sudo apt install p7zip-full"; exit 1; }

./build.sh ci
./build.sh windows ci
strip voxen voxen.exe

TMPD="$(mktemp -d)"
trap 'rm -rf "$TMPD"' EXIT INT TERM
STAGE="$TMPD/stage"
mkdir -p "$STAGE"
cp -r Data Fonts Textures Audio "$STAGE/"
mkdir -p "$STAGE/Models"
(cd Models && tar cf - --exclude='*.blend' .) | (cd "$STAGE/Models" && tar xf -)
cp voxen voxen.exe LICENSE "$STAGE/"
(cd "$STAGE" && 7z a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on "$TMPD/$ARCHIVE" Data Fonts Textures Models Audio voxen voxen.exe LICENSE >/dev/null)
mkdir -p "$OUTDIR"
mv "$TMPD/$ARCHIVE" "$OUTDIR/$ARCHIVE"
echo "wrote $OUTDIR/$ARCHIVE"
