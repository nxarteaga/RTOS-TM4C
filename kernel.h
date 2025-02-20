// Kernel functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef KERNEL_H_
#define KERNEL_H_

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
//#include "shell.h"

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// function pointer
typedef void (*_fn)();

// mutex
#define MAX_MUTEXES 1
#define MAX_MUTEX_QUEUE_SIZE 2
#define resource 0

// semaphore
#define MAX_SEMAPHORES 3
#define MAX_SEMAPHORE_QUEUE_SIZE 2
#define keyPressed 0
#define keyReleased 1
#define flashReq 2

// tasks
#define MAX_TASKS 12




typedef struct _memInfo
{
    uint32_t pid;                     // ***USE address of function.used to uniquely identify thread (add of task fn)
    char name[16];                 // name of task used in ps command
    uint32_t stackBytes;
    void *base;
}memoryInfo;  //

//memoryInfo allocationInfo[12];
typedef struct _ipcsMutex
{
    uint8_t queuesizeMutex;
    char name[16];
    //uint8_t processQueueMutex[MAX_MUTEX_QUEUE_SIZE];
    uint8_t mutexLockedBy;
    char waiting[15];
   // bool mutexLock;
}ipcsMutex_;

typedef struct _ipcsSemaphore
{
    uint8_t countSemaphore;
    uint8_t queueSizeSemaphore;
    char queueName[15];
//    char name2[10];
//    char name3[10];
    //uint8_t processQueue[MAX_SEMAPHORE_QUEUE_SIZE];
   // bool mutexLock;
}ipcsSemaphore_;


typedef struct _ps
{

    uint32_t pid;
    char name[16];
    uint32_t cpuTime;
    uint32_t state;
    uint8_t mutex;
    uint8_t semaphore;

}ps_;



//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex); // Already written
bool initSemaphore(uint8_t semaphore, uint8_t count);// Already written

void initRtos(void);// Requires writing

void *getPid(void);

void startRtos(void);//Requires writing

bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes);
void restartThread(_fn fn);
void stopThread(_fn fn);
void setThreadPriority(_fn fn, uint8_t priority);

void yield(void);
void sleep(uint32_t tick);
void lock(int8_t mutex);
void unlock(int8_t mutex);
void wait(int8_t semaphore);
void post(int8_t semaphore);
void * mallocFromHeapWrapper(uint32_t stackBytes);
void _sched(bool pri_on);
void _preempt(bool pri_on);
void _memoryInfo(memoryInfo *allocationInfo);
void _ipcsCommand(ipcsMutex_ *ipcs_mutx, ipcsSemaphore_ *ipcs_sem);
void _psInfo(ps_ *ps_cpu);
void _reboot(void);

uint32_t _pidof(const char schedulerType[]);
uint32_t getMallocPID();

void systickIsr(void);
void pendSvIsr(void); //place holder code. Already have.
void svCallIsr(void);


void killThread(uint32_t pid);

#endif
