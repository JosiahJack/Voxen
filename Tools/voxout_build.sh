#!/bin/bash
# voxout_build.sh - standalone build for the voxout tooling.
#
# Deliberately separate from ./build.sh: the engine's build only compiles a
# fixed SOURCES list, and voxout is an offline fitting tool, not part of the
# game.  Nothing here is linked into the voxen binary.
#
#   ./Tools/voxout_build.sh          build the C reference renderer
#   ./Tools/voxout_build.sh check    verify python deps only
#   ./Tools/voxout_build.sh all      build + smoke test
set -euo pipefail
cd "$(dirname "$0")"

MODE="${1:-all}"
BIN="voxout_render"
SRC="voxout_render.c"
TOOL="voxout.py"

# Prefer the same toolchain the engine uses, fall back to a plain cc.
if command -v zig >/dev/null 2>&1; then
    CC="zig cc"
elif command -v cc >/dev/null 2>&1; then
    CC="cc"
elif command -v clang >/dev/null 2>&1; then
    CC="clang"
else
    echo "voxout_build: no C compiler found (need zig cc, cc, or clang)" >&2
    exit 1
fi

check_deps() {
    python3 - <<'EOF'
import sys
missing = []
for m in ("numpy", "scipy", "soundfile"):
    try:
        __import__(m)
    except ImportError:
        missing.append(m)
if missing:
    sys.stderr.write("voxout_build: missing python modules: %s\n" % ", ".join(missing))
    sys.stderr.write("               pip3 install --user %s\n" % " ".join(missing))
    sys.exit(1)
import numpy, scipy
print("voxout_build: numpy %s, scipy %s ok" % (numpy.__version__, scipy.__version__))
EOF
}

build_c() {
    rm -f "./$BIN"
    # No engine headers: this is a self-contained reference kernel.
    $CC "$SRC" -O2 -std=c11 -o "./$BIN" -lm
    echo "voxout_build: built ./$BIN with ($CC)"
}

smoke() {
    check_deps
    python3 -c "import ast,sys; ast.parse(open('$TOOL').read())" \
        && echo "voxout_build: $TOOL parses ok"
    # The real trial file; a short budget keeps this fast.
    local wav="../Audio/hud/select.wav"
    if [[ -f "$wav" ]]; then
        echo "voxout_build: smoke test on $wav"
        python3 "./$TOOL" "$wav" --budget 4 --ladder >/dev/null \
            && echo "voxout_build: smoke test PASSED" \
            || echo "voxout_build: smoke test returned nonzero (below gate is ok)"
    fi
}

case "$MODE" in
    check) check_deps ;;
    c)     check_deps; build_c ;;
    all)   check_deps; build_c; smoke ;;
    *)     echo "usage: $0 [all|check|c]" >&2; exit 2 ;;
esac
echo "voxout_build: done"