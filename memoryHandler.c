#include "common.h"
#include "memoryHandler.h"

static uint8_t allocatorNodes[BITMAP_LENGTH];
static paddr_t allocatorRamBegin;

static inline uint32_t levelOffset(uint32_t x) {
    return (1U << x) -1;
}

void initAllocator(paddr_t __ram_start) {
    memset(&allocatorNodes, 0, BITMAP_LENGTH); // why not just in case
    
    allocatorNodes[0] |= ALLOCATOR_NODE_VALID;
    allocatorRamBegin = __ram_start;
}

paddr_t allocMemory(uint32_t pages) {
    if (pages > MAX_ALLOCATE_SIZE || pages == 0) {
        panic("Invalid Amount of Pages Requested: %d\n", pages);
    }

    paddr_t pAddress;
    uint32_t alignedPagesCount = pow2RoundUp(pages);

    uint32_t levelToLook = (LEVELS-1)-bitscan(alignedPagesCount);
    uint32_t currentLevel = levelToLook;

    uint32_t indexFound;

    bool memoryFound = false;
    do {
        if (currentLevel >= LEVELS) {
            panic("No More Memory Left to Allocate");
        }
        
        for (indexFound = 0; indexFound < (1U << currentLevel); indexFound++) {
            if ((allocatorNodes[levelOffset(currentLevel) + indexFound] & ALLOCATOR_NODE_VALID) && !(allocatorNodes[levelOffset(currentLevel) + indexFound] & ALLOCATOR_NODE_SPLIT) && !(allocatorNodes[levelOffset(currentLevel) + indexFound] & ALLOCATOR_NODE_ALLOCATED)) {
                memoryFound = true;
                break;
            }
        }
        if (!memoryFound) {
            currentLevel--;
        }
    } while (!memoryFound);
    
    while (currentLevel < levelToLook) {
        allocatorNodes[levelOffset(currentLevel) + indexFound] |= ALLOCATOR_NODE_SPLIT | ALLOCATOR_NODE_VALID;
        if (currentLevel < (LEVELS-1)) {
            allocatorNodes[levelOffset(currentLevel+1) + 2*indexFound] |= ALLOCATOR_NODE_VALID;
            allocatorNodes[levelOffset(currentLevel+1) + 2*indexFound+1] |= ALLOCATOR_NODE_VALID;
            currentLevel++;
        }
        // this is the same as indexFoudn *= 2 but eh 
        indexFound = indexFound << 1;
    }
    uint32_t offset = (indexFound * (PAGE_SIZE<<((LEVELS-1)-currentLevel)));
    
    allocatorNodes[levelOffset(currentLevel) + indexFound] |= ALLOCATOR_NODE_ALLOCATED;
    pAddress = allocatorRamBegin + offset;
    return pAddress;
}


void deallocMemory(paddr_t pAddress, uint32_t pages) {
    uint32_t alignedPagesCount = pow2RoundUp(pages);
    if (pages > MAX_ALLOCATE_SIZE || pages == 0) {
        panic("Amount of Pages Requested To Deallocate is Invalid");
    }
    uint32_t levelToLook = (LEVELS-1)-bitscan(alignedPagesCount);
    uint32_t currentLevel = levelToLook;
    uint32_t offsetInMemory = pAddress - allocatorRamBegin;
    uint32_t indexInTree = offsetInMemory / (PAGE_SIZE<<((LEVELS-1)-currentLevel));

    if ((allocatorNodes[levelOffset(currentLevel) + indexInTree] & ALLOCATOR_NODE_SPLIT) || !(allocatorNodes[levelOffset(currentLevel) + indexInTree] & ALLOCATOR_NODE_VALID)) {
        panic("Deallocation Address is Invalid");
    }
    if (!(allocatorNodes[levelOffset(currentLevel) + (indexInTree ^ 1)] & ALLOCATOR_NODE_VALID)) {
        panic("Allocator Broken, Buddy Somehow Isnt Valid");
    }

    bool pairFound = false;
    do {
        // FREE CURRENT NODE
        allocatorNodes[levelOffset(currentLevel) + indexInTree] &= 0;
        allocatorNodes[levelOffset(currentLevel) + indexInTree] |= ALLOCATOR_NODE_VALID;
        if (!(allocatorNodes[levelOffset(currentLevel) + (indexInTree ^ 1)] & ALLOCATOR_NODE_SPLIT) && !(allocatorNodes[levelOffset(currentLevel) + (indexInTree ^ 1)] & ALLOCATOR_NODE_ALLOCATED) && currentLevel > 0) { // CHECK IF BUDDY IS AVAILABLE
            // DE-VALID BOTH AND STEP DOWN A LEVEL
            allocatorNodes[levelOffset(currentLevel) + (indexInTree ^ 1)] &= 0;
            allocatorNodes[levelOffset(currentLevel) + indexInTree] &= 0;
            // DIVIDE BY 2 TO GET THE INDEX IN THE LEVEL ABOVE
            indexInTree = indexInTree >> 1;
            currentLevel--;
            pairFound = true;
        } else {
            pairFound = false;
        }
    } while (pairFound);
}


void mapPage(uint32_t *pageTable1, vaddr_t vAddress, paddr_t pAddress, uint32_t flags) {
    if (!is_aligned(vAddress, PAGE_SIZE)) {
        panic("Unaligned Virtual Address When Mapping Page\nVirtual Address: 0x%x\nOS May Be Corrupted", vAddress);
    }
    if (!is_aligned(pAddress, PAGE_SIZE)) {
        panic("Unaligned Physical Address When Mapping Page\nPhysical Address: 0x%x\nOS May Be Corrupted", pAddress);
    }

    // EXTRACT THE PAGE TABLE INDEXES FROM THE VIRTUAL ADDRESSES
    uint32_t vpn1Index = (vAddress >> 22) & 0x3ff;
    uint32_t vpn0Index = (vAddress >> 12) & 0x3ff;

    if (!(*(pageTable1 + vpn1Index) & PTE_VALID)) { // Create a new PTE
        uint32_t pageTable0 = allocMemory(1);
        memset((uint8_t*)pageTable0, 0, 1024); // CLEAR THE NEW PAGE TABLE (FOR SAFTEY)
        *(pageTable1+vpn1Index) = ((pageTable0 / PAGE_SIZE) << 10) | PTE_VALID;
    }
    
    uint32_t *table0 = (uint32_t*)((*(pageTable1 + vpn1Index) >> 10) * PAGE_SIZE);
    *(table0 + vpn0Index)  = ((pAddress / PAGE_SIZE) << 10) | flags | PTE_VALID;
}


void demapPage(uint32_t *pageTable1, vaddr_t vAddress) {
    if (!is_aligned(vAddress, PAGE_SIZE)) {
        panic("Unaligned Virtual Address When Demapping Page\nVirtual Address: 0x%x\nOS May Be Corrupted", vAddress);
    }
    // EXTRACT THE PAGE TABLE INDEXES FROM THE VIRTUAL ADDRESSES
    uint32_t vpn1Index = (vAddress >> 22) & 0x3ff;
    uint32_t vpn0Index = (vAddress >> 12) & 0x3ff;

    uint32_t *table0 = (uint32_t*)((*(pageTable1 + vpn1Index) >> 10) * PAGE_SIZE);
    *(table0 + vpn0Index) = 0; // Clear Flags

    bool devalidateTable1 = 1;
    for (uint32_t i = 0; i < 1024; i++) {
        if (*(table0 + i) & PTE_VALID) {
            devalidateTable1 = 0;
            break;
        }
    }
    *(pageTable1 + vpn1Index) = 0;
    if (devalidateTable1) {
        deallocMemory((paddr_t)table0, 1);
    }
}