#pragma once
#include <stddef.h>
#include <stdint.h>
void OS_Write(int fd, const void* buffer, size_t size, const char* filePath);
void OS_Exit(int errorNumber);
void* OS_AllocateFileBackedRAMReadonly(size_t size, int fileDescriptor, char* filePath);
void* OS_DeallocateRAM(void* ramSpacePointer, size_t size);
int OS_OpenReadonly(const char* filePath);
int OS_OpenWriteonly(const char* filePath);
void OS_Close(int fileDescriptor);
void* OS_OpenAndAllocateFileBufferReadonly(const char* filePath, int* fileDescriptor, int* size);
int OS_FileSize(int fileDescriptor);
void OS_CPUInfo(void);
