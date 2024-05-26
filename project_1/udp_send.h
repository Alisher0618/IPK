/**
* @file udp_send.h
* @brief Impelementation of crafting and sending message to a remote server
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#ifndef UDP_SEND
#define UDP_SEND

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
#include "udp.h"
#include "define_macros.h"


/**
 * @brief Function for crafting the message to a remote server
 * 
 * @param start_buf Buffer for saving the crafted message
 * @param tokens Array of tokens, which represents message splitted by spaces
 * @param num_tokens Number of tokens(words)
 */
void concat_udp_message(char* start_buf, char* tokens[], int num_tokens);

/**
 * @brief Function for checking the buffer/tokens before sending to a remote server
 * 
 * @param tokens Array of tokens, which represents message splitted by spaces
 * @param num_tokens NUmber of tokens(words)
 * @return Return Code, Error, Success or Try_Again(in case of wrong user input)
 */
int check_udp_buffer(char* tokens[], int num_tokens);

/**
 * @brief Function for sending the message to a remote server
 * 
 * @param tokens Array of tokens, which represents message splitted by spaces
 * @param n Number of bytes of the sending message
 * @param num_tokens NUmber of tokens(words)
 * @return Returns Error or Success
 */
int send_message(char* tokens[], int n, int num_tokens);

#endif
