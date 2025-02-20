// Memory manager functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz
#include <stdlib.h>

#ifndef MM_H_
#define MM_H_

#define NUM_SRAM_REGIONS 4

typedef struct Node
{
    uint32_t pid;
    uint32_t size;
    uint16_t numberOfBlocks;
    uint16_t upperBound;
    uint16_t lowerBound;
    void *bottom_add;
}Node_;

extern Node_ allocations[12];

//extern Node* head;
//extern Node* last;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void * mallocFromHeap(uint32_t size_in_bytes);

//void freeToHeap(void *pMemory);
void freeToHeapImproved(void *p);

void mpuOverallBackground(void);
void allowFlashAccess(void);
void allowPeripheralAccess(void);
void setupSramAccess(void);
uint64_t createNoSramAccessMask(void);
void addSramAccessWindow(uint64_t *srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes);
void addSramAccessWindowImproved(uint64_t *srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes);
void applySramAccessMask(uint64_t srdBitMask);
void setSramAccessWindow(uint32_t *baseAdd,uint32_t size_in_bytes);
uint64_t srdMask(uint32_t *baseAdd,uint32_t stackBytes);

#endif
