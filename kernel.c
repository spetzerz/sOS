#include "common.h"
#include "kernel.h"

extern char __bss[], __bss_end[], __stack_top[];

void kernelMain(void) {
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

    // testing functions n shi
    OSprintf("\n\nHello %s\n", "World!");
    panic("balls");

    // probably temp kernel loop
    for(;;) {
        __asm__ __volatile__("wfi");
    }
}

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot (void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n"
        "j kernelMain\n"
        :
        : [stack_top] "r" (__stack_top)
    );
}

