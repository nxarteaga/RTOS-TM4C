//NEEDS INTEGER to ASCII FUNCTION
// Serial Example
// Modified by Nestor Arteaga
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "kernel.h"

// Bitband aliases
#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))

// PortF masks
#define GREEN_LED_MASK 8
#define RED_LED_MASK 2



//for string manipulation lab 4
#define MAX_CHARS 80
#define MAX_FIELDS 6


#define ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
//#define IS_NUM(c) (((c) >= '0' && (c) <= '9') || (c) == '-' || (c) == '.' || (c) == '_' || (c) == ' ')
#define IS_NUM(c) (((c) >= '0' && (c) <= '9') || (c) == '-' || (c) == '.' || (c) == '_')





//Struct for holding UI Information
typedef struct USER_DATA
{
    char buffer [MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
}USER_DATA;
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw4()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    // Configure LED pins
    GPIO_PORTF_DIR_R |= GREEN_LED_MASK | RED_LED_MASK;  // bits 1 and 3 are outputs
    GPIO_PORTF_DR2R_R |= GREEN_LED_MASK | RED_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R |= GREEN_LED_MASK | RED_LED_MASK;  // enable LEDs
}









// Function to get a string from UART0
//void getsUart0(USER_DATA* data) {
//    char c;
//    uint8_t count = 0;
//
//    while (1) {
//        // Get a character from UART0
//        c = getcUart0();
//
//        // Handle backspace
//        if (c == 8 || c == 127) {
//           // putsUart0("backspace");
//            if (count > 0) {
//                count--;
//            }
//        }
//        // Handle carriage return
//        else if (c == 13) {
//            //putsUart0("carriage");
//            data->buffer[count] = '\0';  // Add null terminator
//            return;  // Exit the function
//        }
//        // Handle space or printable character
//        else if (c >= 32) {
//            // Store the character in the buffer
//            data->buffer[count] = c;
//            count++;
//
//            // Check if the buffer is full
//            if (count == MAX_CHARS) {
//                data->buffer[count] = '\0';  // Add null terminator
//                return;  // Exit the function
//            }
//        }
//    }
//}












// Function to get a string from UART0
void getsUart0(USER_DATA* data) {
    char c;
    uint8_t count = 0;

    while (1) {
        if(kbhitUart0())
        {
            // Get a character from UART0
            c = getcUart0();

            // Handle backspace
            if (c == 8 || c == 127) {
               // putsUart0("backspace");
                if (count > 0) {
                    count--;
                }
            }
            // Handle carriage return
            else if (c == 13) {
                //putsUart0("carriage");
                data->buffer[count] = '\0';  // Add null terminator
                return;  // Exit the function
            }
            // Handle space or printable character
            else if (c == 32 || c >= 32) {
                // Store the character in the buffer
                data->buffer[count] = c;
                count++;

                // Check if the buffer is full
                if (count == MAX_CHARS) {
                    data->buffer[count] = '\0';  // Add null terminator
                    return;  // Exit the function
                }
            }

        }else{
            yield();
        }
    }
}

//


void parseFields(USER_DATA *userData) {

    uint8_t i=0;
    uint8_t index=0;
    char current_char;
    bool prevChar = true;
    userData->fieldCount = 0;

    while(userData->buffer[i]){
        //if(prevChar){
        current_char = userData->buffer[i];
        if(ALPHA(current_char)){
            if(prevChar){
                userData->fieldCount++;
                userData->fieldType[index] = 'a';
                userData->fieldPosition[index] = i;
                //userData->fieldPosition[index] = next;
                //next++;
                //putsUart0("alpha\n");
                prevChar = false;
                index++;
            }
        }else if(IS_NUM(current_char)){
           if(prevChar){
                userData->fieldCount++;
                userData->fieldType[index] = 'n';
                userData->fieldPosition[index] = i;
                //userData->fieldPosition[index] = next;
                //next++;
                index++;
                //putsUart0("numeric\n");
                prevChar = false;
           }
        }else{                       //userData->fieldType[index] = 'd';
             userData->buffer[i] = '\0';
             //putsUart0("Delimiter\n");
             prevChar = true;
        }
        i++;
    }

        //count++;
        //userData->fieldCount[&index] = count
}

/*
void parseFields(USER_DATA *userData) {
 for(i = 0; i < userData->fieldCount; i++){
                putcUart0(userData->fieldType[i]);
                putcUart0('\t');
                putsUart0(&userData->buffer[userData->fieldPosition[i]]);
                putcUart0('\n');
    }





    int field_count = 0;
    char prev_char = ' '; // Assume the previous character is a delimiter
    int i = 0;
    //bool delimiter true;

    //while()
    for (i = 0; userData->buffer[i] != '\0' && field_count < MAX_FIELDS; i++) {
        char current_char = userData->buffer[i];

        if (!IS_ALPHA(current_char) && !IS_NUMERIC(current_char)) {
            // If the current character is a delimiter
            if (prev_char != ' ') {
                // If the previous character was not a delimiter, mark the end of the field
                userData->fieldType[field_count] = (prev_char == '-' || prev_char == '.') ? 'n' : 'a'; // 'a' for alpha, 'n' for numeric
                userData->fieldPosition[field_count] = i;
                field_count++;
            }


            // Convert delimiter to NULL character
            userData->buffer[i] = '\0';
            }
            prev_char = current_char;
        }
           // If the buffer ends with a field, mark it
           if (field_count < MAX_FIELDS && (IS_ALPHA(prev_char) || IS_NUMERIC(prev_char))) {
                   userData->fieldType[field_count] = (prev_char == '-' || prev_char == '.') ? 'n' : 'a'; // 'a' for alpha, 'n' for numeric
                   userData->fieldPosition[field_count] = i; // Set the position to the end of the buffer
                   field_count++;
              }
       // Update fieldCount
       userData->fieldCount = field_count;
}
*/


void intToAscii(uint32_t integerValue,char intToStringValue[])
{

    if(integerValue == 0)
    {
        intToStringValue[0] = '0';
        intToStringValue[1] = '\0';
        return;
    }



    int digit, strLength;
    //char intToStringValue[10];
    uint8_t index = 0;
    while (integerValue != 0) {
           digit = integerValue % 10;
           intToStringValue[index++] = digit + '0';  // Convert digit to ASCII
           integerValue /= 10;
    }

    strLength = index;
    intToStringValue[index] = '\0';

    uint8_t i;
    char tempVariable;
    for(i = 0; i < strLength / 2; i++)
    {
        tempVariable = intToStringValue[i];
        intToStringValue[i] = intToStringValue[strLength - i - 1];
        intToStringValue[strLength - i - 1] = tempVariable;
    }
}


// A to I function
int32_t AsciiToInteger(char* str)
{
    int32_t result = 0;
    // Initialize sign as positive
    int sign = 1;

    int i = 0;

    // If number is negative,
    // then update sign
    if (str[0] == '-') {
        sign = -1;

        // Also update index of first digit
        i++;
    }

    // Iterate through all digits
    // and update the result
    for (; str[i] != '\0'; ++i)
        result = result * 10 + str[i] - '0';

    // Return result with sign
    return sign * result;
}

char* getFieldString(USER_DATA* data, uint8_t fieldNumber){
    if(fieldNumber <= data->fieldCount){
        return &(data->buffer[data->fieldPosition[fieldNumber-1]]);
        //putsUart0("works\n");
    }
   return NULL;
}


int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber){
    int32_t val;
    if(fieldNumber <= data->fieldCount && (data->fieldType[fieldNumber] == 'n')){
        val = AsciiToInteger(&data->buffer[data->fieldPosition[fieldNumber]]);
        //return (&data->buffer[data->fieldPosition[fieldNumber-1]]);
        //putsUart0("works\n");
      return val;
    }
   return 0;
}

void myStrCpy(const char source[], char destination[])
{
    while(*source !='\0')
    {
        *destination = *source;
        source ++;
        destination++;
    }
    *destination = '\0';
}


int my_strcmp(const char *ptrToStringOne, const char *ptrToStringTwo) {

    //while ptrToStringOne exist and *ptrToStringOne == *ptrToStringTwo
    while (*ptrToStringOne && *ptrToStringOne == *ptrToStringTwo) {

        //Move pointers to next position
        ++ptrToStringOne;
        ++ptrToStringTwo;
    }
    return (int)(unsigned char)(*ptrToStringOne) - (int)(unsigned char)(*ptrToStringTwo);
}

void uintToHex(uint32_t address, char hexAddress[]) {
    int8_t i = 0, index = 2;
    hexAddress[0] = '0';
    hexAddress[1] = 'x';
    uint8_t nibble = 0;

    for (i = 7; i >= 0; i--) {
        nibble = (address >> (i * 4)) & 0xF;

        if (nibble >= 10) {
            hexAddress[index] = 'A' + (nibble - 10);  // Convert 10-15 to 'A'-'F'
        } else {
            hexAddress[index] = '0' + nibble;         // Convert 0-9 to '0'-'9'
        }
        index++;
    }
    hexAddress[index] = '\0';  // Null-terminate the string
}


bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments) {

    uint8_t argCount = (data->fieldCount)-1;
    uint8_t string_result;
    char *str;

    str = getFieldString(data,1);
    string_result = my_strcmp(str,strCommand);
    if((string_result == 0) && argCount >= minArguments){
        //putsUart0("More than 2\n");
        return true;
    }else{
        //putsUart0("less than 2\n");
        return false;
    }

}

bool isCommandNew(USER_DATA* data, const char strCommand[], uint8_t minArguments) {

    //uint8_t argCount = (data->fieldCount)-1;
    uint8_t string_result;

    char *str;

    str = getFieldString(data,minArguments);
    string_result = my_strcmp(str,strCommand);
    if((string_result == 0)){
        //putsUart0("More than 2\n");
        return true;
    }else{
        //putsUart0("less than 2\n");
        return false;
    }

}

uint16_t stringLength(const char source[])
{
    uint16_t length = 0;
    while(*source !='\0')
    {
        source ++;
        length++;

    }

    return length;
}

