/**
* @file udp.c
* @brief Impelementation of UDP variant of IPK24-CHAT protocol
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#include "udp.h"


int my_udp_socket;
struct sockaddr_in serv_addr;
int udp_state;
uint16_t message_id = 0;
char myDispName[DNAME];
int can_finish_program = 0;
int udp_send;


/**
 * @brief Function for finishing the communication with a remote server.
 *        Funtion sends BYE message to the server and waiting for CONFIRM
 * 
 * @return Return Code
 */
void end_program(){                        
    int result;					
    socklen_t len_recv = sizeof(serv_addr);		   	 
    char buffer[BUFFER_SIZE];	
    memset(buffer, 0, sizeof(BUFFER_SIZE));
	   	 
    char message_id_char[2];
    memset(message_id_char, 0, 2);

    message_id_char[0] = message_id >> 8;
    message_id_char[1] = message_id; 
    buffer[0] = '\xFF';
    buffer[1] = message_id_char[0];
    buffer[2] = message_id_char[1]; 						 
    								  	 
    while(result = sendto(my_udp_socket, buffer, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0);   

    while (1){
        result = recvfrom(my_udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serv_addr, &len_recv);
        if(result < 0){
            fprintf(stderr, "ERR: response not given or lost\n");
            exit(1);
        }
        if(buffer[0] == CONFIRM){
            break;
        }
    }
    exit(SUCCESS);      
}


/**
 * @brief Function for connecting with a remote server
 * 
 * @param hostname Input hostname
 * @param port Input port
 * @param retrans Input number of retransmissions. By default value is 3
 * @param timeout Input timeout. By default value is 250
 * @return No return, but function may terminate programe
 */
void connect_udp(char* hostname, char* port, int retrans, int timeout){
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    signal(SIGINT, end_program);

    // receiving ip address by using DNS

    if ((server = gethostbyname(hostname)) == NULL) {
        fprintf(stderr,"ERR: unknown hostname '%s'\n", hostname);
        exit(1);
    }
    int udp_port = atoi(port);

    // setting the port number and IP address
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(udp_port);

    my_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (my_udp_socket < 0) {
        fprintf(stderr, "ERR: error while opening a socket.\n");
        exit(1);
    }
    uint16_t message_id = 0;
    uint8_t auth = '\x02';
    int result;

    char* delimiter = " ";
    char *tokens[MAX_TOKENS];


    char new_buf[BUFFER_SIZE];
    socklen_t recv_len = sizeof(serv_addr);

    fd_set read_fds;
    memset(buffer, 0, BUFFER_SIZE);
    udp_state = STATE_START;
    udp_send = 0;
    // if data come from stdin
    if (isatty(fileno(stdin))){
        while(1){
            FD_ZERO(&read_fds);
            FD_SET(my_udp_socket, &read_fds);
            FD_SET(fileno(stdin), &read_fds);
            
            result = select(my_udp_socket + 1, &read_fds, NULL, NULL, NULL);
            if (result < 0) {
                fprintf(stderr, "ERR: problems with select()\n");
                exit(EXIT_FAILURE);
            }

            //RECEIVING MESSAGES FROM SERVER
            if (FD_ISSET(my_udp_socket, &read_fds)) {
                result = recvfrom(my_udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serv_addr, &recv_len);
                if(result < 0){
                    fprintf(stderr, "ERR: response not given or lost\n");
                    exit(1);
                }

                if(receive_message(buffer)){
                    break;
                }

                memset(buffer, 0, BUFFER_SIZE);
            }

            //SENDING MESSAGES TO SERVER
            if (FD_ISSET(fileno(stdin), &read_fds)){
                int n = 0;
                while ((buffer[n++] = getchar()) != '\n'){
                    if(buffer[n - 1] == EOF){
                        memset(buffer, 0, BUFFER_SIZE);
                        exit(SUCCESS);
                    }
                }

                int num_tokens = 0;

                if(strlen(buffer) == 1){
                    memset(buffer, 0, BUFFER_SIZE);
                    continue;
                }

                // splitting the received message into the sequence of tokens
                //******************************************************
                buffer[strlen(buffer) - 1] = '\0';
                char *token = strtok(buffer, delimiter);

                
                while (token != NULL && num_tokens < MAX_TOKENS) {
                    tokens[num_tokens] = token;
                    num_tokens++;
                    token = strtok(NULL, delimiter);
                }
                //******************************************************

                if(send_message(tokens, n, num_tokens)){
                    memset(buffer, 0, BUFFER_SIZE);
                    exit(1);
                }
                
                memset(buffer, 0, BUFFER_SIZE);
                
                n = 0;
            }
        }
    }
    // if data come from a file
    else{
        while(1){
            FD_ZERO(&read_fds);
            FD_SET(my_udp_socket, &read_fds);
            FD_SET(fileno(stdin), &read_fds);
            
            result = select(my_udp_socket + 1, &read_fds, NULL, NULL, NULL);
            if (result < 0) {
                fprintf(stderr, "ERR: problems with select()\n");
                exit(EXIT_FAILURE);
            }

            //RECEIVING MESSAGES FROM SERVER
            if (FD_ISSET(my_udp_socket, &read_fds)) {
                result = recvfrom(my_udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serv_addr, &recv_len);
                if(result < 0){
                    fprintf(stderr, "ERR: response not given or lost\n");
                    exit(1);
                }

                if(receive_message(buffer)){
                    break;
                }

                memset(buffer, 0, BUFFER_SIZE);
            }

            //SENDING MESSAGES TO SERVER
            if (FD_ISSET(fileno(stdin), &read_fds) && !udp_send){
                int n = 0;
                while ((buffer[n++] = getchar()) != '\n'){
                    if(buffer[n - 1] == EOF){
                        memset(buffer, 0, BUFFER_SIZE);
                        end_program();
                        exit(SUCCESS);
                    }
                }
                
                int num_tokens = 0;

                if(strlen(buffer) == 1){
                    memset(buffer, 0, BUFFER_SIZE);
                    continue;
                }

                // splitting the received message into the sequence of tokens
                //******************************************************
                buffer[strlen(buffer) - 1] = '\0';
                char *token = strtok(buffer, delimiter);

                while (token != NULL && num_tokens < MAX_TOKENS) {
                    tokens[num_tokens] = token;
                    num_tokens++;
                    token = strtok(NULL, delimiter);
                }
                //******************************************************

                if(send_message(tokens, n, num_tokens)){
                    memset(buffer, 0, BUFFER_SIZE);
                    exit(1);
                }
                
                memset(buffer, 0, BUFFER_SIZE);
                
                n = 0;
            }
        }
    }

}