#!/bin/sh
set -e
cd "$(dirname "$0")"
VERSION="0.99.94"
ARCHIVE="Citadel_v${VERSION}_x64.7z"
ARCHIVE_BASE="${ARCHIVE%.7z}"
OUTDIR="${1:-..}"
STAGE="$OUTDIR/$ARCHIVE_BASE"
mkdir -p "$OUTDIR"
rm -rf "$STAGE"
mkdir -p "$STAGE/Data" "$STAGE/Models" "$STAGE/Screenshots"
(cd Data && tar cf - --exclude='*.bin' .) | (cd "$STAGE/Data" && tar xf -)
cp -r Fonts Textures Audio "$STAGE/"
(cd Models && tar cf - --exclude='*.blend' .) | (cd "$STAGE/Models" && tar xf -)
cp voxen  "$STAGE/Citadel"
cp voxen.exe "$STAGE/Citadel.exe"
cp LICENSE "$STAGE/"
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
rm -f "$OUTDIR/$ARCHIVE"
(cd "$OUTDIR" && 7z a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on "$ARCHIVE" "$ARCHIVE_BASE" >/dev/null)
echo "wrote $OUTDIR/$ARCHIVE"
echo "staged folder: $STAGE"
