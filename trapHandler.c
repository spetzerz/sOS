#include "kernel.h"
#include "common.h"
#include "trapHandler.h"

void trapHandler(void) {
    uint32_t trapCause = csrRead(scause);
    uint32_t trapVal = csrRead(stval);
    uint32_t trapPC = csrRead(sepc);
    uint32_t sstatus = csrRead(sstatus);
    uint32_t causedByInterrupt = trapCause & 0x80000000;
    trapCause = trapCause & 0x7FFFFFFF;
    
    if (causedByInterrupt) {
        // temporary
        panic("Interrupt");
    } else {
        // also temporary
        // ecall from user mode
        if (trapCause == 8) {
            trapPC += 4;
            csrWrite(sepc, trapPC);
            if ((sstatus & (1 << 5))) {
                __asm__ volatile("csrs sstatus, %0" :: "r"(1 << 1));
            }// we dont need to clear it if it was before cause it gets auto cleared on trap
            return;
        } else {
            panic("Haha dumass ur program broke lol, scause=%x, stval=%x, sepc=%x\n", trapCause, trapPC, trapVal);
        }
    }
}

__attribute__((naked, aligned(4))) // DO NOT, I REPEAT DO NOT LET YOUR DUMASS CHANGE THIS TO 2 LINES OR ELSE THE WHOLE OS WILL FUCK ITSELF AND DIE WHEN THERE IS AN EXCEPTION
void kernelTrapHandler(void) {
    __asm__ __volatile__(
        "csrw sscratch, sp\n"
        "addi sp, sp, -4*31\n"

        "sw ra, 4*0(sp)\n"
        "sw gp, 4*1(sp)\n"
        "sw tp, 4*2(sp)\n"
        "sw t0, 4*3(sp)\n"
        "sw t1, 4*4(sp)\n"
        "sw t2, 4*5(sp)\n"
        "sw fp, 4*6(sp)\n"
        "sw s1, 4*7(sp)\n"
        "sw a0, 4*8(sp)\n"
        "sw a1, 4*9(sp)\n"
        "sw a2, 4*10(sp)\n"
        "sw a3, 4*11(sp)\n"
        "sw a4, 4*12(sp)\n"
        "sw a5, 4*13(sp)\n"
        "sw a6, 4*14(sp)\n"
        "sw a7, 4*15(sp)\n"
        "sw s2, 4*16(sp)\n"
        "sw s3, 4*17(sp)\n"
        "sw s4, 4*18(sp)\n"
        "sw s5, 4*19(sp)\n"
        "sw s6, 4*20(sp)\n"
        "sw s7, 4*21(sp)\n"
        "sw s8, 4*22(sp)\n"
        "sw s9, 4*23(sp)\n"
        "sw s10, 4*24(sp)\n"
        "sw s11, 4*25(sp)\n"
        "sw t3, 4*26(sp)\n"
        "sw t4, 4*27(sp)\n"
        "sw t5, 4*28(sp)\n"
        "sw t6, 4*29(sp)\n"

        "csrr a0, sscratch\n"
        "sw a0, 4*30(sp)\n"
        "lw a0, 4*8(sp)\n" // reload a0 with original value
        // DO NOT FUCKING OVERWRITE THE STACK POINTER 
        "call trapHandler\n"

        "lw ra, 4*0(sp)\n"
        "lw gp, 4*1(sp)\n"
        "lw tp, 4*2(sp)\n"
        "lw t0, 4*3(sp)\n"
        "lw t1, 4*4(sp)\n"
        "lw t2, 4*5(sp)\n"
        "lw fp, 4*6(sp)\n"
        "lw s1, 4*7(sp)\n"
        "lw a0, 4*8(sp)\n"
        "lw a1, 4*9(sp)\n"
        "lw a2, 4*10(sp)\n"
        "lw a3, 4*11(sp)\n"
        "lw a4, 4*12(sp)\n"
        "lw a5, 4*13(sp)\n"
        "lw a6, 4*14(sp)\n"
        "lw a7, 4*15(sp)\n"
        "lw s2, 4*16(sp)\n"
        "lw s3, 4*17(sp)\n"
        "lw s4, 4*18(sp)\n"
        "lw s5, 4*19(sp)\n"
        "lw s6, 4*20(sp)\n"
        "lw s7, 4*21(sp)\n"
        "lw s8, 4*22(sp)\n"
        "lw s9, 4*23(sp)\n"
        "lw s10, 4*24(sp)\n"
        "lw s11, 4*25(sp)\n"
        "lw t3, 4*26(sp)\n"
        "lw t4, 4*27(sp)\n"
        "lw t5, 4*28(sp)\n"
        "lw t6, 4*29(sp)\n"
        "lw sp, 4*30(sp)\n"
        "sret\n"
    );
}
