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
#include "shell.h"
#include "asm.h"
#include "kernel.h"
#include "clock.h"
#include "tasks.h"

// REQUIRED: Add header files here for your strings functions, ...

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// REQUIRED: add processing for the shell commands through the UART here



//Reboots the microcontroller. This will be implemented as part of the mini project
//void reboot()
//{
//    NVIC_APINT_R = (0X05FA0000 | NVIC_APINT_SYSRESETREQ);
//}

void rows(uint16_t width)
{
    uint16_t j = 0;
    for(j = 0; j < width; j++)
    {
        putsUart0("-");
    }
    putsUart0("\n");
}


//uint32_t totalTime(uint32_t time)
//{
//
//}

//Displays the process(thread) status. For now, it calls a function ipcs()
//which displays the text "PS called"
void ps()
{
    //uint8_t i = 0;
    ps_ psData[MAX_TASKS];

    uint32_t totalTime = 0,percentage = 0,integerPart = 0, decimalValue = 0;

    uint8_t i = 0 ,j = 0, length = 0, width = 0;

    char buffer[150];
    putsUart0("\t\tPS called\n\n");
    putsUart0("\n\nPID\t\tTask Name\t\tCPU%\t\tSTATE\tBlocked By\n");
    rows(70);


    _psInfo(psData);

    for(i = 0; i < 10;i++)
    {
        totalTime += psData[i].cpuTime;
    }


    for(i = 0; i < 10; i++)
    {
        //ONLY print if pid is ! = 0
       //
            width = 28;

           intToAscii(psData[i].pid,buffer);
           putsUart0(buffer);
           putsUart0("\t");

           putsUart0(psData[i].name);

           length = stringLength(psData[i].name);

           if(length == 6){
               width = 27;
           }
           else if(length == 7)
           {
               width = 27;
           }
           else if(length == 8){
               width = 25;
           }
           else if(length == 9){
               width = 26;
           }

           for(j = length;j < width; j++)
           {
               putsUart0(" ");
           }


//
//           intToAscii(psData[i].cpuTime, buffer);
//           putsUart0(buffer);
//           putsUart0("\t\t");




           percentage = (psData[i].cpuTime *10000)/ totalTime;
//           intToAscii(percentage, buffer);
//           putsUart0(buffer);


           putsUart0("    %");



           integerPart = (percentage / 100);

           intToAscii(integerPart, buffer);
           putsUart0(buffer);
           putsUart0(".");

           decimalValue = integerPart % 100;
           intToAscii(integerPart, buffer);
           putsUart0(buffer);

           putsUart0("\t\t");
           intToAscii(psData[i].state,buffer);
           putsUart0(buffer);

           if(psData[i].state == 4){
               putsUart0("\t\t");
               intToAscii(psData[i].mutex,buffer);
               putsUart0(buffer);
           }

           putsUart0("\n");
           rows(70);
    }



}

void row()
{
    uint16_t j = 0;
    for(j = 0; j < 55; j++)
    {
        putsUart0("-");
    }
    putsUart0("\n");
}



void col()
{
    putsUart0("     |");
}

//Displays the inter-process (thread) communication status.
//For now, it calls a function ipcs() which displays the text “IPCS called”
void ipcs()
{

    putsUart0("\n\tIPCS called\n\n");
    ipcsMutex_ ipcs_mutex[2];
    ipcsSemaphore_ ipcs_semaphore[3];
    uint8_t i = 0, j = 0;
    char buffer[150];
    _ipcsCommand(ipcs_mutex, ipcs_semaphore);

    rows(70);
    putsUart0("MUTEX  1");

    for(i = 0; i < 1; i++)
    {

        putsUart0("  LOCKED BY:  ");

        putsUart0(ipcs_mutex[i].name);
        putsUart0("   QUEUE SIZE:  ");

        intToAscii((uint32_t)ipcs_mutex[i].queuesizeMutex, buffer);
        putsUart0(buffer);

        putsUart0("   WAITING:  ");
        putsUart0(ipcs_mutex[i].waiting);
        putsUart0("\n");
        rows(70);
    }


    for(j = 0; j < 3; j++)
    {

        putsUart0("SEMAPHORE  ");
        intToAscii((uint32_t)j, buffer);
        putsUart0(buffer);

        putsUart0("   Count:  ");
        intToAscii((uint32_t)ipcs_semaphore[j].countSemaphore, buffer);
        putsUart0(buffer);

        putsUart0("   ");
        putsUart0("QueueSize:  ");
        intToAscii((uint32_t)ipcs_semaphore[j].queueSizeSemaphore, buffer);
        putsUart0(buffer);
        putsUart0("   ");
        putsUart0("Waiting:  ");
        if(ipcs_semaphore[j].queueSizeSemaphore > 0){
            putsUart0(ipcs_semaphore[j].queueName);
        }


        putsUart0("\n\n");
    }
    rows(70);


}


void meminfo()
{
    memoryInfo allocation[12];
    char buffer[150];


    _memoryInfo(allocation);

    putsUart0("\n\nPID\t\tTASK\t\t\tBASE ADD\t\tSIZE\n");

    row();
    uint8_t i = 0 ,j = 0, length = 0, width = 0, tab = 0;
    for(i = 0; i < 11;i++)
    {

        tab = 25;

        //ONLY print if pid is ! = 0
        if(allocation[i].pid != 0){
            width = 25;

           intToAscii(allocation[i].pid,buffer);
           putsUart0(buffer);
           //putsUart0("\t");
           putsUart0("    ");

           putsUart0(allocation[i].name);

           length = stringLength(allocation[i].name);

//           if(length == 6){
//               width = 27;
//           }
//           else if(length == 7)
//           {
//               width = 27;
//           }
//           else if(length == 8){
//               width = 25;
//           }
//           else if(length == 9){
//               width = 26;
//           }
           //tab = tab - length;

           if(length <= 5){
               for(j = length;j < 29; j++)
                          {
                              putsUart0(" ");
                          }
           }else if(length == 6)
           {
               for(j = length;j < 28; j++)
                           {
                                    putsUart0(" ");
                           }
           }else if(length == 7)
           {
               for(j = length;j < 24; j++)
                           {
                                    putsUart0(" ");
                           }
           }else if(length == 8)
           {
               for(j = length;j < 24; j++)
                           {
                                    putsUart0(" ");
                           }
           }else if(length == 9)
           {
               for(j = length;j < 25; j++)
                           {
                                    putsUart0(" ");
                           }
           }else
           {
               for(j = length;j < 23; j++)
                          {
                              putsUart0(" ");
                          }
           }



          putsUart0("   ");


//           putsUart0("  ");


           uintToHex((uint32_t)allocation[i].base, buffer);
           putsUart0(buffer);
           putsUart0("\t\t");

           intToAscii(allocation[i].stackBytes, buffer);
           putsUart0(buffer);

           putsUart0("\n");
           row();
        }
    }
}


//Kills the process (thread) with the matching PID. For now, it calls a function kill(uint32_t pid)
//which displays the text “PID killed” (PID is the number passed)
void kill_Pid(uint32_t pid)
{
    stopThread((_fn)pid);
    //Integer to Ascci function NEEDED
    char intToStringVal[12]={'1'};
    intToAscii(pid,intToStringVal);
    //intToStringVal --;
    putsUart0(intToStringVal);
    putsUart0(" Killed \n");

}


//Kills the thread based on the process name
void pkill(char proc_name[])
{
    stopThread(proc_name);
    //AsciiToInteger()
    putsUart0(proc_name);
    putsUart0(" has been killed\n");
}


//Turns priority inheritance on or off.
//For now, it calls a function pi(bool on) that displays “pi on” or “pi off”.
void pi(bool on)
{
    if(on){
        putsUart0("PI ON\n");
    }else{
        putsUart0("PI OFF\n");
    }

}


//Turns preemption on or off. For now, it calls a function
//preempt(bool on) that displays “preeempt on” or “preempt off”.
void preempt(bool on)
{
    if(on)
    {
        _preempt(true);
        putsUart0("PREEMPT ON\n");
    }else
    {
        putsUart0("PREEMPT OFF\n");
    }

}


//Selected priority or round-robin scheduling.
//For now, it calls a function sched(bool prio_on) that displays “sched prio” or “sched rr”.
void sched(bool prio_on)
{

    if(prio_on)
    {
        putsUart0("sche PRIO\n");
        _sched(true);
    }else
    {
        _sched(false);
        putsUart0("sche RR\n");
    }
}


//Displays the PID of the process (thread). For now, it calls a
//function pidof(const char name[]) that displays “proc_name launched”.
void pidof(char *name)
{
    char buffer[15];
    uint32_t pid=1000;
    pid = _pidof(name);
    if(pid == 1000){
        putsUart0("NOT FOUND \n");
    }else{
        intToAscii(pid, buffer);
        putsUart0(buffer);
        putsUart0("\t");
        uintToHex(pid,buffer);
        putsUart0(buffer);
        putsUart0("\n");

    }


}


//Runs the selected program in the background. For now, turn on the red LED.
void proc_name(char *name)
{
//    char functions[3][15] = {"Hello", "Test", "Rand"};
//
//    bool state = false;
//    if(my_strcmp(name, functions[0]) == 0){
//        state = true;
//    }else if(my_strcmp(name, functions[1]) == 0)
//    {
//        state = true;
//    }else if(my_strcmp(name, functions[2]) == 0)
//    {
//        state = true;
//    }
    //uint32_t error;
    restartThread(name);

//    if(error == 255)
//    {
//        putsUart0("Invalid function\n");
//    }

//    if(state){
//        //Turn on RED LED
//        setPinValue(PORTF,1,1);
//        waitMicrosecond(2000000);
//        setPinValue(PORTF,1,0);
//    }else{
//        putsUart0("Invalid function\n");
//    }


}

void help()
{

    putsUart0("Commands:\n");
    putsUart0(" reboot \n");
    putsUart0(" ps \n");
    putsUart0(" ipcs \n");
    putsUart0(" kill \n");
    putsUart0(" pkill \n");
    putsUart0(" pi on | off  --Priority on | off \n");
    putsUart0(" preempt on | off \n");
    putsUart0(" sched on | off  --Selects between Priority Schedule or Round Robin \n");
    putsUart0(" pidof \n");
    putsUart0(" meminfo \n");


}

void shell(void) {
    USER_DATA data;


    while(true)
    {
        //readPbs();

        if(kbhitUart0()){

            getsUart0(&data);
                    parseFields(&data);


                    if (isCommand(&data, "reboot", 0) && data.fieldCount == 1) {
                        _reboot();
                    } else if (isCommand(&data, "ps", 0) ) {

                        ps();

                    } else if (isCommand(&data, "ipcs", 0) && data.fieldCount == 1) {
                        //Insert div/0 to cause usage fault
                        ipcs();

                    } else if (isCommand(&data, "kill", 0)&& data.fieldCount == 2) {
                        // Implement kill command
                        int32_t pid = getFieldInteger(&data,1);
                        kill_Pid(pid);

                    } else if (isCommand(&data, "pkill", 0) && data.fieldCount == 2) {
                        // putsUart0("REBOOT\n");
                        char *proc_name;
                        proc_name = getFieldString(&data,2);
                        pkill(proc_name);

                    } else if (isCommandNew(&data, "pi", 1)) {
                        if (isCommandNew(&data, "ON", 2) || isCommandNew(&data, "on", 2)) {
                            pi(1);
                        } else if (isCommandNew(&data, "OFF", 2) || isCommandNew(&data, "off", 2)) {
                            pi(0);
                        }

                    } else if (isCommandNew(&data, "preempt", 1)) {
                        if (isCommandNew(&data, "ON", 2) || isCommandNew(&data, "on", 2)) {
                            preempt(1);
                        } else if (isCommandNew(&data, "OFF", 2) || isCommandNew(&data, "off", 2)) {
                            preempt(0);
                        }

                    } else if (isCommandNew(&data, "sched", 1)) {
                        if (isCommandNew(&data, "PRIO", 2) || isCommandNew(&data, "prio", 2)) {
                            sched(1);
                        } else if (isCommandNew(&data, "RR", 2) || isCommandNew(&data, "rr", 2)) {
                            sched(0);
                        }

                    } else if (isCommand(&data, "pidof", 0) && data.fieldCount == 2) {
                        //FIX THIS ONE proc_name is
                        char *str;
                        str = getFieldString(&data,2);
                        pidof(str);

                    } else if (isCommand(&data, "meminfo", 0) && data.fieldCount == 1) {
                        //FIX THIS ONE proc_name is
                        //char *str;
                        //str = getFieldString(&data,2);
                        meminfo();

                    }else if (isCommand(&data, "help", 0) && data.fieldCount == 1) {
                        //FIX THIS ONE proc_name is
                        //char *str;
                        //str = getFieldString(&data,2);
                        help();

                    }else{
                        char *str1 = getFieldString(&data, 1);
                        proc_name(str1);
                    }
        }
        yield();
    }

}
