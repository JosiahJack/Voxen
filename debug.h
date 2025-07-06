#ifndef VOXEN_DEBUG_H
#define VOXEN_DEBUG_H

void DualLog(const char *fmt, ...);
void DualLogError(const char *fmt, ...);
void print_bytes_no_newline(int count);
void DebugRAM(const char *context, ...);

#endif // VOXEN_DEBUG_H
