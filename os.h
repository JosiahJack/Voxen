#pragma once
void OS_Exit(int errorNumber);
void* OS_AllocateFileBackedRAMReadonly(int size, int fileIndex, char* filePath);
void OS_MemoryAdviseDontNeed(void* ramSpacePointer, int size);
void* OS_DeallocateRAM(void* ramSpacePointer, int size);
int OS_OpenReadonly(const char* filePath);
void OS_Close(int fileIndex);
int OS_FileSize(int fileIndex);
void OS_CPUInfo(void);
