// Memory manager functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include <stdlib.h>
#include "mm.h"
#include "custom_str_library.h"
#include <stdbool.h>
#include <uart0.h>
#include "kernel.h"

#define smallBlockSize 512
#define largeBlockSize 1024
#define RW_FULL_ACCESS 011

uint64_t availability = 0;
uint16_t lastDeleted = 255;
//bool headNode = false;
//typedef struct Node
//{
//    uint32_t pid;
//    int32_t size;
//    uint16_t numberOfBlocks;
//    uint16_t upperBound;
//    uint16_t lowerBound;
//   // uint32_t * top_add;
//    uint32_t * bottom_add;
//    struct Node* next;
//}Node;




//extern Node* memoryInformation;




Node_ allocations[MAX_TASKS];
uint16_t deleted[10]={255,255,255,255,255,255,255,255,255,255};
//Node* last = NULL;
//Node* temp = NULL;
//Node* memoryInformation[12];
//extern Node* memoryInformation = NULL;


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------



int32_t indexOfContinousMemory(int32_t initialIndex, int16_t numberOfBlocksNeeded)
{
    int16_t index = 1000, j = 0, continousBlocks = 0;

    for(j = initialIndex; j < 40; j++)
    {

        if((availability & (1 << j))==0)
        {
            continousBlocks ++;
        }else{
            continousBlocks = 0;
        }

        if(continousBlocks == numberOfBlocksNeeded)
        {
            index = j - (continousBlocks - 1);
            break;
        }

    }


    return index;
}




void addTolistOfNodes(uint32_t processID,void *newPtr,uint32_t size,uint16_t lowerBound,uint16_t upperBound)
{

    static uint8_t i = 0;
    uint8_t j = 0, available = 255;
    bool found = false;
    for(j = 0; j < (MAX_TASKS - 2); j++)
    {
        if(deleted[j] != 255)
        {
            available = deleted[j];
            found = true;
            deleted[j] = 255;
            break;
        }
    }

    if(found)
    {
        allocations[available].pid =getMallocPID();
        allocations[available].bottom_add = newPtr;
        allocations[available].lowerBound = lowerBound;
        allocations[available].upperBound = upperBound;
        allocations[available].size = size;
    }else
    {
        allocations[i].pid =getMallocPID();
        allocations[i].bottom_add = newPtr;
        allocations[i].lowerBound = lowerBound;
        allocations[i].upperBound = upperBound;
        allocations[i].size = size;
        i++;
    }

//    if(i == MAX_TASKS)
//    {
//        i
//    }
//


//    if(lastDeleted == 255)
//    {



//    }else{
//        allocations[lastDeleted].pid =getMallocPID();
//        allocations[lastDeleted].bottom_add = newPtr;
//        allocations[lastDeleted].lowerBound = lowerBound;
//        allocations[lastDeleted].upperBound = upperBound;
//        allocations[lastDeleted].size = size;



//    }

}

//
//void addTolistOfNodes(uint32_t processID,void *newPtr,uint16_t size,uint16_t lowerBound,uint16_t upperBound)
//{
//   // char buffer[100];
//    Node* temp = NULL;
//
//    newPtr = (void*)((uint32_t)newPtr + 256);
//
//    if ((head == NULL) && (newPtr != NULL)) {
//        // Adding the first node
//        temp = (Node*)newPtr;             // Use the memory pointed by newPtr
//        temp->pid = processID;                   // Assign values to the node
//        temp->bottom_add = newPtr;
//        temp->lowerBound = lowerBound;
//        temp->upperBound = upperBound;
//        temp->size = size;
//        temp->next = NULL;
//        head = temp;                      // Set head to the first node
//        memoryInformation = temp;          ///DEBUG
//        last = temp;                      // Set last to the first node as well
//    }
//    else if ((head != NULL) && (newPtr != NULL)) {
//        // Adding subsequent nodes
//        temp = (Node*)newPtr;             // Use the memory pointed by newPtr
//        temp->pid = processID;                   // Assign values to the new node
//        temp->bottom_add = newPtr;
//        temp->size = size;
//        temp->lowerBound = lowerBound;
//        temp->upperBound = upperBound;
//        temp->next = NULL;
//        last->next = temp;                // Link the last node to the new node
//        last = temp;                      // Update last to the new node
//    }
//    else if (newPtr == NULL) {
//        putsUart0("Cannot allocate node:\n");
//    }
//
//}
//




// REQUIRED: add your malloc code here and update the SRD bits for the current thread
void * mallocFromHeap(uint32_t size_in_bytes)
{



    // allocation * allocationInfo = (allocation*)malloc_from_heap(sizeof(allocation));
     void *ptr =(void *)0x20002000;
     int32_t base_address = 0x20001000;
     uint16_t sizeOfBlock = 0;
     bool smallBlockAvailable  = false;
     bool biggerBlockAvailable  = false;
     int16_t indexSmallBlock = 1000, indexBigBlock = 1000;
     uint32_t pid = getMallocPID();

     uint16_t i = 0,j =0;
     for(i = 0; i < 24; i++)
     {
         if((availability & (1ULL << i))== 0)
         {
             smallBlockAvailable = true;
             indexSmallBlock = i;
             //i = 24;
             break;
         }
     }


     for(j = 24; j < 40; j++)
     {
         if((availability & (1ULL << j)) ==0)
         {
             biggerBlockAvailable = true;
             indexBigBlock = j;
             break;
         }
     }



     if(size_in_bytes <= 512 && smallBlockAvailable == true){
         sizeOfBlock = 512;
         //look in 4k blocks
         //allocate base address + (512 offset * indexSmallBlock)
         base_address = base_address+(indexSmallBlock * smallBlockSize);

         //ptr =(uint32_t *)base_address;
         addTolistOfNodes(pid,(uint32_t*)base_address,sizeOfBlock, indexSmallBlock,0);
         ptr =(void *)base_address;
         availability |= (1ULL << indexSmallBlock);

     }else if((size_in_bytes > 512) && (size_in_bytes < 1024) && (biggerBlockAvailable == true)){
         sizeOfBlock = 1024;
         // go to 8k blocks
         // (indexBigBlock - 12) 12 offset because of how 32k gets distributed between the 5 regions and subregions
         base_address = base_address+((indexBigBlock - 12) * largeBlockSize);
 //        ptr =(uint32_t *)base_address;

         addTolistOfNodes(pid,(uint32_t*)base_address,sizeOfBlock, indexBigBlock,0);

         ptr =(void *)base_address;
         availability |= (1ULL << indexBigBlock);

     }else if(size_in_bytes >= 1024){

         int16_t j = 0;
         int32_t blocksNeeded = 0;
         if((size_in_bytes % 1024) == 0){
             blocksNeeded = size_in_bytes/1024;
         }else{
             blocksNeeded = (int32_t)(size_in_bytes/1024) + 1;
         }

         sizeOfBlock = blocksNeeded * largeBlockSize;

         int16_t index = 0;

         index = indexOfContinousMemory(indexBigBlock,blocksNeeded);

         if(index == 1000) //No allocation available. Return Null ptr
         {
 //            ptr =(uint32_t *) NULL;
             ptr =(void *) NULL;
             return ptr;
         }


         base_address = base_address+((index-12) * largeBlockSize);

         addTolistOfNodes(pid,(uint32_t*)base_address,sizeOfBlock, index,(index+blocksNeeded));
         //ptr =(uint32_t *)base_address;
         ptr =(void *)base_address;
         for(j = index; j < (index+blocksNeeded);j++)
         {
             availability |= (1ULL << j);
         }

     }else if((size_in_bytes <= 512) && (smallBlockAvailable == false) && (biggerBlockAvailable == true)){  //Newly added. Controls the edge cases for 512. If no
         sizeOfBlock = 512;
         //If no small blocks are available, but there are 1024 blocks, then allocate to that region.

         // go to 8k blocks
         // (indexBigBlock - 12) 12 offset because of how 32k gets distributed between the 5 regions and subregions
 //        if(indexBigBlock < 33){
             base_address = base_address+((indexBigBlock - 12) * largeBlockSize);
 //        }else{
 //            base_address = base_address+((indexBigBlock - 8) * largeBlockSize);
 //        }



         //ptr =(uint32_t *)base_address;
         addTolistOfNodes(pid,(uint32_t*)base_address,sizeOfBlock, indexBigBlock,0);
         ptr =(void *)base_address;
         availability |= (1ULL << indexBigBlock);

     }
     else if((biggerBlockAvailable == false) && (smallBlockAvailable == false)){ //when no blocks are available
         //ptr =(uint32_t *) NULL;
         ptr =(void *) NULL;
     }

     return ptr;
}


//Task that is part of freeToHeap
void updateAvailability(uint16_t lowerBound, uint16_t upperBound)
{
    uint16_t i = 0;
    if(upperBound == 0){
        availability &= ~(1ULL << lowerBound);
    }else{
        for(i = lowerBound; i < upperBound;i++){
            availability &= ~(1ULL << i);
        }
    }

}





void freeToHeapImproved(void* ptr)
{

    uint8_t index = 0;
    uint32_t pid = 0;
    static uint16_t indexDeleted = 0;
    for(index = 0; index < MAX_TASKS; index++)
    {
        if(allocations[index].bottom_add == ptr)
        {
            pid = allocations[index].pid;
            break;
        }
    }



    for(index = 0; index < MAX_TASKS; index++)
    {
        if(allocations[index].pid == pid)
        {
            allocations[index].pid = 0;
            allocations[index].size = 0;
            allocations[index].numberOfBlocks = 0;
            updateAvailability(allocations[index].lowerBound,allocations[index].upperBound);
            allocations[index].bottom_add = NULL;

            deleted[indexDeleted] = index;
            indexDeleted++;
        }
    }
}


// REQUIRED: add your free code here and update the SRD bits for the current thread
//void freeToHeapImproved(void *p)
//{
//    //char buffer[100];
//       Node* removeThisNode = (Node *)p;
//       Node* temp = head;
//       Node* prev = NULL;
//       Node* specialCase = NULL;
//       uint32_t pid;
//
//       // Traverse and print all nodes before removing
//       putsUart0("Current heap before removal:\n");
//       temp = head;
//       while(temp != NULL)
//       {
//
//           if(temp == removeThisNode){
//               pid = temp->pid;
//               break;
//           }
//
//           temp = temp->next;
//       }
//
//        //Special case: Node to be removed is the head
//       if (head == removeThisNode) {
//           specialCase = head->next;
//
//           head->pid = 0;
//           head->size = 0;
//           head->bottom_add = (uint32_t*)NULL;
//           head->next = NULL;
//           updateAvailability(head->lowerBound,head->upperBound);
//           //head = head->next;  // Update head to the next node
//           //free(removeThisNode);  // Free the removed node
//           head = specialCase;
//           putsUart0("Head node removed.\n");
//           //break;
//       }
//       else {
//           // Traverse the list to find the node to remove
//           prev = head;
//           temp = head->next;
//           while (temp != NULL) {
//               if (temp == removeThisNode) {
//                   prev->next = temp->next;  // Unlink the node
//                   // Free the removed node
//                   // NEEDS TO UNLINK the global variable fo the index
//                   //freed
//                   temp->pid = 0;
//                   temp->size = 0;
//                   temp->bottom_add =(uint32_t*)NULL;
//                   temp->next = NULL;
//                   updateAvailability(temp->lowerBound,temp->upperBound);
//                  // putsUart0("Node removed.\n");
//                   break;
//               }
//               prev = temp;
//               temp = temp->next;
//           }
//       }



//       // Traverse the list to find the node to remove
//       prev = head;
//       temp = head->next;
//       while (temp != NULL) {
//           if (temp->pid == pid) {
//               prev->next = temp->next;  // Unlink the node
//               // Free the removed node
//               // NEEDS TO UNLINK the global variable fo the index
//               //freed
//               temp->pid = 0;
//               temp->size = 0;
//               temp->bottom_add =(uint32_t*)NULL;
//               temp->next = NULL;
//               updateAvailability(temp->lowerBound,temp->upperBound);
//               putsUart0("Node removed.\n");
//               break;
//           }
//           prev = temp;
//           temp = temp->next;
//       }


//}



// REQUIRED: add your free code here and update the SRD bits for the current thread
//void freeToHeap(void *pMemory)
//{
//    char buffer[100];
//       Node* removeThisNode = (Node *)pMemory;
//       Node* temp = head;
//       Node* prev = NULL;
//       Node* specialCase = NULL;
//
//
//       // Traverse and print all nodes before removing
//       putsUart0("Current heap before removal:\n");
//       temp = head;
//       while(temp != NULL)
//       {
//           putsUart0("Heap Node info:\t");
//           intToAscii(temp->pid, buffer);
//           putsUart0(buffer);
//           putsUart0("\t");
//           uintToHex((uint32_t)temp->bottom_add, buffer);
//           putsUart0(buffer);
//           putsUart0("size of allocation:\t");
//           intToAscii(temp->size, buffer);
//           putsUart0(buffer);
//           putsUart0("\n");
//           temp = temp->next;
//       }
//
//       // Special case: Node to be removed is the head
//       if (head == removeThisNode) {
//           specialCase = head->next;
//           head->pid = 0;
//           head->size = 0;
//           head->bottom_add = (uint32_t*)NULL;
//           head->next = NULL;
//           updateAvailability(head->lowerBound,head->upperBound);
//           //head = head->next;  // Update head to the next node
//           //free(removeThisNode);  // Free the removed node
//           head = specialCase;
//           putsUart0("Head node removed.\n");
//           //break;
//       }
//       else {
//           // Traverse the list to find the node to remove
//           prev = head;
//           temp = head->next;
//           while (temp != NULL) {
//               if (temp == removeThisNode) {
//                   prev->next = temp->next;  // Unlink the node
//                   // Free the removed node
//                   // NEEDS TO UNLINK the global variable fo the index
//                   //freed
//                   temp->pid = 0;
//                   temp->size = 0;
//                   temp->bottom_add =(uint32_t*)NULL;
//                   temp->next = NULL;
//                   updateAvailability(temp->lowerBound,temp->upperBound);
//                   //putsUart0("Node removed.\n");
//                   break;
//               }
//               prev = temp;
//               temp = temp->next;
//           }
//       }
//
//       // Print the list after removal
//       putsUart0("Heap after removal:\n");
//       temp = head;
//       while(temp != NULL)
//       {
//           putsUart0("updated heap:\t");
//           intToAscii(temp->pid, buffer);
//           putsUart0(buffer);
//           putsUart0("\t");
//           uintToHex((uint32_t)temp->bottom_add, buffer);
//           putsUart0(buffer);
//           putsUart0("\n");
//           temp = temp->next;
//       }
//
//}


//MPU OverallBackground, then flash, then peripheral
//Bit 0 is ENABLE, Bit 2 PRIVDEFEN  When this bit is set,
//the background region acts as if it is region number -1.
//Any region that is defined and enabled has priority over this default map
void mpuOverallBackground(void)
{
    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_PRIVDEFEN| NVIC_MPU_CTRL_ENABLE;
}





// REQUIRED: include your solution from the mini project
void allowFlashAccess(void)
{
    // Base address 0x0 size 0x0003FFFF = 262143 Bytes
    // N = log_2(262143) = 17.99 = 18
    NVIC_MPU_NUMBER_R = 5;
    NVIC_MPU_BASE_R = 0x0;
    //NVIC_MPU_ATTR_R &= ~(NVIC_MPU_ATTR_XN);//Bit 28 is zero to allow for "x" excution
    //RW bit 26,25,24 to 011 for Privileged/Unprivileged.
    //            Table 3-6 TEX is always 000S0C1B0   // 18-1= 17
    NVIC_MPU_ATTR_R |= (3 << 24) | (2 << 16) | (17 << 1) | NVIC_MPU_ATTR_ENABLE;

}

void allowPeripheralAccess(void)
{
    // Base address 0x40000000 to 0x43FFFFFF =  67108863 Bytes
    // N = log_2(67108863) = 25.99 =26
    NVIC_MPU_NUMBER_R = 6;
    NVIC_MPU_BASE_R = 0x40000000;
    //Bit 28 instruction execution disabled.
    NVIC_MPU_ATTR_R &=~(NVIC_MPU_ATTR_XN);
    //NVIC_MPU_ATTR_R &=~(0b11111111 << 8);
    //RW bit 26,25,24 to 011 for Privileged/Unprivileged.
    //          Table 3-6 TEX is always 000S1C0B1       // 26-1= 25
    NVIC_MPU_ATTR_R |= (3 << 24) | (5 << 16) | (25 << 1) | NVIC_MPU_ATTR_ENABLE;
   // NVIC_MPU_ATTR_R |= (0b011 << 24) | (0b000101 << 16) | (0b11010 << 1) | NVIC_MPU_ATTR_ENABLE;
}

void setupSramAccess(void)
{
    //SRAM has (3 * 4k) and (2 * 8k) regions.

     //FIRST  4k Region
     //Base address 0x20001000 to 20001FFF = 4095 Bytes
     // N = log_2(4095) = 11.99 =12
     NVIC_MPU_NUMBER_R = 0;
     NVIC_MPU_BASE_R = 0x20001000;
     //                |-x               |AP BITS                                        |12 - 1 = 11
     NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_XN|(3 << 24)|(6 << 16)|(0xFF << 8)| (11 << 1)| NVIC_MPU_ATTR_ENABLE;


     //SECOND 4k Region
     //Base address 0x20001000 to 20002FFF = 4095 Bytes
     // N = log_2(4095) = 11.99 =12
     NVIC_MPU_NUMBER_R = 1;
     NVIC_MPU_BASE_R = 0x20002000;
     //                |-x               |AP BITS
     NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_XN|(3 << 24)|(6 << 16)|(0xFF << 8)| (11 << 1)| NVIC_MPU_ATTR_ENABLE;


     //THIRD 4k Region
     //Base address 0x20001000 to 20003FFF = 4095 Bytes
     // N = log_2(4095) = 11.99 =12
     NVIC_MPU_NUMBER_R = 2;
     NVIC_MPU_BASE_R = 0x20003000;
     //                |-x               |AP BITS
     NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_XN|(3 << 24)|(6 << 16)|(0xFF << 8)| (11 << 1)| NVIC_MPU_ATTR_ENABLE;

     //FIRST 8k Region
     //Base address 0x20004000 to 20005FFF = 8192 Bytes
     // N = log_2(8192) = 13
     NVIC_MPU_NUMBER_R = 3;
     NVIC_MPU_BASE_R = 0x20004000;
     //                |-x               |AP BITS                                        |13 - 1 = 12
     NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_XN|(3 << 24)|(6 << 16)|(0xFF << 8)| (12 << 1)| NVIC_MPU_ATTR_ENABLE;



     //SECOND 8k Region
     //Base address 0x20006000 to 20007FFF = 8192 Bytes
     // N = log_2(8192) = 13
     NVIC_MPU_NUMBER_R = 4;
     NVIC_MPU_BASE_R = 0x20006000;
     //                |-x               |AP BITS                                        |13 - 1 = 12
     NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_XN|(3 << 24)|(6 << 16)|(0xFF << 8)| (12 << 1)| NVIC_MPU_ATTR_ENABLE;

}

uint64_t createNoSramAccessMask(void)
{
    return 0xFFFFFFFFFFFFFFFF;
}




uint16_t calculateLowerIndex_4k(uint32_t *baseAdd)
{
    uint32_t baseValue = (uint32_t)baseAdd;

    // Shift the base address to get the relevant bits
    baseValue = (baseValue >> 20);

    // Calculate the lowerIndex based on the baseValue
    if (baseValue == 0)
    {
        return 0;
    }
    else
    {
        return baseValue / 512;
    }
}

uint16_t calculateLowerIndex_8k(uint32_t *baseAdd)
{
    //uint32_t baseValue = (uint32_t)baseAdd;

    // Shift the base address to get the relevant bits
    //baseValue = (baseValue >> 20);
    uint32_t baseValue = ((uint32_t)baseAdd)&0xFFF;
    // Calculate the lowerIndex based on the baseValue
    if (baseValue == 0)
    {
        return 0;
    }
    else
    {
        return baseValue / 1024;
    }
}


uint32_t indexImproved(uint32_t *baseAdd, uint32_t startingRange,uint32_t size_in_bytes, uint16_t index)
{

    uint16_t i = 0;
    uint32_t found = 100, sum = 0;

    sum = startingRange;
    uint32_t add = (uint32_t)baseAdd;

    for(i = index;i < 40; i++)
    {
        if(add == sum)
        {
            found = i;
            break;
        }
        sum+= size_in_bytes;
    }

    return found;
}


void addSramAccessWindowImproved(uint64_t *srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes)
{
    uint16_t lowerIndex = 0, upperIndex = 0, i = 0;


    if(baseAdd >= (uint32_t*)(0x20001000) && baseAdd <= (uint32_t*)0x20003E00)
    {


        lowerIndex = indexImproved(baseAdd,0x20001000,size_in_bytes,0);
        upperIndex = lowerIndex + 1;

        for(i = lowerIndex; i < upperIndex; i++){
            *srdBitMask &= ~(1ULL << i);
        }

    }
    else if(baseAdd >= (uint32_t*)(0x20004000) && baseAdd <= (uint32_t*)0x20007C00)
    {

        uint32_t subRegions = 0;

        subRegions = size_in_bytes % 1024;

        if(subRegions != 0)
        {
            subRegions = ((uint32_t)(size_in_bytes / 1024)) + 1;
        }else
        {
            subRegions = size_in_bytes / 1024;
        }

        lowerIndex = indexImproved(baseAdd, 0x20004000,1024,24);

        upperIndex = lowerIndex + subRegions;

        for(i = lowerIndex; i < upperIndex; i++){
            *srdBitMask &= ~(1ULL << i);
        }

    }


    if(size_in_bytes == 28672)
    {
        *srdBitMask = 0xFFFFFF0000000000;
    }
}







uint32_t index(uint32_t *baseAdd, uint32_t startingRange, uint32_t size_in_bytes)
{


    uint16_t i = 0;
    uint32_t found = 100, sum = 0;
    sum = startingRange;
    uint32_t add = (uint32_t)baseAdd;
    for(i = 0;i < 8; i++)
    {
        if(add == sum)
        {
                   found = i;
                   break;
               }
                sum+= size_in_bytes;
    }


    return found;

}


void addSramAccessWindow(uint64_t *srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes)
{
    uint16_t lowerIndex = 0, upperIndex = 0,i = 0;
   // uint32_t sum = 0, found = 0;

    if(baseAdd >= (uint32_t*)(0x20001000) && baseAdd <= (uint32_t*)0x20001E00)
    {
//        sum = ((uint32_t)0x20001000);
//        uint32_t add = (uint32_t)baseAdd;
//        for(i = 0;i < 8; i++)
//        {
//
//            if(add == sum){
//                found = i;
//                break;
//            }
//             sum+= size_in_bytes;
//        }

        //SRD bits 0 to 7

        //lowerIndex = calculateLowerIndex_4k(baseAdd);
        lowerIndex = index(baseAdd,0x20001000,size_in_bytes);
        upperIndex = lowerIndex + 1;
        //lowerIndex = lowerIndex - 1;
        for(i = lowerIndex; i < upperIndex; i++){
            *srdBitMask &= ~(1ULL << i);
        }

    }else if(baseAdd >= (uint32_t*)(0x20002000) && baseAdd <= (uint32_t*)0x20002E00)
    {
        //SRD bits 8 to 15
//        lowerIndex = 8 + calculateLowerIndex_4k(baseAdd);
//        upperIndex = lowerIndex + (size_in_bytes / 512);
//        lowerIndex = lowerIndex -1;
        lowerIndex = 8 + index(baseAdd,0x20002000,size_in_bytes);
        upperIndex = lowerIndex + 1;
        for(i = lowerIndex; i < upperIndex; i++){
            *srdBitMask &= ~(1ULL << i);
        }



    }else if(baseAdd >= (uint32_t*)(0x20003000) && baseAdd <= (uint32_t*)0x20003E00)
    {
        //SRD bits 16 to 23
        //lowerIndex = 16 + calculateLowerIndex_4k(baseAdd);
        lowerIndex = 16 + index(baseAdd,0x20003000,size_in_bytes);
        //upperIndex = lowerIndex + (size_in_bytes / 512);
        upperIndex = lowerIndex + 1;
        //lowerIndex = lowerIndex -1;
        for(i = lowerIndex; i < upperIndex; i++){
            *srdBitMask &= ~(1ULL << i);
        }



    }else if(baseAdd >= (uint32_t*)(0x20004000) && baseAdd <= (uint32_t*)0x20005C00)
    {
        //SRD bits 24 to 31
        //uint32_t offset = 0;


        uint32_t subRegions = 0;
        subRegions = size_in_bytes % 1024;

        if(subRegions != 0)
        {
            subRegions = ((uint32_t)(size_in_bytes / 1024)) + 1;
        }else
        {
            subRegions = size_in_bytes / 1024;
        }

        lowerIndex = 24 + index(baseAdd, 0x20004000,size_in_bytes);

        upperIndex = lowerIndex + subRegions;
        //upperIndex = lowerIndex + (size_in_bytes / 1024);

        //lowerIndex = lowerIndex;
        for(i = lowerIndex; i < upperIndex; i++){
            *srdBitMask &= ~(1ULL << i);
        }



    }else if(baseAdd >= (uint32_t*)(0x20006000) && baseAdd <= (uint32_t*)0x20007C00)
    {
        //SRD bits 32 to 39
        lowerIndex = 32 + calculateLowerIndex_8k(baseAdd);
        upperIndex = lowerIndex + (size_in_bytes / 1024);
        lowerIndex = lowerIndex -1;
        for(i = lowerIndex; i < upperIndex; i++){
            *srdBitMask &= ~(1ULL << i);
        }
    }

    if(size_in_bytes == 28672)
    {
        *srdBitMask = 0xFFFFFF0000000000;
    }
}

void applySramAccessMask(uint64_t srdBitMask)
{
    uint8_t i = 0;
    uint32_t valueBits = 0;
    for(i = 0; i < 5; i++){     //
        NVIC_MPU_NUMBER_R = i;
        valueBits = (srdBitMask >>(i*8)) & 0xFF;

        NVIC_MPU_ATTR_R &= 0xFFFF00FF;
        NVIC_MPU_ATTR_R |= (valueBits << 8);

    }
}

uint64_t srdMask(uint32_t *baseAdd,uint32_t stackBytes)
{
    uint64_t srdBitMask = createNoSramAccessMask();
    addSramAccessWindow(&srdBitMask, baseAdd, stackBytes);
    return srdBitMask;
}

void setSramAccessWindow(uint32_t *baseAdd,uint32_t size_in_bytes)
{

    uint64_t srdBitMask = createNoSramAccessMask();
    addSramAccessWindow(&srdBitMask, baseAdd, size_in_bytes);
    applySramAccessMask(srdBitMask);

}


