/**
* @file server_udp.h
* @brief Implementation of TCP server
*
* @author Alisher Mazhirinov (xmazhi00)
* https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Stubs/cpp/DemoTcp
*/

#ifndef SERVER_UDP
#define SERVER_UDP

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

#define CHECK_PERROR(condition, message) \
	if (condition)                       \
	{                                    \
		perror(message);                 \
		return EXIT_FAILURE;             \
	}


#define RECEIVE_BUFSIZE 1600


int udp_server(char* port, char* hostname);

#endif