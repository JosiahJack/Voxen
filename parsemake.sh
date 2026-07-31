#!/bin/bash
set -euo pipefail; PLATFORM="linux"; IS_CI=false
while [[ $# -gt 0 ]]; do
    case "$1" in
        win|windows) PLATFORM="windows" ;;
        ci) IS_CI=true ;;
        *) echo "Unknown: $1" >&2; exit 1 ;;
    esac
    shift
done
$IS_CI || clear
TEMP_DIR=temp_build
mkdir -p "$TEMP_DIR" # Ensuring directory exists before compilation
rm -f "$TEMP_DIR"/*.o
export TMPDIR=/dev/shm
now_ms() { date +%s%3N; }
build_start=$(now_ms)
echo "Compiling vparser (CI build)..."
LINUX_CC="zig cc"
WINDOWS_CC="zig cc -target x86_64-windows-gnu -Wframe-larger-than=65536"
COMMON_CFLAGS="-ferror-limit=500 -fno-stack-protector -fno-unwind-tables -Wno-format-nonliteral -fvisibility=hidden -pipe -fno-ident -fdata-sections -Wno-int-to-void-pointer-cast \
               -Wshadow -ffunction-sections -ffast-math -std=c11 -Wall -Wextra -Wno-implicit-fallthrough -Wno-switch -fdeclspec -fomit-frame-pointer -g0 -fstrict-aliasing \
               -Wno-overlength-strings -fno-math-errno -fno-sanitize=all -fno-trapping-math -fmerge-all-constants -m64 -Os -march=haswell -mf16c -mavx -Wbool-conversion -Wno-empty-body -nostdinc"
COMMON_LFLAGS="-Wl,-z,relro,-z,now,--gc-sections,--as-needed,--build-id=none"
if [ "$PLATFORM" = "windows" ]; then
    CC=$WINDOWS_CC
    LINKER=$CC
    CFLAGS="-D_WIN32 $COMMON_CFLAGS -mno-stack-arg-probe -Wl,-Bstatic -lmingw32 -lmingwex"
    LDFLAGS="$COMMON_LFLAGS -L. -lgdi32 -lole32 -static-libgcc"
    BINARY_NAME="vparser.exe"
else
    CC=$LINUX_CC
    LINKER=$CC
    CFLAGS="$COMMON_CFLAGS -fno-plt -fno-semantic-interposition -fno-builtin"
    LDFLAGS="$COMMON_LFLAGS -target x86_64-linux-gnu.2.7 -ldl"
    BINARY_NAME="vparser"
fi

export CC=$CC
export CFLAGS=$CFLAGS
SOURCES="vparser.c lib.c"
export TEMP_DIR=temp_build
printf "%s\n" $SOURCES | xargs -P12 -I{} sh -c "$CC -c {} $CFLAGS -o $TEMP_DIR/\$(basename {}).o"
$LINKER "$TEMP_DIR"/*.o $LDFLAGS -o $BINARY_NAME
build_end=$(now_ms)
total_build_time=$((build_end - build_start))
echo "Built offline vparser in ${total_build_time} ms"
if ! $IS_CI; then
    case "$PLATFORM" in
        windows)  wine ./vparser.exe -all ;;
        *)        ./vparser -all ;;   # linux
    esac
    rm -f "$TEMP_DIR"/*.o ./vparser.upx ./vparser.pdb # Cleanup
fi
