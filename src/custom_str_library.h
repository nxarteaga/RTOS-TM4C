#ifndef CUSTOM_STR_LIBRARY_H_
#define CUSTOM_STR_LIBRARY_H_

#include <stdbool.h>


#define MAX_CHARS 80
#define MAX_FIELDS 6
#define ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
//#define IS_NUM(c) (((c) >= '0' && (c) <= '9') || (c) == '-' || (c) == '.')
#define IS_NUM(c) (((c) >= '0' && (c) <= '9') || (c) == '-' || (c) == '.' || (c) == '_' || (c) == '32')

typedef struct USER_DATA
{
    char buffer [MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
}USER_DATA;

void getsUart0(USER_DATA* data);
void parseFields(USER_DATA *userData);
int32_t AsciiToInteger(char* str);
void intToAscii(uint32_t integerValue,char intToStringValue[]);
char* getFieldString(USER_DATA* data, uint8_t fieldNumber);
int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber);
int my_strcmp(const char *a, const char *b);
void myStrCpy(char source[], char destination[]);
void uintToHex(uint32_t address, char hexAddress[]);
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments);
bool isCommandNew(USER_DATA* data, const char strCommand[], uint8_t minArguments);

uint16_t stringLength(const char source[]);

#endif
