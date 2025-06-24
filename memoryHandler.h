#pragma once
#include "common.h"

#define MAX_ALLOCATE_SIZE 64*1024 * 1024 / PAGE_SIZE /* 64MB  In Pages*/
#define LEVELS 15 // log2(MAX_ALLOCATE_SIZE) + 1
#define BITMAP_LENGTH ((1U << LEVELS) )

#define ALLOCATOR_NODE_VALID 0x1
#define ALLOCATOR_NODE_SPLIT 0x2
#define ALLOCATOR_NODE_ALLOCATED 0x4

#define PTE_VALID 0x1
#define PTE_READABLE 0x2
#define PTE_WRITEABLE 0x4
#define PTE_EXECUTEABLE 0x8
#define PTE_USER 0x10
#define PTE_GLOBAL 0x20
#define PTE_ACCESSED 0x40
#define PTE_DIRTY 0x80

#define SATP_VIRTUAL (1U<<31)

void mapPage(uint32_t *pageTable1, vaddr_t vAddress, paddr_t pAddress, uint32_t flags);
void demapPage(uint32_t *pageTable1, vaddr_t vAddress);
void initAllocator(paddr_t __ram_start);
paddr_t allocMemory(uint32_t pages);
void deallocMemory(paddr_t pAddress, uint32_t pages);