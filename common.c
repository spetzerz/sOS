#include "common.h"

struct sbiret sbiCall(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                        : "=r"(a0), "=r"(a1)
                        : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                        : "memory");

    return (struct sbiret){.error = a0, .value = a1};
}

// https://tools.cloudbear.ru/docs/riscv-sbi-2.0-20231006.pdf page 15 Chaper 5. Legacy Extensions (EIDs #0x00-#0x0F) for the putchar 
void OSputchar(char c) {
    sbiCall(c, 0, 0, 0, 0, 0, 0, 1 /*putchar*/);
}

// budget printf
void OSprintf(const char *fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);

    while(*fmt) {
        if (*fmt == '%') {
            fmt++; // skip % and read character after
            switch (*fmt) {// read next character
                case '\0': // end of format string
                    OSputchar('%');
                    goto end;
                
                case '%':
                    OSputchar('%');
                    break;
                    
                case 's': {// string
                    const char *s = va_arg(vargs, const char *);
                    while (*s) {
                        OSputchar(*s);
                        s++;
                    }
                    break;
                }
                
                case 'd': {// integer in decimal
                    int value = va_arg(vargs, int);
                    unsigned magnitude = value;
                    if (value < 0) {
                        OSputchar('-');
                        magnitude = -magnitude;
                    }
                    
                    unsigned divisor = 1;
                    while (magnitude / divisor > 9) {
                        divisor *= 10;
                    }

                    while (divisor > 0) {
                        OSputchar('0' + magnitude/divisor);
                        magnitude %= divisor;
                        divisor /= 10;
                    }
                    break;
                }

                case 'x': {
                    unsigned value = va_arg(vargs, unsigned);
                    for (int i = 7; i >= 0; i--) {
                        unsigned nibble = (value >> (i*4)) & 0xf;
                        OSputchar("0123456789abcdef"[nibble]);
                    }
                }
                    
            }
        } else {
            OSputchar(*fmt);
        }

        fmt++;
    }

    end: 
        va_end(vargs);
}

void *memcpy(void *dst, const void *src, size_t n) {
    // cast to acess individual bytes
    uint8_t *d = (uint8_t *) dst;
    const uint8_t *s = (const uint8_t *) src;

    while (n--) {
        *d++ = *s++;
    }

    return dst;
}

void *memset(void *buf, char c, size_t n) {
    uint8_t *p = (uint8_t *) buf;
    /// set a certain region in memory to a char
    while (n--) {
        *p++ = c;
    }
    return buf;
}

char *strcpy(char *restrict dst, const char* restrict src) {
    char *d = dst;
    while (*src) {
        *d++ = *src++;
    }
    *d = '\0';
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            break;
        }
        s1++;
        s2++;
    }

    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

uint32_t bitscan(uint32_t x) {
    if(x == 0) return 0x0;
    x = x & -x;
    uint32_t ret = 0;
    if(x & 0xFFFF0000) ret += 16;
    if(x & 0xFF00FF00) ret += 8;
    if(x & 0xF0F0F0F0) ret += 4;
    if(x & 0xCCCCCCCC) ret += 2;
    if(x & 0xAAAAAAAA) ret += 1;
    return ret;
}

uint32_t pow2RoundUp(uint32_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

uint32_t pow(uint32_t x, uint32_t power) {
    if (power == 0) return 1;
    uint32_t result = x;
    for (uint32_t i = 1; i < power; i++) {
        result *= x;
    }
    return result;
}