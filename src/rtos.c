// RTOS Framework - Fall 2023
// J Losh

// Student Name: Nestor Arteaga.
// TO DO: Fix allocation for 1536 task. ADD code in create thread.
//        Do not include your ID number(s) in the file.

// Please do not change any function name in this code or the thread priorities

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// 6 Pushbuttons and 5 LEDs, UART
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1
// Memory Protection Unit (MPU):
//   Region to control access to flash, peripherals, and bitbanded areas
//   4 or more regions to allow SRAM access (RW or none for task)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include "tm4c123gh6pm.h"
#include "clock.h"
#include "gpio.h"
#include "uart0.h"
#include "wait.h"
#include "mm.h"
#include "kernel.h"
#include "faults.h"
#include "tasks.h"
#include "shell.h"

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

void enableFaultHandlers()
{
    //NVIC_CFG_CTRL_R |= NVIC_CFG_CTRL_DIV0;
    //Sets the Trap on Divide by Zero. Page 169
   // NVIC_CFG_CTRL_R |= (1 << 4);

    //enable Fault handlers.    Page 173
    //Bit 18 Usage, Bit 17 Bus Fault, Bit 16 Memory Management Fault
    NVIC_SYS_HND_CTRL_R |=  (1<<18) | (1<<17) |(1<<16);
}


int main(void)
{
    bool ok;

    // Initialize hardware
    initSystemClockTo40Mhz();
    initHw(); //push buttons and LEDS
    initUart0();

    allowFlashAccess();
    allowPeripheralAccess();
    setupSramAccess();
    //we will not initialize MPU until later.
    //enable Fault handlers here for DEBUG
    enableFaultHandlers();
    //enable MPU
    mpuOverallBackground();



    initRtos();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);




    // Initialize mutexes and semaphores
    initMutex(resource);
    initSemaphore(keyPressed, 1); //Initialize debounce.
    initSemaphore(keyReleased, 0);// key has not been released.
    initSemaphore(flashReq, 5);// 5 seconds power on,

    // Add required idle process at lowest priority.
    ok =  createThread(idle, "Idle", 15, 512); //There must be one program available at all times.
    //This one is here for visual observation . This is the lowest priority.
    //ok &=  createThread(idle2, "Idle2", 13, 512);

    //ok &=  createThread(idle3, "Idle3", 0, 512);


     //Add other processes. If there is no memory for Malloc it will fail.
    ok &= createThread(lengthyFn, "LengthyFn", 12, 1024);
    ok &= createThread(flash4Hz, "Flash4Hz", 8, 512);
    ok &= createThread(oneshot, "OneShot", 4, 1024);//This one should be 1536
    ok &= createThread(readKeys, "ReadKeys", 12, 1024);
    ok &= createThread(debounce, "Debounce", 12, 1024);
    ok &= createThread(important, "Important", 0, 1024);
    ok &= createThread(uncooperative, "Uncoop", 12, 1024);
    ok &= createThread(errant, "Errant", 12, 512); // will cause an MPU fault.
    ok &= createThread(shell, "Shell", 12, 4096);
   // ok &= createThread(memInfo, "MemInfo", 12, 512);

//If RTOS doesnt run is becuase one of the threads failed.
    // STEP 19 of project. TODO: Add code to implement a periodic timer and ISR
    initTimer();
    // Start up RTOS
    if (ok)
        startRtos(); // never returns
    else
        while(true);
}
