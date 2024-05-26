/**
* @file tcp_send.h
* @brief Impelementation of crafting and sending message to a remote server
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#ifndef TCP_SEND
#define TCP_SEND

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>

#include "ipk24chat-client.h"
#include "define_macros.h"




/**
 * @brief Function for checking the buffer/tokens before sending to a remote server
 * 
 * @param token Array of tokens, which represents message splitted by spaces
 * @param num_tokens NUmber of tokens(words)
 * @return Return Code, Error, Success or Try_Again(in case of wrong user input)
 */
int checkBuffer(char* token[], int num_tokens);

/**
 * @brief Function for crafting the message to a remote server
 * 
 * @param tokens Array of tokens, which represents message splitted by spaces
 * @param new_buffer Buffer for saving the crafted message
 * @param num_tokens Number of tokens(words)
 */
void concatMessage(char* tokens[], char* new_buffer, int num_tokens);

/**
 * @brief Function for sending the message to a remote server
 * 
 * @param buffer Buffer contains user input message
 * @param result Amount of sending data
 * @param my_socket Socket
 * @return Returns Error or Success
 */
int sendToServer(char* buffer, int result, int my_socket);

#endif