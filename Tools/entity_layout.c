/* Stub for clang's -fdump-record-layouts.  Never linked into the game; the
   parse_coverage_check and entity_dump_diff tools compile it to extract the
   authoritative Entity field list (names, offsets, sizes) straight from the
   compiler, so neither tool can drift from the struct. */
#include "common.h"
Entity voxen_layout_probe;
