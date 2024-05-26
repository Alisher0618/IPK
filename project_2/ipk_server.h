/**
* @file ipk_server.h
* @brief Main function of ipk24chat-server implementation
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#ifndef IPK_SERVER
#define IPK_SERVER

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
#include <time.h>
#include <sys/time.h>

#include "define_macros.h"
#include "receive_msg.h"
#include "parse_args.h"

#define CHECK_PERROR(condition, message) \
	if (condition)                       \
	{                                    \
		perror(message);                 \
		return EXIT_FAILURE;             \
	}

// struct for input data from CLI
struct InputData{
    char* host_name;
    int port;
    int timeout;
    int retrans;
};


struct UserTCP{
    char user_name[ID];
    char display_name[DNAME];
    char secret[SECRET];
    char channel_id[CHANNELID];

    char state[10];
    char user_ip[16];
    int order;
    char port[10];
    int socket;
};

struct UserUDP{
    char user_name[ID];
    char display_name[DNAME];
    char secret[SECRET];
    char channel_id[CHANNELID];
    int serv_id;
    char state[10];
    int order;
    //time_t last_seen;
    int active;
    char user_ip[16];
    char port[10];
    int socket;
    struct sockaddr_in address;    
};


/**
 * @brief Emergency Exit
 * 
 * @param socket
 */
void emergencyExit(int socket);

/**
 * @brief Handling of C-c
 * 
 */
void handle_sigint();

/**
 * @brief Function for sending messages to the users
 * 
 * @param server_buffer received buffer from a client
 * @param type Type of operation
 * @param order_tcp Order of the client
 * @param order_udp Order of the client
 * @param protocol Var for understending which protocol is sending
 */
void send_to_all(char* server_buffer, char* type, int order_tcp, int order_udp, int protocol);


/**
 * @brief Function for processing TCP 
 * 
 * @param user Struct of tcp users
 * @param order Order of the client
 * @param tokens Array of tokens, which represents message splitted by spaces 
 * @param num_tokens Number of tokens
 * @return Return Code, Error or Success
 */
int tcp_communication(struct UserTCP user[], int order, char *tokens[], int num_tokens);


/**
 * @brief Function for printing connected TCP users 
 * 
 * @param user Struct of tcp users
 * @param order Order of the client
 */
void identify_user(struct UserTCP user[], int order);


/**
 * @brief Function for processing some edge cases before TCP communication
 * 
 * @param user Struct of tcp users
 * @param input_buffer Received message from client
 * @param order Order of the client
 * @param socket Socket of a client
 * @return Return Code, Error or Success
 */
void connection(struct UserTCP user[], char *input_buffer, int order, int socket);


/**
 * @brief Function for processing some edge cases for UDP 
 * 
 * @param order Order of the client
 * @param buffer Received message from a client
 * @return Return Code, Error or Success
 */
int process_udp_message(int order, char* buffer);



/**
 * @brief Function for processing UDP 
 * 
 * @param udp_server_socket Socket of udp server
 * @param buffer Received message from a client
 * @param port_str Port
 * @return Return Code, Error or Success
 */
int handle_udp_message(int udp_server_socket, char* buffer, char* port_str);

#endif