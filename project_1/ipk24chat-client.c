/**
* @file ipk24chat-client.c
* @brief Impelementation of IPK24-CHAT protocol
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#include "ipk24chat-client.h"

int actual_state;
int can_send;
int my_socket;

struct addrinfo *server_info;

/**
 * @brief Emergency Exit
 * 
 * @param socket
 * @param server_info 
 * @param returnCode 
 * @return end program with passed return code
 */
void emergencyExit(int socket, struct addrinfo *server_info, int returnCode){
    close(socket);
    free(server_info);
    exit(returnCode);
}

/**
 * @brief Handling of C-c
 * 
 */
void handle_sigint(){                        
    int result;							   	 
    char buffer[BUFFER_SIZE] = {0};	
    if(actual_state == STATE_START || actual_state == STATE_END){
        emergencyExit(my_socket, server_info, EXIT_SUCCESS);
    }
    strcpy(buffer, "BYE");			
    strcat(buffer, "\r\n");		   	 
    								  	 
    while((result = send(my_socket, buffer, strlen(buffer),0)) < 0);        
    emergencyExit(my_socket, server_info, EXIT_SUCCESS);
}

/**
 * @brief main function
 * 
 * @param argc
 * @param argv
 * @return end program with passed return code
 */
int main(int argc, char** argv){
    
    struct InputData input_data;
    char buffer[BUFFER_SIZE]; 
    if(argc == 2 || argc >= 12 || argc == 8){
        printHelp();
        exit(SUCCESS);
    }

    input_data = parseArguments(argc, argv);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;       // IPv4
    hints.ai_protocol = 0;           // Protocol

    if(strcmp(input_data.protocol, "tcp") == 0){
        hints.ai_socktype = SOCK_STREAM; // TCP
    }else if(strcmp(input_data.protocol, "udp") == 0){
        connect_udp(input_data.host_name, input_data.port, input_data.retrans, input_data.timeout);
        return 0;
    }
    
    // resolving hostname by using DNS
    int status = getaddrinfo(input_data.host_name, input_data.port, &hints, &server_info);
    if (status != 0 || (server_info)->ai_addr == NULL){
        fprintf(stderr, "ERR: getaddrinfo: failed to resolve hostname!\n");
        emergencyExit(my_socket, server_info, EXIT_FAILURE);
    }

    
    int step = 0;
    if(strcmp(input_data.protocol, "tcp") == 0){
        signal(SIGINT, handle_sigint);
        
        // Creating the socket
        if ((my_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol)) < 0){
            fprintf(stderr, "ERR: socket can not be created\n");
            emergencyExit(my_socket, server_info, EXIT_FAILURE);
        }

        // Connecting to a server
        if (connect(my_socket, server_info->ai_addr, server_info->ai_addrlen) < 0) {
            fprintf(stderr, "ERR: cannot connect to a server\n");
            emergencyExit(my_socket, server_info, EXIT_FAILURE);
        }


        int result;
        bzero(buffer, BUFFER_SIZE);
        fd_set read_fds;    
        
        actual_state = STATE_START;
        can_send = 0; // variable for allowing fgets read from stdin

        // if data come from stdin
        if (isatty(fileno(stdin))){
            while(1){
                FD_ZERO(&read_fds);
                FD_SET(my_socket, &read_fds);
                FD_SET(STDIN_FILENO, &read_fds);
                
                result = select(my_socket + 1, &read_fds, NULL, NULL, NULL);
                if (result < 0) {
                    fprintf(stderr, "ERR: problems with select()\n");
                    emergencyExit(my_socket, server_info, EXIT_FAILURE);
                }

                //RECEIVING MESSAGES FROM SERVER
                if (FD_ISSET(my_socket, &read_fds)) {
                    if(parseMsg(buffer, result, my_socket)){
                        bzero(buffer, BUFFER_SIZE);
                        emergencyExit(my_socket, server_info, EXIT_FAILURE);
                        break;
                    }

                    bzero(buffer, BUFFER_SIZE);
                }

                //SENDING MESSAGES TO SERVER
                if (FD_ISSET(STDIN_FILENO, &read_fds)){
                    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL){
                        break;
                    }

                    if(strlen(buffer) == 1){
                        continue;
                    }
                    
                    if(sendToServer(buffer, result, my_socket)){
                        break;
                    }

                    bzero(buffer, BUFFER_SIZE);
                
                }
            }
            if (feof(stdin)) {
                handle_sigint();
            }
        }
        // if data come from a file
        else{
            while(1){
                FD_ZERO(&read_fds);
                FD_SET(my_socket, &read_fds);
                FD_SET(STDIN_FILENO, &read_fds);
                result = select(my_socket + 1, &read_fds, NULL, NULL, NULL);
                if (result < 0) {
                    fprintf(stderr, "ERR: problems with select()\n");
                    emergencyExit(my_socket, server_info, EXIT_FAILURE);
                    exit(EXIT_FAILURE);
                }
                //RECEIVING MESSAGES FROM SERVER
                if (FD_ISSET(my_socket, &read_fds)) {
                    if(parseMsg(buffer, result, my_socket)){
                        break;
                    }

                    bzero(buffer, BUFFER_SIZE);
                }
                //SENDING MESSAGES TO SERVER
                if (FD_ISSET(STDIN_FILENO, &read_fds) && !can_send){
                    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL){
                        emergencyExit(my_socket, server_info, EXIT_FAILURE);
                        break;
                    }

                    if(strlen(buffer) == 1){
                        continue;
                    }

                    if(sendToServer(buffer, result, my_socket)){
                        break;
                    }

                    bzero(buffer, BUFFER_SIZE);
                }
            }           
        }

        bzero(buffer, BUFFER_SIZE);
        close(my_socket);
    }

    freeaddrinfo(server_info);
    return 0;
}
