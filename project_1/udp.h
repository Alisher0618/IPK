/**
* @file udp.h
* @brief Impelementation of UDP variant of IPK24-CHAT protocol
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/

#ifndef UDP_PARSE
#define UDP_PARSE

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
#include "udp_send.h"
#include "udp_receive.h"


/**
 * @brief Structure contains hexadecimal representations of all possible MessageTypes
 *        
 */

typedef enum {
    CONFIRM = 0x00, // maybe done
    REPLY   = 0x01, // done
    AUTH    = 0x02, // done
    JOIN    = 0x03,
    MSG     = 0x04, // maybe done
    ERR     = 0xFE,
    BYE     = 0xFF
} MessageType;


#define TIMEOUT_MS 250

/**
 * @brief Function for finishing the communication with a remote server.
 *        Funtion sends BYE message to the server and waiting for CONFIRM
 * 
 * @return Return Code
 */
void end_program();

/**
 * @brief Function for connecting with a remote server
 * 
 * @param hostname Input hostname
 * @param port Input port
 * @param retrans Input number of retransmissions. By default value is 3
 * @param timeout Input timeout. By default value is 250
 * @return No return, but function may terminate programe
 */
void connect_udp(char* hostname, char* port, int retrans, int timeout);


#endif
