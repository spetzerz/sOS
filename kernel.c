#include "common.h"
#include "kernel.h"
#include "trapHandler.h"
#include "memoryHandler.h"

extern char __kernel[], __kernel_end[];
extern char __bss[], __bss_end[];
extern char __stack_top[], __stack_bottom[];
extern char __text[], __text_end[];
extern char __rodata[], __rodata_end[];
extern char __data[], __data_end[];
extern char __free_ram[], __free_ram_end[];

void kernelMain(void) {
    // init
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);
    initAllocator((uint32_t)__free_ram);
    OSprintf("%dMB of Ram Available\n", (((uint32_t)__free_ram_end-(uint32_t)__free_ram)>>20));
    OSprintf("Ram Begin: 0x%x\nRam End: 0x%x\n", (uint32_t*)__free_ram, (uint32_t*)__free_ram_end);

    // set the address for the kernelTrapHandler
    csrWrite(stvec, (uint32_t) kernelTrapHandler); 

    // testing functions n shi
    uint32_t currentAddress = (uint32_t)__kernel;
    
    uint32_t pageTable1 = allocMemory(1);
    while (currentAddress < (uint32_t)__kernel_end) {
        OSprintf("Page Mapped: 0x%x\n", currentAddress);
        mapPage((uint32_t*)pageTable1, currentAddress, currentAddress, PTE_EXECUTEABLE | PTE_WRITEABLE | PTE_READABLE);
        currentAddress += PAGE_SIZE;
    }
    __asm__ __volatile__("sfence.vma\n");
    csrWrite(satp, (SATP_VIRTUAL | (pageTable1 >> 12)));

    OSprintf("If this is displayed that means the Kernel Has Sucessfully Been Identity Mapped Into Virtual Memory\n");

    __asm__ __volatile__("ecall");
    OSprintf("If this is displayed that means an Ecall has been sucessfully triggered and Handled while in Virtual Memory\n");
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

