// Kernel functions
// J Losh
//Modified by Nestor Arteaga

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
#include "uart0.h"
#include "mm.h"
#include "asm.h"
#include "custom_str_library.h"
#include "kernel.h"
#include "shell.h"

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// mutex
typedef struct _mutex
{
    bool lock;
    uint8_t queueSize; // How many process waiting
    uint8_t processQueue[MAX_MUTEX_QUEUE_SIZE]; // processQueue[2]
    uint8_t lockedBy;// can only be unlocked by same pid
} mutex;
mutex mutexes[MAX_MUTEXES];

// semaphore
typedef struct _semaphore
{
    uint8_t count;// current count of semaphore. we have 20
    uint8_t queueSize;
    uint8_t processQueue[MAX_SEMAPHORE_QUEUE_SIZE];
} semaphore;
semaphore semaphores[MAX_SEMAPHORES];

//Every entry in the TCB is invalid at the beginning.
//STATE_STOPPED someone called stopped thread.
//STATE_READY
// task states
#define STATE_INVALID           0 // no task
#define STATE_STOPPED           1 // stopped, all memory freed
#define STATE_READY             2 // can start or resume at any time
#define STATE_DELAYED           3 // has run, but now awaiting timer.
#define STATE_BLOCKED_MUTEX     4 // has run, but now blocked by semaphore
#define STATE_BLOCKED_SEMAPHORE 5 // has run, but now blocked by semaphore. ***

//SVC calls
#define START_RTOS  0
#define PENDS_SV    1
#define SLEEP       2
#define LOCK        3
#define UNLOCK      4
#define POST        6
#define WAIT        8
#define MALLOC      10
#define SCHED       11
#define PIDOF       12
#define PREEMPT     13
#define RESTART_THREAD 20
#define STOP_THREAD 30
#define MEM_INFO    35
#define IPCS        37
#define PS          39
#define REBOOT      41
#define THREAD_PRIO 43
// task
uint8_t taskCurrent = 0;          // index of last dispatched task.  ***Same as process Queue.
uint8_t taskCount = 0;            // total number of valid tasks in the TCB.

uint32_t sysTicks = 0;
//uint32_t elapsedTime = 0;



bool writeA = true;    //control flag for PING PONG Buffer
bool writeB = false;    //control flag for PING PONG Buffer
// control
bool priorityScheduler = true;    // priority (true) or round-robin (false)
bool priorityInheritance = false; // priority inheritance for mutexes
bool preemption = true;          // preemption (true) or cooperative (false)
//TRUE when I turn in project.


//PID for malloc
volatile uint32_t mallocPID = 0; //helps with Malloc allocation ownership. Used for second allocation of Lengthy


void scheduler(void)
{
    __asm("    SVC #0");
}

// tcb
#define NUM_PRIORITIES   16
struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    void *pid;                     // ***USE address of function.used to uniquely identify thread (add of task fn)
    void *spInit;                  // original top of stack.
    void *sp;                      // current stack pointer   ..****
    uint8_t priority;              // 0=highest
    //uint8_t currentPriority;       // 0=highest (needed for pi)
    uint32_t stackBytes;           //Used for restoring threads. RESTART_THREAD with same allocation
    uint32_t bufferA;              //PING PONG buffers
    uint32_t bufferB;
    uint32_t startTime;
    void *base;
    uint32_t ticks;                // ticks until sleep complete. ****SysTick ISR -- This is # of milisecs until sleep command goes to zero.
    uint64_t srd;                  // MPU subregion disable bits.
    char name[16];                 // name of task used in ps command
    uint8_t mutex;                 // index of the mutex in use or blocking the thread
    uint8_t semaphore;             // index of the semaphore that is blocking the thread
} tcb[MAX_TASKS];  //8 bit state variable. Invalid, values above.

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex)
{
    bool ok = (mutex < MAX_MUTEXES);
    if (ok)
    {
        mutexes[mutex].lock = false;
        mutexes[mutex].lockedBy = 0;
    }
    return ok;
}

bool initSemaphore(uint8_t semaphore, uint8_t count)
{
    bool ok = (semaphore < MAX_SEMAPHORES);
    {
        semaphores[semaphore].count = count;
    }
    return ok;
}

// Done: initialize systick for 1ms system timer
void initRtos(void)
{
    uint8_t i;
    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }
    NVIC_ST_RELOAD_R = 39999; // (40Mhz / 1khz) - 1
                          // clock source        enable int           enable systick
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_INTEN | NVIC_ST_CTRL_ENABLE;
}


// REQUIRED: Implement prioritization to NUM_PRIORITIES
//uint8_t rtosScheduler(void)
//{
//    bool ok;
//    static uint8_t task = 0xFF;
//    ok = false;
//    while (!ok)
//    {
//        task++;
//        if (task >= MAX_TASKS)
//            task = 0;
//        ok = (tcb[task].state == STATE_READY);
//    }
//    return task;
//}

// DONE: Implement prioritization to NUM_PRIORITIES
uint8_t rtosScheduler(void)
{
        uint8_t  level = 0;
//        bool ok;
       // static uint8_t task = 0xFF;  // Currently selected task
        static uint8_t lastTask = 0xFF;  // Last task that ran
        uint8_t highPriority = 0, i = 0;
        uint8_t schedule[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};  // Array to hold ready tasks
        bool found = false, schedulerReady = false;
    //uint8_t j = 0;
    bool ok;
    static uint8_t task = 0xFF;
    ok = false;
    while (!ok)
    {
        if(priorityScheduler == true)
        {
            while (!found) {

                level = 0;  // Reset the schedule array for the current priority level

                ///ADDD ANOTHER
                for(highPriority = 0;highPriority < NUM_PRIORITIES;highPriority++ ){
                    for (i = 0; i < 10; i++) {
                        if (tcb[i].priority == highPriority && tcb[i].state == STATE_READY) {
                            schedule[level++] = i;
                            found = true;   //Find highest priority
                            break;
                        }

                    }
                    if(found)
                        break;
                }

                // If no tasks are found at this priority level, increase priority
//                if (!found) {
//                    highPriority++;
//                }

                if(found){
                    level = 0;
                    // Perform round-robin
                    for (i = 0; i < MAX_TASKS; i++) {
                        if (tcb[i].priority == highPriority && tcb[i].state == STATE_READY) {

                            schedule[level++] = i;

                        }
                    }

                    schedulerReady = true;
                }


                if(schedulerReady == true)
                {
                    for (i = 0; i < level; i++) {
                        if (schedule[i] == lastTask) {
                            // Select the next task in round-robin
                            task = schedule[(i + 1) % level];
                            break;
                        }
                    }



                    // If lastTask was not in the schedule, pick the first task
                    if (task == lastTask || task == 0xFF) {
                        task = schedule[0];
                    }

                    // Update lastTask and exit loop
                    lastTask = task;
                    ok = true;
                }

            }
        }else{
            task++;         //ROUND ROBIN
            if (task >= MAX_TASKS)
                task = 0;
            ok = (tcb[task].state == STATE_READY);
        }



    }
    return task;
}










void *getPid(void)  //Getter function for MALLOC ownership
{

    return tcb[taskCurrent].pid;
}








// REQUIRED: modify this function to start the operating system
// by calling scheduler, set srd bits, setting PSP, ASP bit, call fn with fn add in R0
// fn set TMPL bit, and PC <= fn
void startRtos(void)
{

    setPsp(tcb[taskCurrent].sp);
    //setPsp((uint32_t*)0x20004000);
    setAspBit();

    //Debug Ensure stack_ptr in scheduler is the right ptr

   // scheduler();
    __asm("    SVC #0");


    //NEEDS to call fn with fn add in R?????


    //NEEDS to  PC <= fn????????????


    //set Sram Access window has the bitMask already
   // applySramAccessMask();

    //SECOND half of PendSv code.
}


uint32_t getMallocPID()
{
    return mallocPID;
}

// DONE:
// add task if room in task list -- Done
// store the thread name        --Done
// allocate stack space and store top of stack in sp --Done and spInit --spInit Done
// set the srd bits based on the memory allocation
// initialize the created stack to make it appear the thread has run before

//_fn will be a function address
bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    bool ok = false;
    uint8_t i = 0;
    bool found = false;


    if (taskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent reentrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }
        if (!found)
        {
            // find first available tcb record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}
            tcb[i].state = STATE_READY;
            tcb[i].pid = fn;
            mallocPID = (uint32_t)fn;

            //spInit is originaltop of stack; a void *. use malloc to allocate to ptr
            //tcb[i].spInit = (void*)getPSP();


            uint32_t *ptr = mallocFromHeap(stackBytes);

            tcb[i].base = ptr; //use it for freeToHeap
            tcb[i].stackBytes = stackBytes;

            tcb[i].spInit =(void*)((uint32_t)ptr + stackBytes);

            //tcb[i].spInit =(void*)((uint32_t)mallocFromHeap(stackBytes)+(stackBytes));
           // ptr = mallocFromHeap(stackBytes);

            tcb[i].sp = tcb[i].spInit;

            tcb[i].priority = priority;

            //setSramAccessWindow();

            //tcb[i].srd = srdMask(ptr,stackBytes);

           // tcb[1].srd = 0xFFFFFFFFFFFFFFFD;
            uint64_t srdBitMask = createNoSramAccessMask();
            addSramAccessWindowImproved(&srdBitMask, ptr, stackBytes);
            tcb[i].srd = srdBitMask;

            myStrCpy((char*)name,tcb[i].name);

            uint32_t *stack_ptr = tcb[i].sp;

            *(--stack_ptr) = 0x01000000;             // xPSR
            *(--stack_ptr) = (uint32_t)tcb[i].pid;   //   PC
            *(--stack_ptr) = (uint32_t)0x1234;  //   LR
            *(--stack_ptr) = 12;  //R12
            *(--stack_ptr) = 33;  //R3
            *(--stack_ptr) = 22;  //R2
            *(--stack_ptr) = 11;  //R1
            *(--stack_ptr) = 101010;  //R0


            *(--stack_ptr) = 4444444;  //R4
            *(--stack_ptr) = 10;  //R5
            *(--stack_ptr) = 99;  //R6
            *(--stack_ptr) = 88;  //R7
            *(--stack_ptr) = 77;  //R8
            *(--stack_ptr) = 66;  //R9
            *(--stack_ptr) = 55;  //R10
            *(--stack_ptr) = 44;  //R11
            *(--stack_ptr) = (uint32_t)0xFFFFFFFD;
            tcb[i].sp = (void*)stack_ptr;


            // increment task count
            taskCount++;
            ok = true;
        }
    }
    return ok;
}

// DONE: modify this function to restart a thread
void restartThread(_fn fn)
{
    __asm("    SVC #20");
}

// DONE: modify this function to stop a thread
// REQUIRED: remove any pending semaphore waiting, unlock any mutexes
void stopThread(_fn fn)// by name
{
    __asm("    SVC #30");

}

// DONE: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority)
{
    __asm("    SVC #43");
}

// DONE: modify this function to yield execution back to scheduler using pendsv
void yield(void)
{
    __asm("    SVC #1");
}

// DONE: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
void sleep(uint32_t tick)
{
    __asm("    SVC #2");
}

// DONE: modify this function to lock a mutex using pendsv
void lock(int8_t mutex)
{
    __asm("    SVC #3");
}

// DONE: modify this function to unlock a mutex using pendsv
void unlock(int8_t mutex)
{
    __asm("    SVC #4");
}

// DONE: modify this function to wait a semaphore using pendsv
void wait(int8_t semaphore)
{
    __asm("    SVC #8");
}

// DONE: for lengthyFunction
void post(int8_t semaphore)
{
    __asm("    SVC #6"); //Needs to update SRD bits
}

// DONE: Malloc wrapper needed for  Lenghty Function
void * mallocFromHeapWrapper(uint32_t stackBytes)
{
    __asm("    SVC #10");

}




//SHELL COMMANDS

void _sched(bool pri_on) //Shell Command for type of scheduler
{
    __asm("    SVC #11");
}

uint32_t _pidof(const char schedulerType[]) //Gets PID of a Proc_name
{
    __asm("    SVC #12");
}





void killThread(uint32_t pid) //Used for kill pid and Pkill proc_name
{

    //search for base address in tcb
    uint8_t i = 0;
    //uint32_t stack_bytes = 0;
    void * ptr;


    for(i = 0; i < MAX_TASKS; i++) //Return base address of task. Base address will be passed to FreeHeap
    {

        if((void *)pid == tcb[i].pid)
        {

            ptr = tcb[i].base; //
            break;

        }

    }


   // addSramAccessWindowImproved(&tcb[taskCurrent].srd, ptr, tcb[taskCurrent].stackBytes);

   // applySramAccessMask(tcb[taskToKill].srd);

    //Blocked by mutex?
    //blocked by semaphore?
    //locked by ?
    uint16_t taskToKill = i;

    if(tcb[taskToKill].state != STATE_STOPPED){

            uint64_t srdMask = createNoSramAccessMask();

            freeToHeapImproved(ptr);

            tcb[taskToKill].srd =  srdMask;


            if(tcb[taskToKill].state == STATE_BLOCKED_MUTEX)
            {

                uint8_t mutex = tcb[taskToKill].mutex; //Index of the mutex in use or blocking the thread
                uint8_t j = 0;

                for(j = 0; j < (MAX_MUTEX_QUEUE_SIZE - 1); j++)
                {
                    mutexes[mutex].processQueue[j] = mutexes[mutex].processQueue[j + 1];
                }

                mutexes[mutex].queueSize--;

            }



            if(tcb[taskToKill].state == STATE_BLOCKED_SEMAPHORE)
            {
                //semaphores[taskToKill].count++;////DEBUGGGGGG


                if(semaphores[taskToKill].queueSize > 0)
                    {
                        int8_t i = 0;
                        tcb[semaphores[taskToKill].processQueue[i]].state = STATE_READY;

                        // semaphores[semaphoreResource].queueSize--;


                        for(i = 0;i < (MAX_SEMAPHORE_QUEUE_SIZE - 1);i++)
                        {
                            semaphores[taskToKill].processQueue[i] = semaphores[taskToKill].processQueue[i + 1];
                        }

                       // semaphores[taskToKill].count--;
                        semaphores[taskToKill].queueSize--;
                    }

            }






            if(mutexes[0].lockedBy == taskToKill){//Should next process lock


                ///DEBUG!!! NOT
                uint8_t mutex = tcb[taskToKill].mutex; //Index of the mutex in use or blocking the thread
                mutexes[mutex].lock = false;

                //if someome is waiting
                if(mutexes[mutex].queueSize > 0)
                {

                    uint8_t nextTask = mutexes[mutex].processQueue[0];
                    tcb[nextTask].state = STATE_READY;

                    for(i = 0; i < (MAX_MUTEX_QUEUE_SIZE - 1); i++)
                    {

                        mutexes[mutex].processQueue[i] = mutexes[mutex].processQueue[i+1];

                    }


                    //Zero out queue position that was moved?
                    //mutexes[mutex].processQueue[MAX_MUTEX_QUEUE_SIZE - 1] = 0;
                    mutexes[mutex].lock = true;
                    mutexes[mutex].lockedBy = nextTask;

                    mutexes[0].queueSize--;

                }


        }



            //Maybe increase count by

        taskCount--;
        tcb[i].state = STATE_STOPPED;

    }
}

void restart_Thread(uint32_t task)
{





    mallocPID = (uint32_t)tcb[task].pid;

    uint32_t stackBytes = tcb[task].stackBytes;

    void *ptr;

    ptr = mallocFromHeap(stackBytes);

    tcb[task].base = ptr;


    //tcb[task].semaphore = 1;


    tcb[task].spInit = (void*)((uint32_t)ptr + stackBytes);

    tcb[task].sp = tcb[task].spInit;

    uint64_t srdBitMask = createNoSramAccessMask();
    addSramAccessWindowImproved(&srdBitMask, ptr, stackBytes);
    tcb[task].srd = srdBitMask;




    //applySramAccessMask(tcb[tcb].srd);
    tcb[task].state = STATE_READY;


    uint32_t *stack_ptr = tcb[task].sp;

               *(--stack_ptr) = 0x01000000;             // xPSR
               *(--stack_ptr) = (uint32_t)tcb[task].pid;   //   PC
               *(--stack_ptr) = (uint32_t)0x1234;  //   LR
               *(--stack_ptr) = 12;  //R12
               *(--stack_ptr) = 33;  //R3
               *(--stack_ptr) = 22;  //R2
               *(--stack_ptr) = 11;  //R1
               *(--stack_ptr) = 101010;  //R0


               *(--stack_ptr) = 4444444;  //R4
               *(--stack_ptr) = 10;  //R5
               *(--stack_ptr) = 99;  //R6
               *(--stack_ptr) = 88;  //R7
               *(--stack_ptr) = 77;  //R8
               *(--stack_ptr) = 66;  //R9
               *(--stack_ptr) = 55;  //R10
               *(--stack_ptr) = 44;  //R11
               *(--stack_ptr) = (uint32_t)0xFFFFFFFD;
               tcb[task].sp = (void*)stack_ptr;

    taskCount++;
}



//
//uint32_t stopThread(const char schedulerType[])
//{
//    __asm("    SVC #3");
//}


// Restart Thread
//void * mallocFromHeapWrapper(uint32_t stackBytes)
//{
//    __asm("    SVC #10");
//
//}

//const char schedulerType[]
void _preempt(bool pri_on)
{
    __asm("    SVC #13");
}

void _memoryInfo(memoryInfo *_memInfo)
{
    __asm("    SVC #35");
}


void _ipcsCommand(ipcsMutex_ *ipcs_mutx, ipcsSemaphore_ *ipcs_sem)
{
    __asm("    SVC #37");
}


void _psInfo(ps_ *ps_cpu)
{
    __asm("    SVC #39");
}


void _reboot(void)
{
    __asm("    SVC #41");
}


// Done: modify this function to add support for the system timer
// Done: in preemptive code, add code to request task switch
void systickIsr(void)
{


    sysTicks++;

    uint8_t j = 0;

    if (sysTicks % 1000 == 0) { // Check if a second has elapsed
        sysTicks = 0; // Reset sysTicks for the next second

        if (writeA) {
            // Reset bufferA for all tasks
            for (j = 0; j < taskCount; j++) {
                tcb[j].bufferA = 0;
                tcb[j].startTime = 0;
            }

            // Switch to writing to bufferB
            writeA = false;
            writeB = true;
        } else if (writeB) {
//            // Reset bufferB for all tasks
            for (j = 0; j < taskCount; j++) {
                tcb[j].bufferB = 0;
                tcb[j].startTime = 0;
            }

            // Switch to writing to bufferA
            writeA = true;
            writeB = false;
        }
    }

//
//    if((sysTicks % 1000) == 0)
//    {
//       // tcb[taskCurrent].bufferA = 0;
////        systicks = 0;
//        writeB = true;
//        writeA = false;
//    }else{
////        tcb[taskCurrent].bufferB = 0;
//        writeA = true;
//        writeB = false;
//    }
//
//    if((sysTicks % 1000) == 0)  //zero out the buffer you are about to write to.
//    {
//
//        sysTicks = 1000;
//        for(j = 0; j < taskCount;j++)
//        {
//            tcb[j].bufferB = 0;
//        }
//
//    }else{
//
//        for(j = 0; j < taskCount;j++)
//        {
//            tcb[j].bufferA = 0;  //Invert
//        }
//    }


//    if(writeA == true)
//    {
//
//
//    }else{
//
//
//    }





    uint8_t i;
    for (i = 0; i < taskCount; i++) //taskCount gets updated in createThread()
    {
        if (tcb[i].state == STATE_DELAYED)
        {
            tcb[i].ticks--; // if STATE of task is Delayed then decrement ticks.
                            //when ticks are zero, the task is ready to execute.
            if (tcb[i].ticks == 0)
                tcb[i].state = STATE_READY;
            }
    }







    //Everyone will get the same time slice
    if(preemption == true){
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
    }

}

// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
//__attribute__((naked)) void pendSvIsr(void)
//void pendSvIsr(void)
__attribute__((naked)) void pendSvIsr(void)
{
    __asm("    MOV R12, LR");
    uint32_t reg = NVIC_FAULT_STAT_R;
    //putsUart0(" Pendsv in process N.\n");


    if((reg & NVIC_FAULT_STAT_DERR) || (reg & NVIC_FAULT_STAT_IERR)){
        NVIC_FAULT_STAT_R |= (1 << 1) & (1 << 0);
        putsUart0(" Called from MPU\n");
    }

    //Save registers
    push_R4_R11();

    tcb[taskCurrent].sp = (void*)getPsp();


    //tcb[taskCurrent].elapsedTime += switchBuffer - tcb[taskCurrent].startTime;

    //save in here for Buffer A or B depending on ready state


    if(writeA == true)
    {
        tcb[taskCurrent].bufferA += sysTicks - tcb[taskCurrent].startTime;
        //tcb[taskCurrent].bufferB = 0;

    }else{

       tcb[taskCurrent].bufferB += sysTicks - tcb[taskCurrent].startTime;
       //tcb[taskCurrent].bufferA = 0;
    }

    taskCurrent = rtosScheduler();

    tcb[taskCurrent].startTime = sysTicks;



    setPsp(tcb[taskCurrent].sp);

    applySramAccessMask(tcb[taskCurrent].srd);

    pop_R4_R11();
}


//void getMemInfo()
//{
//
//
//
//}


// REQUIRED: modify this function to add support for the service call
// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr(void)
{

    uint32_t svcCallNumber = getSVC();
    //uint32_t svcCallNumber = (uint32_t)(getPsp()+6);
    switch(svcCallNumber)
    {
        case START_RTOS: //Start RTOS
        {
           // setAspBit();
             //set TMPL
            setUnprivileged();

            taskCurrent = rtosScheduler();
             applySramAccessMask(tcb[taskCurrent].srd);
             //RESTORE PSP? setPSP to tcb[taskCurrent].sp?
             setPsp(tcb[taskCurrent].sp);

             //RESTORE(POP) REGS R4 - R11
             //make BX LR -> 0xFFFFFFFD
            // uint32_t *stack_ptr = getPsp();

             pop_R4_R11();
             break;

        }
        case PENDS_SV://PendSv
        {
            NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
            break;
        }
        case SLEEP://Sleep
        {
            uint32_t ticks = *getPsp();
            tcb[taskCurrent].state = STATE_DELAYED;
            tcb[taskCurrent].ticks = ticks;

            NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
            break;
        }
        case LOCK://Lock
        {

            uint8_t resourceMutex = *getPsp();

            if(mutexes[resourceMutex].lock == false){

                mutexes[resourceMutex].lock = true;
                mutexes[resourceMutex].lockedBy = taskCurrent;
                tcb[resourceMutex].mutex = resourceMutex;



            }else if(mutexes[resourceMutex].lockedBy != taskCurrent){

                //if(mutexes[resourceMutex].lockedBy != taskCurrent){
                    //mutex queue add your id. taskCurrent is the id of the running task
                    //it has to be added to the process queue
                    mutexes[resource].processQueue[mutexes[resource].queueSize] = taskCurrent;
                    //


                    mutexes[resourceMutex].queueSize++; //increment

                    tcb[taskCurrent].state = STATE_BLOCKED_MUTEX;

                    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;

               // }
            }
            break;
        }
        case UNLOCK://unlock
        {
            uint8_t resourceMutex = *getPsp();
            if(mutexes[resourceMutex].lockedBy == taskCurrent)
            {
                mutexes[resourceMutex].lock = false;

                if(mutexes[resourceMutex].queueSize > 0)
                {

                    uint8_t i = 0;
                    //Make next  process in queue ready
                    //make next process(task) ready(STATE_READY). This will be in process processQueue
                    uint8_t nextTask = mutexes[resourceMutex].processQueue[0];
                    tcb[nextTask].state = STATE_READY;

                    //move users in queue?
                    //MAX_MUTEX_QUEUE_SIZE is defined as 2.
                    //After making the next process ready, The queue has to be updated.


                    for(i = 0; i < (MAX_MUTEX_QUEUE_SIZE - 1); i++)
                    {

                        mutexes[resourceMutex].processQueue[i] = mutexes[resourceMutex].processQueue[i+1];

                    }
                    //Zero out queue position that was moved?
                    //mutexes[mutex].processQueue[MAX_MUTEX_QUEUE_SIZE - 1] = 0;
                    mutexes[resourceMutex].lock = true;
                    mutexes[resourceMutex].lockedBy = nextTask;

                    //decrement queue count
                    mutexes[resourceMutex].queueSize--;

                }


            }
            //else
//            {
//                //KILL
//
//            }



            break;
        }
        case POST://
        {
            uint32_t semaphoreResource = *getPsp();

            semaphores[semaphoreResource].count++;

            if(semaphores[semaphoreResource].queueSize > 0)
            {
                int8_t i = 0;
                tcb[semaphores[semaphoreResource].processQueue[i]].state = STATE_READY;

               // semaphores[semaphoreResource].queueSize--;


                for(i = 0;i < (MAX_SEMAPHORE_QUEUE_SIZE - 1);i++)
                {
                    semaphores[semaphoreResource].processQueue[i] = semaphores[semaphoreResource].processQueue[i + 1];
                }

                semaphores[semaphoreResource].count--;
                semaphores[semaphoreResource].queueSize--;
            }

            break;
        }
        case WAIT://post is the same as unlock
        {
            uint32_t semaphoreResource = *getPsp();

            if(semaphores[semaphoreResource].count > 0)
            {
                semaphores[semaphoreResource].count--;
            }
            else
            {
                semaphores[semaphoreResource].processQueue[semaphores[semaphoreResource].queueSize] = taskCurrent;

                semaphores[semaphoreResource].queueSize++;

                tcb[taskCurrent].state = STATE_BLOCKED_SEMAPHORE;

                tcb[taskCurrent].semaphore = semaphoreResource;


                //context switch
                NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;

            }

            break;
        }
        case MALLOC:
        {
            uint32_t size_in_bytes = *getPsp();
            mallocPID = (uint32_t)tcb[taskCurrent].pid;
            void * ptr = mallocFromHeap(size_in_bytes);

            uint64_t srdBitMask = createNoSramAccessMask();
            addSramAccessWindowImproved(&tcb[taskCurrent].srd, ptr, size_in_bytes);
            //tcb[taskCurrent].srd = srdBitMask | 0xFFFFFFFF;
            applySramAccessMask(tcb[taskCurrent].srd);
            *getPsp() = (uint32_t)ptr;

            break;
        }
        case SCHED:
        {
            bool prio;
            prio = *getPsp();
            priorityScheduler = prio;




          break;
        }case PREEMPT: //add preemption code to systick
        {
            bool preempt;
            preempt = *getPsp();
            //priorityScheduler = prio;
            preemption = preempt;


          break;
        }case PIDOF: //add preemption code to systick
        {
            uint32_t i = 0, result = 1000, pid =1000;
            char *proc_name;
            proc_name =(char*)*getPsp();

            for(i = 0; i < MAX_TASKS; i++)
            {
                result = my_strcmp(proc_name, tcb[i].name);
                if(result == 0)
                {
                    pid = (uint32_t)tcb[i].pid;
                    break;
                }
            }

            *getPsp() = (uint32_t)pid;



            //priorityScheduler = prio;
            //preemption = preempt;


          break;
        }
        case RESTART_THREAD:
        {
            //Similar to Create Thread
            //puts state back to READY

            uint32_t i = 0, result = 1000, task = 255, pid = 0;
            char *proc_name;

            proc_name =(char*)*getPsp();
            pid = *getPsp();
            bool found = false;

            for(i = 0; i < MAX_TASKS; i++)
            {
                result = my_strcmp(proc_name, tcb[i].name);
                if((result == 0) || (pid == (uint32_t)tcb[i].pid))
                {
                    //pid = (uint32_t)tcb[i].pid;
                    //should get task #
                    task = i;
                    found = true;
                    break;
                }
            }

            if(found && tcb[i].state == STATE_STOPPED){
                restart_Thread(task);
            }
            else
            {
                putsUart0("Invalid function\n");
            }
//            else{         //If it doesn't exist let the user know.
//
//                *getPsp() = (uint32_t)pid;
//            }
//

            break;
        }
        case STOP_THREAD:
        {
            uint32_t pid = 0, i = 0, task = 255, result = 255;
            pid = (uint32_t)*getPsp();
            char *proc_name;
            proc_name =(char*)*getPsp();
            bool found = false;

            uint32_t errorNotFound = 255;

            for(i = 0; i < MAX_TASKS; i++)
            {
                result = my_strcmp(proc_name, tcb[i].name);
                if(result == 0)
                {
                    //pid = (uint32_t)tcb[i].pid;
                    //should get task #
                    //task = i;
                    pid = (uint32_t)tcb[i].pid;
                    task = i;
                    found = true;
                    break;
                }else if(pid == (uint32_t)tcb[i].pid)
                {
                    pid = (uint32_t)tcb[i].pid;
                    task = i;
                    found = true;
                    break;
                }

            }

            if(found && tcb[task].state !=STATE_STOPPED){
                killThread(pid);
            }else{
                putsUart0("Invalid function\n");
            }






            break;
        }case IPCS:
        {
            ipcsMutex_* ipcs_mutex = (ipcsMutex_*)*getPsp();
            ipcsSemaphore_* ipcs_sempahore = (ipcsSemaphore_*)*getR1();

            uint8_t i = 0,j = 0, task;

            for(i = 0; i < MAX_MUTEXES; i++)
            {
               //ipcs_mutex[i].mutexLock = mutexes[i].lock;
               ipcs_mutex[i].mutexLockedBy = mutexes[i].lockedBy;

               myStrCpy(tcb[i+1].name, ipcs_mutex[i].name);
               //ipcs_mutex[i].processQueueMutex = mutexes[i].processQueue[i];
               ipcs_mutex[i].queuesizeMutex = mutexes[i].queueSize;
               if(mutexes[i].queueSize > 0){
                   task = mutexes[i].processQueue[i];
                   myStrCpy(tcb[task].name, ipcs_mutex[i].waiting);
               }

            }

            uint8_t semaphore = 0;

            for(j = 0; j < MAX_SEMAPHORES; j++)
            {
                ipcs_sempahore[j].countSemaphore = semaphores[j].count;
                //ipcs_sempahore[j].countSemaphore = j;
                ipcs_sempahore[j].queueSizeSemaphore = semaphores[j].queueSize;
                if(semaphores[j].queueSize > 0)
                {
                    semaphore = semaphores[j].processQueue[0];
                }
                if(semaphore > 0)
                    myStrCpy(tcb[semaphore].name, ipcs_sempahore[j].queueName);


            }




            break;
        }case PS:
        {
            ps_* ps_information = (ps_*)*getPsp();
            uint8_t i = 0;


            //if writeA is true :: READ bufferB
            //if writeA is false :: READ bufferA

            for(i = 0; i < 10; i++) //to 10 or maybe MAX_TASK
            {

                if(writeA == true)
                {
                    //read from B
                    ps_information[i].cpuTime  = tcb[i].bufferA;
//                    tcb[i].bufferB  = 0;
//                    tcb[i].startTime = 0;

                }else{
                    //read from A
                    ps_information[i].cpuTime  = tcb[i].bufferB;
//                    tcb[i].bufferA  = 0;
//                    tcb[i].startTime = 0;
                }
            }



            for(i = 0; i < 10; i++) //to 10 or maybe MAX_TASK
            {
//                if(writeA == true)
//                {
//                    //read from B
//
//
//                }else{
//                    //read from A
//                    tcb[i].bufferA  = 0;
//                    tcb[i].startTime = 0;
//                }
                ps_information[i].pid = (uint32_t)tcb[i].pid;
                ps_information[i].state = (uint32_t)tcb[i].state;
                ps_information[i].mutex = (uint32_t)tcb[i].mutex;
                ps_information[i].semaphore = (uint32_t)tcb[i].semaphore;
                // place holder for now
                myStrCpy(tcb[i].name,ps_information[i].name);
            }

            break;
        }


        case MEM_INFO: //This one should be for MemInfo
        {
            memoryInfo* allocationInfo = (memoryInfo*)*getPsp();
            uint8_t i = 0, j = 0;



            for(i = 0; i < 11; i++)
            {
                allocationInfo[i].pid = allocations[i].pid;

                for(j = 0; j < MAX_TASKS; j++)
                {
                    if((uint32_t)tcb[j].pid == allocations[i].pid)
                    {
                        myStrCpy(tcb[j].name, allocationInfo[i].name);
                        break;
                    }
                }

                allocationInfo[i].stackBytes = allocations[i].size;

                allocationInfo[i].base = allocations[i].bottom_add;
            }



            break;
        }case REBOOT: // Reboots device. SVC needed because user cannot access NVIC Registers
        {
            NVIC_APINT_R = (0X05FA0000 | NVIC_APINT_SYSRESETREQ);


            break;
        }case THREAD_PRIO: //Sets thread Priority. This gets called by push button #5
        {
            uint32_t pid = 0, priority= 255;
            uint8_t i = 0;
            pid = (uint32_t)*getPsp();
            priority = (uint32_t)*(getPsp() + 1);

            for(i = 0; i < MAX_TASKS;i++)
            {
                if(pid == (uint32_t)tcb[i].pid)
                {
                    tcb[i].priority = priority;
                    break;
                }
            }


            break;
        }
        default:
        {
            break;
        }

    }
}


//
//case PKILL: //BY NAME NEEDS FIXING
//        {
//            //Similar to Create Thread
//            //puts state back to READY
//
//            uint32_t i = 0, result = 1000, task = 255;
//            char *proc_name;
//            proc_name =(char*)*getPsp();
//            bool found = false;
//
//            for(i = 0; i < MAX_TASKS; i++)
//            {
//                result = my_strcmp(proc_name, tcb[i].name);
//                if(result == 0)
//                {
//                    //pid = (uint32_t)tcb[i].pid;
//                    //should get task #
//                    task = i;
//                    found = true;
//                    break;
//                }
//            }
//
//            if(found && tcb[i].state !== STATE_STOPPED){
//                killThread(task);
//            }
//
//
//            break;
//        }case 35:
//
