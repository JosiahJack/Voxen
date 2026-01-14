// os.h - starts most translation units and defines the shim layer between Voxen and the OS as well as defining project wide OS defines.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
    #define WINDOWS
    #include <windows.h>
    #include <io.h>           // for _get_osfhandle
    #include <fcntl.h>        // O_RDONLY etc.
    typedef HANDLE OsFileHandle;
    #define OS_INVALID_HANDLE INVALID_HANDLE_VALUE
    HANDLE OS_OpenReadonly(const char* filePath);
#else
    #if defined(__linux__) && !defined(__ANDROID__)
        #define LINUX
    #endif

    #if defined(__ANDROID__)
        #define ANDROID
    #endif
    
    #if defined(__APPLE__)
        #define MAC
    #endif
    typedef int OsFileHandle;
    #define OS_INVALID_HANDLE -1
    #include <sys/stat.h>
    #include <sys/mman.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    int OS_OpenReadonly(const char* filePath);
#endif

void OS_Exit(int errorNumber);
void OS_Write(OsFileHandle fd, const void* buffer, size_t size, const char* filePath);
OsFileHandle OS_OpenReadonly(const char* filePath);
OsFileHandle OS_OpenWriteonly(const char* filePath);
void* OS_AllocateFileBackedRAMReadonly(size_t size, OsFileHandle fileDescriptor, char* filePath);
void* OS_AllocateRAM(void* addr, size_t length, int prot, int flags, OsFileHandle fd, off_t offset);
void* OS_DeallocateRAM(void* ramSpacePointer, size_t size);
void* OS_OpenAndAllocateFileBufferReadonly(const char* filePath, OsFileHandle* fileDescriptor, int* size);
void OS_Close(OsFileHandle fileDescriptor);
int OS_FileSize(OsFileHandle fileDescriptor);
void OS_CPUInfo(void);

void DualLog(const char* fmt, ...);
void DualLogWarn(const char* fmt, ...);
void DualLogError(const char* fmt, ...);
