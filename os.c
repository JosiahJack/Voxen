// os.c - Operating System calls shim layer.
#include "os.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
void DualLog(const char* fmt, ...);
void DualLogWarn(const char* fmt, ...);
void DualLogError(const char* fmt, ...);

void OS_Exit(int errorNumber) { _exit(errorNumber); }

int64_t OS_RawWrite(int fd, const void* buf, size_t count, const char* filePath) {
    register int64_t rax __asm__("rax") = 1;   // sys_write
    register int     rdi __asm__("rdi") = fd;
    register const void* rsi __asm__("rsi") = buf;
    register size_t  rdx __asm__("rdx") = count;
    __asm__ __volatile__(
        "syscall"
        : "+r"(rax)                     // Output: rax is updated by the kernel
        : "r"(rdi), "r"(rsi), "r"(rdx)  // Inputs
        : "rcx", "r11", "memory"        // Clobbered by syscall
    );

    if (rax < 0) { DualLogError("Write error when attempting write to %s: %s (code: %d)\n", filePath, rax, (int)(-rax)); OS_Exit(1); } // Errors are returned as -1 to -4095

    return rax; // Returns bytes written
}

void OS_Write(int fd, const void* buffer, size_t size, const char* filePath) {
    size_t total = 0;
    while (total < size) {
        int64_t written = OS_RawWrite(fd, (const char*)buffer + total, size - total, filePath);

        if (written == 0 && size > 0) { DualLogError("Zero bytes written to %s (Disk full or EOF)\n", filePath); return; } // Handle the rare case of a 0-byte return (which could cause an infinite loop)
        total += (size_t)written;
    }
}

int OS_OpenReadonly(const char* filePath) {
    int fp = open(filePath, O_RDONLY);
    if (fp < 0) { DualLog("Could not find file %s\n", filePath); return -1; }
    return fp;
}

int OS_OpenWriteonly(const char* filePath) {
    int fp = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (!fp) { DualLogError("Failed to open %s\n", filePath); OS_Exit(1); }
    return fp;
}

int OS_FileSize(int fileDescriptor) {
    struct stat fileStatisticsStruct;
    fstat(fileDescriptor, &fileStatisticsStruct);
    return fileStatisticsStruct.st_size;
}

void* OS_AllocateFileBackedRAMReadonly(size_t size, int32_t fileDescriptor, char* filePath) {
    void* ramSpacePointer = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fileDescriptor, 0);
    if (ramSpacePointer == MAP_FAILED) { DualLogError("Failed to mmap %s\n", filePath); return NULL; }
    return ramSpacePointer;
}

void* OS_OpenAndAllocateFileBufferReadonly(const char* filePath, int* fileDescriptor, int* size) {
    *fileDescriptor = OS_OpenReadonly(filePath);
    if (*fileDescriptor < 0) { *size = 0; return NULL; }
    *size = OS_FileSize(*fileDescriptor);
    if (*size == 0) { DualLogWarn("Warning: File %s is empty, skipping allocation.\n", filePath); OS_Close(*fileDescriptor); return NULL; }
    
    void* ramSpacePointer = OS_AllocateFileBackedRAMReadonly(*size, *fileDescriptor, (char*)filePath);
    OS_Close(*fileDescriptor);
    return ramSpacePointer;
}

void* OS_DeallocateRAM(void* ramSpacePointer, size_t size) {
    if (!ramSpacePointer || ramSpacePointer == MAP_FAILED) { DualLogError("Attempting to double free!\n"); OS_Exit(1); }
    
    munmap(ramSpacePointer, size);
    return NULL;
}

void OS_Close(int fileDescriptor) { close(fileDescriptor); }

void OS_CPUInfo(void) {
    char cpu_brand[256];
    cpu_brand[0] = 0;
    char buf[4096];
    int fd = open("/proc/cpuinfo", O_RDONLY);
    if (fd >= 0) {
        int n = read(fd, buf, sizeof(buf)-1);
        close(fd);
        if (n > 0) {
            buf[n] = 0;
            const char *key = "model name";
            int keylen = 10;
            const char *p = buf;
            const char *end = buf + n;
            while (p < end) {
                const char *line = p;
                while (p < end && *p != '\n') p++;
                const char *line_end = p;
                if (p < end) p++;
                if (line_end - line > keylen && !__builtin_memcmp(line, key, keylen)) {
                    const char *colon = line;
                    while (colon < line_end && *colon != ':') colon++;
                    if (colon < line_end) {
                        colon++;
                        while (colon < line_end && *colon == ' ') colon++;
                        int k = 0;
                        while (colon < line_end && k < 255) cpu_brand[k++] = *colon++;
                        cpu_brand[k] = 0;
                    }
                    
                    break;
                }
            }
        }
    }

    if (!cpu_brand[0]) {
        const char *u = "Unknown CPU";
        int k = 0;
        while (u[k] && k < 255) { cpu_brand[k] = u[k]; k++; }
        cpu_brand[k] = 0;
    }

    int logical_cores = 1;
    char online[128];
    int fd2 = open("/sys/devices/system/cpu/online", O_RDONLY);
    if (fd2 >= 0) {
        int n = read(fd2, online, sizeof(online) - 1);
        close(fd2);
        if (n > 0) {
            online[n] = 0;
            int total = 0;
            int i = 0;
            while (online[i]) {
                while (online[i] == ',' || online[i] == ' ' || online[i] == '\n' || online[i] == '\t') i++;
                if (!online[i]) break;

                int a = 0, have_a = 0;
                while (online[i] >= '0' && online[i] <= '9') {
                    a = a * 10 + (online[i] - '0');
                    i++;
                    have_a = 1;
                }
                
                if (!have_a) break;

                if (online[i] == '-') {
                    i++;
                    int b = 0, have_b = 0;
                    while (online[i] >= '0' && online[i] <= '9') {
                        b = b * 10 + (online[i] - '0');
                        i++;
                        have_b = 1;
                    }
                    
                    total += (have_b && b >= a) ? (b - a + 1) : 1;
                } else {
                    total += 1;
                }
            }

            if (total > 0) logical_cores = total;
        }
    }

    DualLog("CPU: %s | Logical cores: %d\n", cpu_brand, logical_cores);
}
