/**
* @file ipk24chat-client.h
* @brief Header file of IPK24-CHAT protocol
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#ifndef CLIENT
#define CLIENT


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

#include "tcp_send.h"
#include "tcp_receive.h"
#include "parse_args.h"
#include "define_macros.h"
#include "udp.h"


// struct for input data from CLI
struct InputData{
    char* protocol;
    char* host_name;
    char* port;
    int timeout;
    int retrans;
};


/**
 * @brief Emergency Exit
 * 
 * @param socket
 * @param server_info 
 * @param returnCode 
 * @return end program with passed return code
 */
void emergencyExit(int socket, struct addrinfo *server_info, int returnCode);


#endif