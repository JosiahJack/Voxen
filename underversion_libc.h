// underversion_libc.h - Underversioning for libc symbols to ensure maximum compatibility with all libc versions
#pragma once
__asm__(".symver dlopen,dlopen@GLIBC_2.2.5");
__asm__(".symver dlerror,dlerror@GLIBC_2.2.5");
__asm__(".symver dlsym,dlsym@GLIBC_2.2.5");
__asm__(".symver dlclose,dlclose@GLIBC_2.2.5");
__asm__(".symver qsort,qsort@GLIBC_2.2.5");
__asm__(".symver snprintf,snprintf@GLIBC_2.2.5"); int snprintf(char *str, size_t size, const char *format, ...);
__asm__(".symver vsnprintf,vsnprintf@GLIBC_2.2.5"); int vsnprintf(char *str, size_t size, const char *format, __builtin_va_list ap);
__asm__(".symver fgets,fgets@GLIBC_2.2.5"); char *fgets(char *s, int size, FILE *stream);
__asm__(".symver __libc_start_main,__libc_start_main@GLIBC_2.2.5");
__asm__(".symver strncpy,strncpy@GLIBC_2.2.5"); char *strncpy(char *dest, const char *src, size_t n);
