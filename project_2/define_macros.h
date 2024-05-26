/**
* @file define_macros.h
* @brief General macros
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/


#ifndef DEFINED_MACROS
#define DEFINED_MACROS

//ERROR MACROS
#define INTERNAL_ERROR 99
#define WRONG_PORT 98
#define WRONG_PROTOCOL 97
#define WRONG_PROTOCOL 97
#define LEXICAL_ERROR 96
#define ERROR_CONNECT 95
#define TRY_AGAIN 2
#define SUCCESS 0

//MACROS
#define MIN_PORT 1024
#define MAX_PORT 65535
#define BUFFER_SIZE 1600

//STATES FOR KA
#define STATE_START 0
#define STATE_AUTH 1
#define STATE_OPEN 2
#define STATE_ERROR 3
#define STATE_END 4

//Macro for checking user input message
#define MAX_TOKENS 15
#define ID 20
#define SECRET 128
#define CONTENT 1400
#define DNAME 20
#define CHANNELID 20


#endif