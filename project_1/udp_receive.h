/**
* @file udp_receive.h
* @brief Impelementation of receiving and parsing messages from a remote server
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#ifndef UDP_RECEIVE
#define UDP_RECEIVE

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
 * @brief Function for receiving messages from a remote server
 * 
 * @param buffer Buffer for saving the message
 * @return Return Code, Error or Success
 */
int receive_message(char* buffer);

#endif
