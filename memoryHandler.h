#pragma once
#include "common.h"

#define MAX_ALLOCATE_SIZE 64*1024 * 1024 / PAGE_SIZE /* 64MB  In Pages*/
#define LEVELS 15 // log2(MAX_ALLOCATE_SIZE) + 1
#define BITMAP_LENGTH ((1U << LEVELS) )

#define ALLOCATOR_NODE_VALID 0x1
#define ALLOCATOR_NODE_SPLIT 0x2
#define ALLOCATOR_NODE_ALLOCATED 0x4

#define PTEValid 0x1
#define PTEReadable 0x2
#define PTEWriteable 0x4
#define PTEExecutable 0x8
#define PTEUserPage 0x10
#define PTEGlobal 0x20
#define PTEAccessed 0x40
#define PTEDirty 0x80

void mapPage(uint32_t *pageTable1, uint32_t *pageTables0Start, vaddr_t vAddress, paddr_t pAddress, uint32_t flags);
void initAllocator(paddr_t __ram_start);
paddr_t allocMemory(uint32_t pages);
void deallocMemory(paddr_t pAddress, uint32_t pages);