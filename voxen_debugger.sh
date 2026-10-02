#!/bin/bash
# voxen_debugger.sh - build a debug voxen and launch it under the RAD debugger.
#
# Usage: ./voxen_debugger.sh [raddbg options...]   e.g.  ./voxen_debugger.sh --project:voxen.raddbg_project
#
# `build.sh debug` on its own builds *and runs* the game, and raddbg launches its own instance of the binary, so the
# build here passes `ci` to stay build-only.  --auto_run makes the debugger start the target running instead of leaving
# it halted at the entry point.
set -euo pipefail
cd "$(dirname "$0")"
RADDBG="${RADDBG:-$HOME/Github/raddebugger/build/raddbg}"
if [ ! -x "$RADDBG" ]; then
    echo "voxen_debugger.sh: raddbg not found at $RADDBG" >&2
    echo "  build it with:  (cd ~/Github/raddebugger && ./build.sh raddbg)" >&2
    exit 1
fi
./build.sh debug ci
# raddbg stops parsing -/-- options once it sees the first non-option argument, so any pass-through has to precede the
# target; options after it would be handed to voxen as argv instead.
exec "$RADDBG" "$@" --auto_run voxen
