#include <stdio.h>
#include "cli_args.h"

void cli_args_print_version() {
    printf("-----------------------------------------------------------\n");
    printf("Voxen v0.02.00 7/04/2025\nthe OpenGL Voxel Lit Rendering Engine\n\nby W. Josiah Jack\nMIT-0 licensed\n\n\n");
}

void cli_args_print_help() {
    printf("Voxen the OpenGL Voxel Lit Rendering Engine\n");
    printf("-----------------------------------------------------------\n");
    printf("        This is a rendering engine designed for optimized focused\n");
    printf("        usage of OpenGL making maximal use of GPU Driven rendering\n");
    printf("        techniques, a unified event system for debugging and log\n");
    printf("        playback, full mod support loading all data from external\n");
    printf("        files and using definition files for what to do with the\n");
    printf("        data.\n\n");
    printf("        This project aims to have minimal overhead, profiling,\n");
    printf("        traceability, robustness, and low level control.\n\n");
    printf("\n");
    printf("Valid arguments:\n");
    printf(" < none >\n    Runs the engine as normal, loading data from \n    neighbor directories (./Textures, ./Models, etc.)\n\n");
    printf("-v, --version\n    Prints version information\n\n");
    printf("play <file>\n    Plays back recorded log from current directory\n\n");
    printf("record <file>\n    Records all engine events to designated log\n    as a .dem file\n\n");
    printf("dump <file.dem>\n    Dumps the specified log into ./log_dump.txt\n    as human readable text.  You must provide full\n    file name with extension\n\n");
    printf("-h, --help\n    Provides this help text.  Neat!\n\n");
    printf("-----------------------------------------------------------\n");
}
