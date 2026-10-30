#!/bin/bash
set -euo pipefail
PLATFORM=linux; IS_CI=false; BUILD_MODE=release
for a; do case $a in win|windows) PLATFORM=windows;; ci) IS_CI=true;; debug) BUILD_MODE=debug;; *) echo "Unknown: $a" >&2; exit 1;; esac; done
$IS_CI || clear
mkdir -p temp_build
rm -f Shaders/*.h temp_build/*.o
export TMPDIR=/dev/shm
now_ms() { date +%s%3N; }
shader_start=$(now_ms)
if ! $IS_CI; then
    TODAY=$(date +%F)
    [[ -f builds.csv ]] || echo date,builds_today > builds.csv
    TODAY_COUNT=$(awk -F, -v d="$TODAY" '
        {sub(/\r$/,"")}
        $1==d {gsub(/[^0-9]/,"",$2); n=$2+0; next}
        {print > "builds.csv.tmp"}
        END {n++; print d","n > "builds.csv.tmp"; print n}
    ' builds.csv)
    mv builds.csv{.tmp,}
    echo "Compiling voxen, total iterations today $TODAY_COUNT ($TODAY)..."
else
    echo "Compiling voxen (CI build)..."
fi
gh() {
    { [[ $PLATFORM == android ]] && printf '#version 310 es\nprecision mediump float;\nprecision mediump sampler2D;\n\n' || printf '#version 430 core\n\n'; cat "$1"; } | sed 's|//.*||g' | awk '
    /^\s*#/ {gsub(/^\s+|\s+$/,"");printf"%s\\n",$0;next}
    {gsub(/[ \t\r\n]+/," ");gsub(/^[ \t]+|[ \t]+$/,"");if(!NF)next
    gsub(/ *\* */,"*");gsub(/ *\/ */,"/");gsub(/ *\+ */,"+");gsub(/ *- */,"-");gsub(/ *< */,"<");gsub(/ *> */,">");gsub(/ *= */,"=");gsub(/ *, */,",");gsub(/ *; */,";");gsub(/ *\{ */,"{");gsub(/ *\} */,"}");gsub(/ *\( */,"(");gsub(/ *\) */,")");gsub(/ *\[ */,"[");gsub(/ *\] */,"]");gsub(/ *\. */,".")
    printf"%s",$0} END{print""}' | tr -d '\n' | sed "s/\"/\\\\\"/g;s|^|static const char* $2 = \"|;s|$|\";|" > "$1.h"
}
for s in 'ssr.compute ssrCSSrc' 'rt.compute rtCSSrc' 'rtcopy.compute rtCopyCSSrc' 'voxels.compute voxUpdCSSrc' 'depth_prepass_vert.glsl depthPrepassVertSrc' 'depth_prepass.glsl depthPrepassFragSrc' 'debugunlit_vert.glsl debugUnlitVertSrc' 'debugunlit_frag.glsl debugUnlitFragSrc' \
         'chunk_vert.glsl vertSrc' 'chunk_frag.glsl fragSrc' 'ui_vert.glsl vertUISrc' 'ui_frag.glsl fragUISrc' 'text_vert.glsl textVertSrc' 'text_frag.glsl textFragSrc' 'composite_vert.glsl quadVertSrc' 'composite_frag.glsl quadFragSrc' 'shadowmap_vert.glsl shadowmapVertSrc' 'shadowmap_frag.glsl shadowmapFragSrc' \
         'particle_vert.glsl particleVertSrc' 'particle_frag.glsl particleFragSrc' 'trail_vert.glsl trailVertSrc' 'trail_frag.glsl trailFragSrc'; do gh Shaders/${s%% *} "${s#* }"; done
printf '#include "%s.h"\n' text_{vert,frag}.glsl depth_prepass_vert.glsl depth_prepass.glsl debugunlit_{vert,frag}.glsl chunk_{vert,frag}.glsl ui_{vert,frag}.glsl shadowmap_{vert,frag}.glsl composite_{vert,frag}.glsl particle_{vert,frag}.glsl trail_{vert,frag}.glsl {ssr,rt,rtcopy,voxels}.compute > Shaders/shaders.h
COMMON_CFLAGS="-ferror-limit=500 -fno-stack-protector -fno-unwind-tables -Wno-format-nonliteral -fvisibility=hidden -pipe -fno-ident -fdata-sections -Wno-int-to-void-pointer-cast \
               -Wshadow -ffunction-sections -ffast-math -std=c11 -Wall -Wextra -Wno-implicit-fallthrough -Wno-switch -fdeclspec -fomit-frame-pointer -g0 -Wunused \
               -Wno-overlength-strings -fno-math-errno -fno-sanitize=all -m64 -Oz -march=haswell -mf16c -mavx -Wbool-conversion -Wno-empty-body -nostdinc -ftrivial-auto-var-init=zero"
COMMON_LFLAGS="-Wl,-z,relro,-z,now,--gc-sections,--as-needed,--build-id=none,--print-gc-sections"
[[ $BUILD_MODE == debug ]] && COMMON_CFLAGS+=" -g -O0 -fno-omit-frame-pointer -funwind-tables"
if [ "$PLATFORM" = "windows" ]; then
    CC="zig cc -target x86_64-windows-gnu -Wframe-larger-than=65536"
    LINKER=$CC
    CFLAGS="-D_WIN32 $COMMON_CFLAGS -fno-strict-aliasing -Wl,-Bstatic -lmingw32 -lmingwex"
    LDFLAGS="$COMMON_LFLAGS -L. -lgdi32 -lole32 -static-libgcc"
    BINARY_NAME="voxen.exe"
else
    CC="zig cc"
    LINKER=$CC
    CFLAGS="$COMMON_CFLAGS -fstrict-aliasing -fmerge-all-constants -fno-plt -fno-semantic-interposition -fno-builtin"
    LDFLAGS="$COMMON_LFLAGS -target x86_64-linux-gnu.2.7 -ldl"
    BINARY_NAME="voxen"
fi
export CC=$CC
export CFLAGS=$CFLAGS
if [ "$BUILD_MODE" = "debug" ] && [ "$PLATFORM" = "windows" ]; then
    CFLAGS="${CFLAGS//-fvisibility=hidden/} -gcodeview"
    export CFLAGS=$CFLAGS
fi
export TEMP_DIR=temp_build
BINARY_PDB="${BINARY_PDB:-${BINARY_NAME%.exe}.pdb}"
SOURCES="voxen.c physics.c entity.c lib.c citadel.c ai.c weapons.c text.c textures.c models.c biomonitor.c culling.c particles.c"
printf '%s\n' $SOURCES | xargs -P12 -I{} sh -c '$CC -c "$1" $CFLAGS -o "temp_build/$1.o"' _ {}
if [ "$BUILD_MODE" = "debug" ] && [ "$PLATFORM" = "windows" ]; then
    MINGW_LIB="${MINGW_LIB:-/usr/x86_64-w64-mingw32/lib}"
    MINGW_GCC_LIB="${MINGW_GCC_LIB:-$(dirname "$(ls /usr/lib/gcc/x86_64-w64-mingw32/*/libgcc.a 2>/dev/null | head -1)" 2>/dev/null)}"
    LINKER="lld-link"
    LDFLAGS="/nologo /debug /pdb:$BINARY_PDB /out:$BINARY_NAME /machine:x64 /subsystem:console /entry:mainCRTStartup /nodefaultlib /opt:ref /incremental:no /fixed"
    LDFLAGS="$LDFLAGS /alternatename:__image_base__=__ImageBase"
    LDFLAGS="$LDFLAGS $MINGW_LIB/crt2.o /libpath:$MINGW_LIB /libpath:$MINGW_GCC_LIB libmingw32.a libmingwex.a libgcc.a libgcc_eh.a libmsvcrt.a libkernel32.a libuser32.a libgdi32.a libole32.a"
    LINK_OUTPUT=""
else
    LINKER=$CC
    LINK_OUTPUT="-o $BINARY_NAME"
fi
$LINKER "$TEMP_DIR"/*.o $LDFLAGS $LINK_OUTPUT
if ! $IS_CI; then
    strip --strip-all "$BINARY_NAME"; upx -qqq --best --lzma "$BINARY_NAME"
    echo "Built engine as game in $(( $(now_ms) - shader_start )) ms"
    if [[ $PLATFORM == windows ]]; then
        wine "./$BINARY_NAME"
    else
        "./$BINARY_NAME"
    fi
elif [[ $BUILD_MODE == debug ]]; then
    echo "Debug build: $BINARY_NAME carries DWARF (-g -O0 -fno-omit-frame-pointer), unstripped. Debug with: gdb --args ./$BINARY_NAME"
fi
[[ $BUILD_MODE == debug ]] || rm -f "$BINARY_PDB"
rm -f Shaders/*.h voxen.upx #Cleanup after quitting. Doesn't affect build timer. Gives me a chance to trivially copy out .o files if I want.
rm -r temp_build
