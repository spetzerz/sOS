#pragma once
#include "common.h"

#define MAX_ALLOCATE_SIZE 64*1024 * 1024 / PAGE_SIZE /* 64MB  In Pages*/
#define MAX_ALLOCATE_REAL_SIZE MAX_ALLOCATE_SIZE*PAGE_SIZE
#define LEVELS 15 // log2(MAX_ALLOCATE_SIZE) + 1
#define LEVEL_ARRAY_SIZE MAX_ALLOCATE_SIZE // MAX_ALLOCATE_SIZE

#define PTEValid 0x1
#define PTEReadable 0x2
#define PTEWriteable 0x4
#define PTEExecutable 0x8
#define PTEUserPage 0x10
#define PTEGlobal 0x20
#define PTEAccessed 0x40
#define PTEDirty 0x80

void mapPage(uint32_t *pageTable1, uint32_t *pageTables0Start, vaddr_t vAddress, paddr_t pAddress, uint32_t flags);
void initAllocator(uint32_t *__ram_start);
paddr_t *allocateMemory(uint32_t pages);
void deallocateMemory(paddr_t *pAddress, uint32_t pages);