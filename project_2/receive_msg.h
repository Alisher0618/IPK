/**
* @file receive_msg.h
* @brief Function for receiving messages from users
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#ifndef RECEIVE_MSG
#define RECEIVE_MSG

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
#include <stdint.h>

#include "define_macros.h"
#include "ipk_server.h"

/**
 * @brief Function for processing some edge cases for TCP
 * 
 * @param num_tokens Number of tokens
 * @param token Array of tokens, which represents message splitted by spaces
 * @return Return Code, Error, Success or Try_Again(in case of wrong user input)
 */
int check_received_message(int num_tokens, char *tokens[]);

/**
 * @brief Function for processing some edge cases for UDP
 * 
 * @param buffer Received message from a client
 * @return Return Code, Error, Success or Try_Again(in case of wrong user input)
 */
int parse_message(char *buffer);
#endif