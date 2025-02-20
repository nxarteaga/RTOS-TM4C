//Will move all faults to here from my current file.

// Shell functions
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
#include "faults.h"
#include "asm.h"
#include "custom_str_library.h"
#include "uart0.h"
#include "kernel.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// REQUIRED: If these were written in assembly
//           omit this file and add a faults.s file

// REQUIRED: code this function
void mpuFaultIsr(void)
{
    uint32_t pc   = *(getPC());
    uint32_t xpsr = *(getXPSR());
    uint32_t lr   = *(getLR());
    uint32_t r0 = *(getR0());
    uint32_t r1 = *(getR1());
    uint32_t r2 = *(getR2());
    uint32_t r3 = *(getR3());
    uint32_t r12 = *(getR12());
    uint32_t *psp = getPsp();
    uint32_t *msp = getMsp();
    uint32_t pid = (uint32_t)getPid();
    char buffer[150];


    putsUart0("\n MPU FAULT IN PROGRESS\n");
    putsUart0("\tMPU fault in process: ");
    uintToHex(pid, buffer);
    //intToAscii(pid,buffer);
    putsUart0(buffer);
    //putsUart0("\n");

    putsUart0("\tAddress of offending instruction: ");
    uintToHex((uint32_t)NVIC_MM_ADDR_R, buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0("\tPSP: ");
    uintToHex((uint32_t)psp, buffer);
    putsUart0(buffer);
    //putsUart0("\n");

    putsUart0("\tMSP: ");
    uintToHex((uint32_t)msp, buffer);
    putsUart0(buffer);
    //putsUart0("\n");

    putsUart0("\tMFAULT FLAGS: ");
    uint32_t reg, mflags;
    reg = NVIC_FAULT_STAT_R;
    mflags = (reg & 0xFF);
    uintToHex(mflags, buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" xPSR ");
    uintToHex(xpsr,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" PC\t");
    uintToHex(pc,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" LR\t");
    uintToHex(lr,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" R0\t");
    uintToHex(r0,buffer);
    putsUart0(buffer);
    putsUart0("\n");


    putsUart0(" R1\t");
    uintToHex(r1,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" R2\t");
    uintToHex(r2,buffer);
    putsUart0(buffer);
    putsUart0("\n");


    putsUart0(" R3\t");
    uintToHex(r3,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" R12\t");
    uintToHex(r12,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    //while(1);//DEBUG take off




    NVIC_SYS_HND_CTRL_R &= ~(NVIC_SYS_HND_CTRL_MEMP);
    killThread(pid);
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;

}

// REQUIRED: code this function
void hardFaultIsr(void)
{

    uint32_t pc   = *(getPC());
    uint32_t xpsr = *(getXPSR());
    uint32_t lr   = *(getLR());
    uint32_t r0 = *(getR0());
    uint32_t r1 = *(getR1());
    uint32_t r2 = *(getR2());
    uint32_t r3 = *(getR3());
    uint32_t r12 = *(getR12());
    uint32_t *psp = getPsp();
    uint32_t *msp = getMsp();
    uint32_t pid = (uint32_t)getPid();
    char buffer[100];


    putsUart0("Hard Fault in progress\n\n");
    putsUart0("Hard fault in process\t");
    intToAscii(pid,buffer);
    putsUart0(buffer);
    putsUart0("\n");


    putsUart0(" PSP: \t");
    uintToHex((uint32_t)psp, buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" MSP: \t");
    uintToHex((uint32_t)msp, buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" MFAULT FLAGS\t");
    uint32_t reg, mflags;
    reg = NVIC_FAULT_STAT_R;
    mflags = (reg & 0xFF);
    uintToHex(mflags, buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" xPSR\t");
    uintToHex(xpsr,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" PC\t");
    uintToHex(pc,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" LR\t");
    uintToHex(lr,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" R0\t");
    uintToHex(r0,buffer);
    putsUart0(buffer);
    putsUart0("\n");


    putsUart0(" R1\t");
    uintToHex(r1,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" R2\t");
    uintToHex(r2,buffer);
    putsUart0(buffer);
    putsUart0("\n");


    putsUart0(" R3\t");
    uintToHex(r3,buffer);
    putsUart0(buffer);
    putsUart0("\n");

    putsUart0(" R12\t");
    uintToHex(r12,buffer);
    putsUart0(buffer);
    putsUart0("\n");


while(true);

}

// REQUIRED: code this function
void busFaultIsr(void)
{
//    char buffer[15];
    putsUart0("BUS FAULT in process\n");
//    intToAscii(fn,buffer);
//    putsUart0(buffer);
//    putsUart0("\n");
    while(true);
    //caused by Precise Data Bus Error
}

// REQUIRED: code this function
void usageFaultIsr(void)
{
    putsUart0("USAGE FAULT in process N\n");
    while(true);
}

