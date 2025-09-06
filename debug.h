#ifndef VOXEN_DEBUG_H
#define VOXEN_DEBUG_H

extern FILE *console_log_file;

void DualLog(const char *fmt, ...);
void DualLogWarn(const char *fmt, ...);
void DualLogError(const char *fmt, ...);
void print_bytes_no_newline(int count);
void DebugRAM(const char *context, ...);
void RenderLoadingProgress(int offset, const char* format, ...);

#endif // VOXEN_DEBUG_H
