#pragma once
#include <stddef.h>
#include <stdint.h>
void OS_Exit(int errorNumber);
void* OS_AllocateFileBackedRAMReadonly(size_t size, int fileDescriptor, char* filePath);
void OS_MemoryAdviseDontNeed(void* ramSpacePointer, size_t size);
void* OS_DeallocateRAM(void* ramSpacePointer, size_t size);
int OS_OpenReadonly(const char* filePath);
void OS_Close(int fileDescriptor);
int OS_FileSize(int fileDescriptor);
void OS_CPUInfo(void);
