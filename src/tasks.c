// Tasks
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
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "wait.h"
#include "kernel.h"
#include "tasks.h"
#include "clock.h"
#include "mm.h"

//#define BLUE_LED   PORTF,2 // on-board blue LED
//#define RED_LED    PORTE,0 // off-board red LED
//#define ORANGE_LED PORTA,2 // off-board orange LED
//#define YELLOW_LED PORTA,3 // off-board yellow LED
//#define GREEN_LED  PORTA,4 // off-board green LED

// Pin bitbands
#define GREEN_LED_BOARD    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))

// PortF masks
#define GREEN_LED_MASK 8

#define RED_LED PORTA,2
#define PB0 PORTC,4
#define PB1 PORTC,5
#define PB2 PORTC,6
#define PB3 PORTC,7
#define PB4 PORTD,6
#define PB5 PORTD,7
#define BLUE_LED PORTF,2
#define GREEN_LED PORTE,0
#define YELLOW_LED PORTA,4
#define ORANGE_LED PORTA,3
#define RED_LED_PORTA2 PORTA,2
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------




// Initialize Hardware
// REQUIRED: Add initialization for blue, orange, red, green, and yellow LEDs
//           Add initialization for 6 pushbuttons

initTimer()
{
    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

       // Configure LED pin
       GPIO_PORTF_DIR_R |= GREEN_LED_MASK;
       GPIO_PORTF_DEN_R |= GREEN_LED_MASK;
       //Will be operating in Periodic timer mode  ****PAGE 723 has the steps to turn on the timer. The steps should align with the code below.
       // Configure Timer 1 as the time base

       TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring. TURN OFF BEFORE RECONFIGURING for ALL PERIPHERALS. TIMER CONTROL REGISTER
       //PAGE 728 of DATA SHEET ---MODE ZERO IN THE First three bits
       TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
       //TIMER A MODE REGISTER PAGE 729. LOWER bits tells us how we are going to run. Capture mode, Periodic Timer.
       TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
       //40 Million
       TIMER1_TAILR_R = 40000000;                       // set load value to 40e6 for 1 Hz interrupt rate
       //Interrupt mask register
       TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts for timeout in timer module
       //
       TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
       //This is on the sys control register. They are powered all the time. They cannot be turned off.
       //NVIC ENABLE REGISTER
       NVIC_EN0_R = 1 << (INT_TIMER1A-16);
}


// Periodic timer < less than 1 micro second
void timer1Isr()
{
    GREEN_LED_BOARD ^= 1;
    //Chapter 3 PAGE 103. TIMER1_ICR_R  ---TIMER INTERUPT CLEAR REGISTER
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;               // clear interrupt flag
}


void initHw(void)
{
    // Setup LEDs and pushbuttons
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    enablePort(PORTF);
    enablePort(PORTA);
    enablePort(PORTC);
    enablePort(PORTD);
    enablePort(PORTE);
    _delay_cycles(3);


    // Power-up flash
    setPinValue(GREEN_LED, 1);
    waitMicrosecond(250000);
    setPinValue(GREEN_LED, 0);
    waitMicrosecond(250000);


    selectPinPushPullOutput(RED_LED);
       //selectPinPushPullOutput(RED_LED_PORTA2);
       selectPinPushPullOutput(BLUE_LED);
       selectPinPushPullOutput(GREEN_LED);
       selectPinPushPullOutput(YELLOW_LED);
       selectPinPushPullOutput(ORANGE_LED);


       //Push buttons - pulls ups.
       selectPinDigitalInput(PB0);
       enablePinPullup(PB0);

       selectPinDigitalInput(PB1);
       enablePinPullup(PB1);

       selectPinDigitalInput(PB2);
       enablePinPullup(PB2);

       selectPinDigitalInput(PB3);
       enablePinPullup(PB3);

       selectPinDigitalInput(PB4);
       enablePinPullup(PB4);

       setPinCommitControl(PB5);
       selectPinDigitalInput(PB5);
       enablePinPullup(PB5);
}

// REQUIRED: add code to return a value from 0-63 indicating which of 6 PBs are pressed
uint8_t readPbs(void)
{
    uint8_t num = 0;

        if(!getPinValue(PB0))
            num = 1;

        if(!getPinValue(PB1))
            num = 2;

        if(!getPinValue(PB2))
            num = 4;

        if(!getPinValue(PB3))
            num = 8;

        if(!getPinValue(PB4))
            num = 16;

        if(!getPinValue(PB5))
            num = 32;


        return num;
}

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose


//To measure the amount of time the task switchin takes, just have tasks: idle and idle2.
//set breakpoints in idle @ yield() and idle2 @ setPinValue. When I get to yield, zero out clocks,
//then run the code. to find the amount of time; divide clocks by 40Mhz, this will give you microseconds.

void idle(void)
{
    while(true)
    {
        //lock(resource);
        setPinValue(ORANGE_LED, 1);
        waitMicrosecond(1000);
        setPinValue(ORANGE_LED, 0);
        //sleep(125); // DEBUG
        //unlock(resource);
        yield();
    }
}

void memInfo(void)
{





}




void idle2(void)
{
    while(true)
    {
        //lock(resource);
        setPinValue(BLUE_LED, 1);
        waitMicrosecond(1000);
        setPinValue(BLUE_LED, 0);
        //unlock(resource);
        yield();
    }
}


void idle3(void)
{
    while(true)
    {
        //lock(resource);
        setPinValue(RED_LED, 1);
        waitMicrosecond(1000);
        setPinValue(RED_LED, 0);
        //unlock(resource);
        yield();
    }
}


void flash4Hz(void)
{
    while(true)
    {
        setPinValue(GREEN_LED, !getPinValue(GREEN_LED));
        sleep(125);
    }
}

void oneshot(void)
{
    while(true)
    {
        wait(flashReq);
        setPinValue(YELLOW_LED, 1);
        sleep(1000);
        setPinValue(YELLOW_LED, 0);
    }
}

void partOfLengthyFn(void)
{
    // represent some lengthy operation
    waitMicrosecond(990);
    // give another process a chance to run
    yield();
}

void lengthyFn(void)
{
    uint16_t i;
    uint8_t *mem;
    //mem = mallocFromHeap(5000 * sizeof(uint8_t));
    mem = (uint8_t*)mallocFromHeapWrapper(5000 * sizeof(uint8_t));
    while(true)
    {
        lock(resource);
        for (i = 0; i < 5000; i++)
        {
            partOfLengthyFn();
            mem[i] = i % 256;
        }
        setPinValue(RED_LED, !getPinValue(RED_LED));
        unlock(resource);
    }
}

void readKeys(void)
{
    uint8_t buttons;
    while(true)
    {
        wait(keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            yield();
        }
        post(keyPressed);
        if ((buttons & 1) != 0)
        {
            setPinValue(YELLOW_LED, !getPinValue(YELLOW_LED));
            setPinValue(RED_LED, 1);
        }
        if ((buttons & 2) != 0)
        {
            post(flashReq);
            setPinValue(RED_LED, 0);
        }
        if ((buttons & 4) != 0)
        {
            restartThread(flash4Hz);
        }
        if ((buttons & 8) != 0)
        {
            stopThread(flash4Hz);
            //stopThread(lengthyFn);
        }
        if ((buttons & 16) != 0)
        {
            setThreadPriority(lengthyFn, 4);
        }
        yield();
    }
}

void debounce(void)
{
    uint8_t count;
    while(true)
    {
        wait(keyPressed);
        count = 10;
        while (count != 0)
        {
            sleep(10);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        post(keyReleased);
    }
}

void uncooperative(void)
{
    while(true)
    {
        while (readPbs() == 8)
        {
        }
        yield();
    }
}

void errant(void)
{
    uint32_t* p = (uint32_t*)0x20000000;
    while(true)
    {
        while (readPbs() == 32)
        {
            *p = 0;
        }
        yield();
    }
}

void important(void)
{
    while(true)
    {
        lock(resource);
        setPinValue(BLUE_LED, 1);
        sleep(1000);
        setPinValue(BLUE_LED, 0);
        unlock(resource);
    }
}
