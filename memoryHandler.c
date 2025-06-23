#include "common.h"
#include "memoryHandler.h"

static uint32_t allocatorFreeBlocks[LEVELS][LEVEL_ARRAY_SIZE]; // MUltiply by max blocks count because there are x MB blocks in n ram space, and each x block cannot be any bigger so we need to potentially store all x combinations in 4KiB blocks
static uint32_t allocatorFreeBlocksCount[LEVELS]; // Budget variable list for buddy allocator

void initAllocator(uint32_t *__ram_start) {
    for (int i = 0; i < LEVELS; i++){
        allocatorFreeBlocksCount[i] = 0;
    }
    allocatorFreeBlocksCount[LEVELS-1] = 1;
    allocatorFreeBlocks[LEVELS-1][0] = (uint32_t)__ram_start;
}

paddr_t* allocateMemory(uint32_t pages) {
    if (pages > MAX_ALLOCATE_SIZE || pages == 0) {
        panic("Invalid Amound of Memory Requested\nPages Requested: %d\nIf a User is Seeing This, The OS May Be Corrupted", pages);
    }
    
    paddr_t *pAddress;

    uint32_t lookingLevel = bitscan(pow2RoundUp(pages));
    uint32_t searchLevel = lookingLevel;

    while (allocatorFreeBlocksCount[searchLevel] == 0) {
        if (searchLevel == LEVELS) {
            panic("Ran Out of Available Memory to Allocate");
        }
        searchLevel++;
    }

    uint32_t invertedSearchLevel = LEVELS;
    for (uint32_t i = 0; i < searchLevel; i++) {
        invertedSearchLevel--;
    }

    uint32_t splitAddress = 0;
    while (searchLevel > lookingLevel) {
        splitAddress = allocatorFreeBlocks[searchLevel][allocatorFreeBlocksCount[searchLevel]-1];
        allocatorFreeBlocks[searchLevel][allocatorFreeBlocksCount[searchLevel]-1] = 0;
        allocatorFreeBlocksCount[searchLevel]--;
        searchLevel--;
        
        allocatorFreeBlocks[searchLevel][allocatorFreeBlocksCount[searchLevel]++] = splitAddress;
        allocatorFreeBlocks[searchLevel][allocatorFreeBlocksCount[searchLevel]++] = splitAddress + (PAGE_SIZE<<searchLevel);
        invertedSearchLevel++;
        
    }

    pAddress = (paddr_t*)allocatorFreeBlocks[searchLevel][allocatorFreeBlocksCount[searchLevel]-1];
    allocatorFreeBlocksCount[searchLevel]--;
    return pAddress;
}

void deallocateMemory(paddr_t *pAddress, uint32_t pages) {
    if (!is_aligned((uint32_t)pAddress, PAGE_SIZE)) {
        panic("Unaligned Physical Address When Deallocating Memory\nPhysical Address: 0x%x\nOS May Be Corrupted", pAddress);
    }
    if (pages > MAX_ALLOCATE_SIZE || pages == 0) {
        panic("Invalid Amound of Memory Freed\nPages Freed: %d\nIf a User is Seeing This, The OS May Be Corrupted", pages);
    }

    uint32_t lookingLevel = bitscan(pow2RoundUp(pages));
    uint32_t searchingLevel = lookingLevel;
    allocatorFreeBlocks[lookingLevel][allocatorFreeBlocksCount[lookingLevel]++] = (uint32_t)pAddress;
    uint32_t pairToMatch;
    uint32_t buddyToMatch;

    bool foundPair = 1;
    uint32_t pairIndex;

    // NEEDS URGENT FIXING
    while (foundPair) {
        OSprintf("Search Level: %d\n", searchingLevel);
        if (searchingLevel == LEVELS-1) {
            break;
        }
        foundPair = 0;
        pairToMatch = allocatorFreeBlocks[searchingLevel][allocatorFreeBlocksCount[searchingLevel]-1];
        OSprintf("Pair to Match: 0x%x \n", pairToMatch);
        OSprintf("Buddy to Match: 0x%x \n", (pairToMatch ^ (1<<(12+searchingLevel))));
        for (pairIndex = 0; pairIndex < allocatorFreeBlocksCount[searchingLevel]; pairIndex++) {
            OSprintf("Checked: 0x%x \n", allocatorFreeBlocks[searchingLevel][pairIndex]);
        }
        for (pairIndex = 0; pairIndex < allocatorFreeBlocksCount[searchingLevel]; pairIndex++) {
            if (allocatorFreeBlocks[searchingLevel][pairIndex] == (pairToMatch ^ (1<<(11+searchingLevel)))) {
                foundPair = 1;
                break;
            }
        }
        if (foundPair) {
            allocatorFreeBlocks[searchingLevel][pairIndex] = 0;
            for (uint32_t i = pairIndex+1; i < allocatorFreeBlocksCount[searchingLevel]; i++) {
                allocatorFreeBlocks[searchingLevel][i-1] = allocatorFreeBlocks[searchingLevel][i]; // shift entire array
            }
            allocatorFreeBlocksCount[searchingLevel] -= 2;
            allocatorFreeBlocks[searchingLevel+1][allocatorFreeBlocksCount[searchingLevel+1]++] = (pairToMatch & ~(1<<(12+searchingLevel)));
        } else {
            OSprintf("No pair found\n", searchingLevel);
            break;
        }
        searchingLevel++;
    }
}


// TODO: CHANGE THIS
void mapPage(uint32_t *pageTable1, uint32_t *pageTables0Start, vaddr_t vAddress, paddr_t pAddress, uint32_t flags) {
    if (!is_aligned(vAddress, PAGE_SIZE)) {
        panic("Unaligned Virtual Address When Allocating Memory\nVirtual Address: 0x%x\nOS May Be Corrupted", vAddress);
    }
    if (!is_aligned(pAddress, PAGE_SIZE)) {
        panic("Unaligned Physical Address When Allocating Memory\nPhysical Address: 0x%x\nOS May Be Corrupted", pAddress);
    }

    uint32_t vpn1Index = (vAddress >> 22) & 0x3ff; // Extract the 10 bit field from the virtual address
    uint32_t vpn0Index = (vAddress >> 12) & 0x3ff;

    if (!(*(pageTable1 + vpn1Index) & PTEValid)) { // Create a new PTE
        *(pageTable1 + vpn1Index) = ((size_t)(pageTables0Start+vpn1Index) << 10) | PTEValid; // its a semi-const page table so we already know where the 0th level entry is, we're basically making a 2 level page table into a 1 level with this mapping
    }
    
    uint32_t *pageTable0 = pageTables0Start + vpn1Index * 1024; // "Create" one of the 0th level page tables
    *(pageTable0 + vpn0Index)  = ((pAddress >> 12) << 10) | flags | PTEValid;
}