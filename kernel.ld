/* I HAVE NO FUCKING CLUE WHAT ANY OF THIS MEANS
  ALL I KNOW IS THAT IT SETS THE SECTIONS FOR THE C
  PROGRAM AND IT MAKES IT HAPPY
*/

ENTRY(boot)

SECTIONS {
    . = 0x80200000;
    
    __kernel = .;
    .text : ALIGN(4096){
        __text = .;
        KEEP(*(.text.boot));
        *(.text .text.*);
        . = ALIGN(4096);
        __text_end = .;
    }

    .rodata : ALIGN(4096) {
        __rodata = .;
        *(.rodata .rodata.*);
        . = ALIGN(4096);
        __rodata_end = .;
    }

    .data : ALIGN(4096) {
        __data = .;
        *(.data .data.*);
        . = ALIGN(4096);
        __data_end = .;
    }

    .bss : ALIGN(4096) {
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        . = ALIGN(4096);
        __bss_end = .;
    }
    
    .stack : ALIGN(4096) {
    __stack_bottom = .;
    . += 128 * 1024; /* 128KB */
    __stack_top = .;
    }

    .ram : ALIGN(4096) {
    __kernel_end = .;
    __free_ram = .;
    . += 64 * 1024 * 1024; /* 128 MB */
    __free_ram_end = .;
    }
}
