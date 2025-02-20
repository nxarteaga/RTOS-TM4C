// Shell functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef SHELL_H_
#define SHELL_H_

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>
#include <wait.h>
#include "uart0.h"
#include "nvic.h"
#include <clock.h>
#include "gpio.h"
#include "custom_str_library.h"





void initHwShell();
void yield();
void reboot();
void ps();
void ipcs();
void kill_Pid(uint32_t pid);
void pkill(char proc_name[]);
void pi(bool on);
//void shell();
void shell(void);

#endif
