#include "common.h"
#include "memoryHandler.h"

static uint8_t allocatorNodes[BITMAP_LENGTH] = {0};
static paddr_t allocatorRamBegin;

static inline uint32_t levelOffset(uint32_t x) {
    return (1U << x) -1;
}

void initAllocator(paddr_t __ram_start) {
    allocatorNodes[levelOffset(LEVELS - 1)] |= ALLOCATOR_NODE_VALID;
    allocatorRamBegin = __ram_start;
}

paddr_t allocMemory(uint32_t pages) {
    if (pages > MAX_ALLOCATE_SIZE || pages == 0) {
        panic("Invalid Amount of Pages Requested: %d\n", pages);
    }

    paddr_t pAddress;

    uint32_t levelToLook = bitscan(pow2RoundUp(pages));
    uint32_t currentLevel = levelToLook;

    uint32_t indexFound;

    bool memoryFound = false;
    do {
        if (currentLevel >= LEVELS) {
            panic("No More Memory Left to Allocate");
        }
        OSprintf("Current Level: %d\n", currentLevel);
        OSprintf("Check Amount: %d\n", (1U << ((LEVELS-1)-currentLevel)));
        for (indexFound = 0; indexFound < (1U << ((LEVELS-1)-currentLevel)); indexFound++) {
            
            if ((allocatorNodes[levelOffset(currentLevel) + indexFound] & ALLOCATOR_NODE_VALID) && !(allocatorNodes[levelOffset(currentLevel) + indexFound] & ALLOCATOR_NODE_SPLIT) && !(allocatorNodes[levelOffset(currentLevel) + indexFound] & ALLOCATOR_NODE_ALLOCATED)) {
                OSprintf("Checkd: 0x%x\n", allocatorNodes[levelOffset(currentLevel) + indexFound]);
                OSprintf("At Index: %d\n", indexFound);
                memoryFound = true;
                break;
            }
        }
        if (!memoryFound) {
            currentLevel++;
        }
    } while (!memoryFound);
    
    while (currentLevel > levelToLook) {
        allocatorNodes[levelOffset(currentLevel) + indexFound] |= ALLOCATOR_NODE_SPLIT | ALLOCATOR_NODE_VALID;
        if (currentLevel > 0) {
            allocatorNodes[levelOffset(currentLevel-1) + 2*indexFound] |= ALLOCATOR_NODE_VALID;
            allocatorNodes[levelOffset(currentLevel-1) + 2*indexFound+1] |= ALLOCATOR_NODE_VALID;
            indexFound *= 2;
            currentLevel--;
        }
    }

    allocatorNodes[levelOffset(currentLevel) + indexFound] |= ALLOCATOR_NODE_ALLOCATED;
    pAddress = allocatorRamBegin + (indexFound * (PAGE_SIZE<<currentLevel));
    return pAddress;
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

