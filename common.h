#pragma once

struct sbiret {
    long error;
    long value;
};

// shared functions
struct sbiret sbiCall(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid);
void OSputchar(char c);
void OSprintf(const char *fmt, ...);


// c standard library stuff
typedef int bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t size_t;
typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;

#define true 1;
#define false 0;
#define NULL ((void *) 0)
#define align_up(value, align) __builtin_align_up(value, align);
#define is_aligned(value, align) __builtin_is_aligned(value, align);
#define offsetof (type, member) __builtin_offsetof(type, member);
#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

void *memset(void *buf, char c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
// TODO? probably implement strcpy_s for safe later but im to stupid to do that rn
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);