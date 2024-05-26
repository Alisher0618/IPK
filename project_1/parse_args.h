
/**
* @file parse_args.h
* @brief Parsing of CLI arguments
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#ifndef PARSE_ARGS
#define PARSE_ARGS

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
 * @brief Function for printing useful information
 */
void printHelp();


/**
 * @brief Function for parsing arguments from CLI
 * 
 * @param argc
 * @param argv 
 * @return struct of parse data
 */
struct InputData parseArguments(int argc, char** argv);


#endif