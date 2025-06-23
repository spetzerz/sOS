#include "common.h"
#include "kernel.h"
#include "trapHandler.h"
#include "memoryHandler.h"

extern char __bss[], __bss_end[];
extern char __stack_top[], __stack_bottom[];
extern char __text[], __text_end[];
extern char __rodata[], __rodata_end[];
extern char __data[], __data_end[];
extern char __free_ram[], __free_ram_end[];

void kernelMain(void) {
    // init
    memset(__bss, 0, (size_t*)__bss_end - (size_t*)__bss);
    initAllocator((uint32_t*)__free_ram);
    OSprintf("%dKB of Ram Available\n", (((uint32_t)__free_ram_end-(uint32_t)__free_ram)>>20));
    OSprintf("Ram Begin: 0x%x\nRam End: 0x%x\n", (uint32_t*)__free_ram, (uint32_t*)__free_ram_end);

    // set the address for the kernelTrapHandler
    csrWrite(stvec, (uint32_t) kernelTrapHandler); 

    // testing functions n shi
    uint32_t pagesToAllocate = 1; 
    // Stress test for the allocator
    paddr_t *pAddress1 = allocateMemory(pagesToAllocate);
    OSprintf("%d Pages Allocated Starting at: 0x%x\n", pagesToAllocate, pAddress1);
    paddr_t *pAddress2 = allocateMemory(pagesToAllocate);
    OSprintf("%d Pages Allocated Starting at: 0x%x\n", pagesToAllocate, pAddress2);
    deallocateMemory(pAddress2, pagesToAllocate);
    deallocateMemory(pAddress1, pagesToAllocate);
    pAddress1 = allocateMemory(2);
    OSprintf("1 Pages Allocated Starting at: 0x%x\n", pAddress1);
    deallocateMemory(pAddress1, 2);
    pAddress1 = allocateMemory(1);
    OSprintf("%d Pages Allocated Starting at: 0x%x\n", pagesToAllocate, pAddress1);

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

