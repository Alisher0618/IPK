/**
* @file tcp_receive.h
* @brief Impelementation of receiving and parsing messages from a remote server
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#ifndef TCP_RECEIVE
#define TCP_RECEIVE
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
#include "tcp_send.h"
#include "define_macros.h"




/**
 * @brief Function for checking incoming message from a remote server
 * 
 * @param token Array of tokens, which represents message splitted by spaces
 * @param num_token Number of tokens(words)
 * @param my_socket Socket
 * @param new_buffer Buffer for writing and printing info on stderr
 * @return Return Code, Error or Success
 */
int checkIncomingMsg(char* token[], int num_tokens, int my_socket, char* new_buffer);


/**
 * @brief Function for receiving messages from a remote server
 * 
 * @param buffer Buffer for saving the message
 * @param result Amount of receiving data
 * @param my_socket Socket
 * @return Return Code, Error or Success
 */
int parseMsg(char* buffer, int result, int my_socket);


#endif