#include "common.h"
#include "kernel.h"
#include "trapHandler.h"

extern char __bss[], __bss_end[], __stack_top[];

void kernelMain(void) {
    // init
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

    csrWrite(stvec, (uint32_t) kernelTrapHandler);

    // testing functions n shi
    OSprintf("\n\nHello %s\n", "World!");
    __asm__ __volatile__("unimp"); // temp: crashes the os by forcing a trap
    

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

