/**
* @file ipk_server.c
* @brief Main function of ipk24chat-server implementation
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#include "ipk_server.h"

volatile sig_atomic_t flag = 0;

#define PORT 4567
#define MAXLINE 1024 
#define MAX_USER 25
#define TIMEOUT_SEC 30
struct UserTCP tcp_users[MAX_USER];
struct UserUDP udp_users[MAX_USER];

fd_set read_fds;
struct sockaddr_in server_addr_tcp, server_addr_udp, client_addr;
int tcp_server_socket, udp_server_socket;


int tcp_clients = 0;

int udp_clients = 0;

/**
 * @brief Emergency Exit
 * 
 * @param socket
 */
void emergencyExit(int socket){
    close(socket);
}

/**
 * @brief Handling of C-c
 * 
 */
void handle_sigint(){                        
    int result;							   	 
    char buffer[BUFFER_SIZE] = {0};	
    strcpy(buffer, "BYE");			
    strcat(buffer, "\r\n");	
    flag = 1;
    if(tcp_clients > 0){
        for (int i = 0; i < tcp_clients; i++){
            if(strcmp(tcp_users[i].state, "accept") == 0){
                emergencyExit(tcp_users[i].socket);
            }else{                                    
                while((result = send(tcp_users[i].socket, buffer, strlen(buffer),0)) < 0);        
                emergencyExit(tcp_users[i].socket);
            }
        }
    }

    close(tcp_server_socket);
    close(udp_server_socket);
    
}

/**
 * @brief Function for sending messages to the users
 * 
 * @param server_buffer received buffer from a client
 * @param type Type of operation
 * @param order_tcp Order of the client
 * @param order_udp Order of the client
 * @param protocol Var for understending which protocol is sending
 */
void send_to_all(char* server_buffer, char* type, int order_tcp, int order_udp, int protocol){ //protocol = 1(tcp), protocol = 2(udp)
    char tcp_buffer[BUFFER_SIZE] = {0};
    char udp_buffer[BUFFER_SIZE] = {0};
    char message_id_char[2] = {0};

    if(strcmp(type, "auth") == 0 || strcmp(type, "bye") == 0){
        //TCP MESSAGE
        strcpy(tcp_buffer, "MSG FROM Server IS ");
        if(protocol == 1){
            strcat(tcp_buffer, tcp_users[order_tcp].display_name);
            if(strcmp(type, "bye") == 0){
                strcat(tcp_buffer, " left ");
                strcat(tcp_buffer, tcp_users[order_tcp].channel_id);
                strcat(tcp_buffer, "\r\n");
            }else{
                strcat(tcp_buffer, " joined default");
                strcat(tcp_buffer, "\r\n");
            }
        }else{
            strcat(tcp_buffer, udp_users[order_udp].display_name);
            if(strcmp(type, "bye") == 0){
                strcat(tcp_buffer, " left ");
                strcat(tcp_buffer, udp_users[order_udp].channel_id);
                strcat(tcp_buffer, "\r\n");
            }else{
                strcat(tcp_buffer, " joined default");
                strcat(tcp_buffer, "\r\n");
            }
        }
        
        //UDP MESSAGE
        message_id_char[0] = udp_users[order_udp].serv_id >> 8;
        message_id_char[1] = udp_users[order_udp].serv_id;
        udp_users[order_udp].serv_id++;

        udp_buffer[0] = '\x04';
        udp_buffer[1] = message_id_char[0];
        udp_buffer[2] = message_id_char[1];
        int sent_size = 3;
        strcpy(udp_buffer + sent_size, "Server");
        sent_size += strlen("Server") + 1;
        if(protocol == 1){
            strcpy(udp_buffer + sent_size, tcp_users[order_tcp].display_name);
            sent_size += strlen(tcp_users[order_tcp].display_name);
            if(strcmp(type, "bye") == 0){
                strcpy(udp_buffer + sent_size, " left ");
                sent_size += strlen(" left ");
                strcpy(udp_buffer + sent_size, tcp_users[order_tcp].channel_id);
                sent_size += strlen(tcp_users[order_tcp].channel_id) + 1;
            }else{
                strcpy(udp_buffer + sent_size, " joined default");
                sent_size += strlen(" joined default") + 1;
            }
        }else{
            strcpy(udp_buffer + sent_size, udp_users[order_udp].display_name);
            sent_size += strlen(udp_users[order_udp].display_name);
            if(strcmp(type, "bye") == 0){
                strcpy(udp_buffer + sent_size, " left ");
                sent_size += strlen(" left ");
                strcpy(udp_buffer + sent_size, udp_users[order_udp].channel_id);
                sent_size += strlen(udp_users[order_udp].channel_id) + 1;
            }else{
                strcpy(udp_buffer + sent_size, " joined default");
                sent_size += strlen(" joined default") + 1;
            }
        }

        
        //SENDING TCP MESSAGE
        int bytes_sent;
        for (int i = 0; i < tcp_clients; i++){
            if(protocol == 1){
                
                if(strcmp(tcp_users[i].port, tcp_users[order_tcp].port) != 0 && strcmp(tcp_users[i].channel_id, tcp_users[order_tcp].channel_id) == 0){
                    bytes_sent = send(tcp_users[i].socket, tcp_buffer, strlen(tcp_buffer), 0);
                    if (bytes_sent == -1) {
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", tcp_users[i].user_ip, tcp_users[i].port);
                    sleep(1);
                }
                
            }
            else{
                printf("SEND to tcp %s = %s\n", tcp_users[i].channel_id, udp_users[order_udp].channel_id);
                if(strcmp(tcp_users[i].channel_id, udp_users[order_udp].channel_id) == 0){
                    bytes_sent = send(tcp_users[i].socket, tcp_buffer, strlen(tcp_buffer), 0);
                    if (bytes_sent == -1) {
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", tcp_users[i].user_ip, tcp_users[i].port);
                    sleep(1);
                } 
            } 
            
            
        }

        //SENDING UDP MESSAGE
        for (int i = 0; i < udp_clients; i++){
            if(protocol == 2){
                if(strcmp(udp_users[i].port, udp_users[order_udp].port) != 0 && strcmp(udp_users[i].channel_id, udp_users[order_udp].channel_id) == 0){
                    bytes_sent = sendto(udp_users[i].socket, udp_buffer, sent_size, 0, (struct sockaddr *)&udp_users[i].address, sizeof(server_addr_udp));
                    if(bytes_sent < 0){
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", udp_users[i].user_ip, udp_users[i].port);
                    sleep(1);
                }
            }else{
                if(strcmp(tcp_users[order_tcp].channel_id, udp_users[i].channel_id) == 0){
                    bytes_sent = sendto(udp_users[i].socket, udp_buffer, sent_size, 0, (struct sockaddr *)&udp_users[i].address, sizeof(server_addr_udp));
                    if(bytes_sent < 0){
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", udp_users[i].user_ip, udp_users[i].port);
                    sleep(1);
                }
            }
            
        }
        
        memset(tcp_buffer, 0, BUFFER_SIZE);
        memset(udp_buffer, 0, BUFFER_SIZE);
    }
    else if(strcmp(type, "msg") == 0){
        //TCP MESSAGE
        strcpy(tcp_buffer, "MSG FROM ");
        if(protocol == 1){
            strcat(tcp_buffer, tcp_users[order_tcp].display_name);
        }else{
            strcat(tcp_buffer, udp_users[order_udp].display_name);
        }
        strcat(tcp_buffer, " IS ");
        strcat(tcp_buffer, server_buffer);
        strcat(tcp_buffer, "\r\n");

        //UDP MESSAGE
        message_id_char[0] = udp_users[order_udp].serv_id >> 8;
        message_id_char[1] = udp_users[order_udp].serv_id;
        udp_users[order_udp].serv_id++;

        udp_buffer[0] = '\x04';
        udp_buffer[1] = message_id_char[0];
        udp_buffer[2] = message_id_char[1];
        int sent_size = 3;
        if(protocol == 1){
            strcpy(udp_buffer + sent_size, tcp_users[order_tcp].display_name);
            sent_size += strlen(tcp_users[order_tcp].display_name) + 1;
        }else{
            strcpy(udp_buffer + sent_size, udp_users[order_udp].display_name);
            sent_size += strlen(udp_users[order_udp].display_name) + 1;
        }
        strcpy(udp_buffer + sent_size, server_buffer);
        sent_size += strlen(server_buffer) + 1;

        //SENDING TCP MESSAGE
        int bytes_sent;
        for (int i = 0; i < tcp_clients; i++){
            if(protocol == 1){
                if(strcmp(tcp_users[i].port, tcp_users[order_tcp].port) != 0 && 
                    strcmp(tcp_users[i].channel_id, tcp_users[order_tcp].channel_id) == 0){
                    bytes_sent = send(tcp_users[i].socket, tcp_buffer, strlen(tcp_buffer), 0);
                    if (bytes_sent == -1) {
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", tcp_users[i].user_ip, tcp_users[i].port);
                    sleep(1);
                }
            }
            else{
                if(strcmp(tcp_users[i].channel_id, udp_users[order_udp].channel_id) == 0){
                    bytes_sent = send(tcp_users[i].socket, tcp_buffer, strlen(tcp_buffer), 0);
                    if (bytes_sent == -1) {
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", tcp_users[i].user_ip, tcp_users[i].port);
                    sleep(1);
                }
            } 
            
            
        }

        //SENDING UDP MESSAGE
        for (int i = 0; i < udp_clients; i++){
            if(protocol == 2){
                if(strcmp(udp_users[i].port, udp_users[order_udp].port) != 0 && strcmp(udp_users[i].channel_id, udp_users[order_udp].channel_id) == 0){
                    bytes_sent = sendto(udp_users[i].socket, udp_buffer, sent_size, 0, (struct sockaddr *)&udp_users[i].address, sizeof(server_addr_udp));
                    if(bytes_sent < 0){
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", udp_users[i].user_ip, udp_users[i].port);
                    sleep(1);
                }
            }
            else{
                if(strcmp(tcp_users[order_tcp].channel_id, udp_users[i].channel_id) == 0){
                    bytes_sent = sendto(udp_users[i].socket, udp_buffer, sent_size, 0, (struct sockaddr *)&udp_users[i].address, sizeof(server_addr_udp));
                    if(bytes_sent < 0){
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", udp_users[i].user_ip, udp_users[i].port);
                    sleep(1);
                }
            }
            
            
        }
        
        memset(tcp_buffer, 0, BUFFER_SIZE);
        memset(udp_buffer, 0, BUFFER_SIZE);
    }
    else if(strcmp(type, "join1") == 0 || strcmp(type, "join2") == 0){
        //TCP MESSAGE
        strcpy(tcp_buffer, "MSG FROM Server IS ");
        if(protocol == 1){
            strcat(tcp_buffer, tcp_users[order_tcp].display_name);
            if(strcmp(type, "join1") == 0){
                strcat(tcp_buffer, " left ");
                strcat(tcp_buffer, tcp_users[order_tcp].channel_id);
                strcat(tcp_buffer, "\r\n");
            }else{
                strcat(tcp_buffer, " joined ");
                strcat(tcp_buffer, tcp_users[order_tcp].channel_id);
                strcat(tcp_buffer, "\r\n");
            }
        }else{
            strcat(tcp_buffer, udp_users[order_udp].display_name);
            if(strcmp(type, "join1") == 0){
                strcat(tcp_buffer, " left ");
                strcat(tcp_buffer, udp_users[order_udp].channel_id);
                strcat(tcp_buffer, "\r\n");
            }else{
                strcat(tcp_buffer, " joined ");
                strcat(tcp_buffer, udp_users[order_udp].channel_id);
                strcat(tcp_buffer, "\r\n");
            }
        }
        
        //UDP MESSAGE
        message_id_char[0] = udp_users[order_udp].serv_id >> 8;
        message_id_char[1] = udp_users[order_udp].serv_id;
        udp_users[order_udp].serv_id++;

        udp_buffer[0] = '\x04';
        udp_buffer[1] = message_id_char[0];
        udp_buffer[2] = message_id_char[1];
        int sent_size = 3;
        strcpy(udp_buffer + sent_size, "Server");
        sent_size += strlen("Server") + 1;
        if(protocol == 1){
            strcpy(udp_buffer + sent_size, tcp_users[order_tcp].display_name);
            sent_size += strlen(tcp_users[order_tcp].display_name);
            if(strcmp(type, "join1") == 0){
                strcpy(udp_buffer + sent_size, " left ");
                sent_size += strlen(" left ");
                strcpy(udp_buffer + sent_size, tcp_users[order_tcp].channel_id);
                sent_size += strlen(tcp_users[order_tcp].channel_id) + 1;
            }else{
                strcpy(udp_buffer + sent_size, " joined ");
                sent_size += strlen(" joined ");
                strcpy(udp_buffer + sent_size, tcp_users[order_tcp].channel_id);
                sent_size += strlen(tcp_users[order_tcp].channel_id) + 1;
            }
        }else{
            strcpy(udp_buffer + sent_size, udp_users[order_udp].display_name);
            sent_size += strlen(udp_users[order_udp].display_name);
            if(strcmp(type, "join1") == 0){
                strcpy(udp_buffer + sent_size, " left ");
                sent_size += strlen(" left ");
                strcpy(udp_buffer + sent_size, udp_users[order_udp].channel_id);
                sent_size += strlen(udp_users[order_udp].channel_id) + 1;
            }else{
                strcpy(udp_buffer + sent_size, " joined ");
                sent_size += strlen(" joined ");
                strcpy(udp_buffer + sent_size, udp_users[order_udp].channel_id);
                sent_size += strlen(udp_users[order_udp].channel_id) + 1;
            }
        }

        //SENDING TCP MESSAGE
        int bytes_sent;
        for (int i = 0; i < tcp_clients; i++){
            if(protocol == 1){
                if(tcp_users[i].port != tcp_users[order_tcp].port && strcmp(tcp_users[i].channel_id, tcp_users[order_tcp].channel_id) == 0){
                    bytes_sent = send(tcp_users[i].socket, tcp_buffer, strlen(tcp_buffer), 0);
                    if (bytes_sent == -1) {
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", tcp_users[i].user_ip, tcp_users[i].port);
                }
            }
            else{
                if(strcmp(tcp_users[i].channel_id, udp_users[order_udp].channel_id) == 0){
                    bytes_sent = send(tcp_users[i].socket, tcp_buffer, strlen(tcp_buffer), 0);
                    if (bytes_sent == -1) {
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", tcp_users[i].user_ip, tcp_users[i].port);
                } 
            } 
            
            sleep(1);
        }

        //SENDING UDP MESSAGE
        for (int i = 0; i < udp_clients; i++){
            if(protocol == 2){
                if(udp_users[i].port != udp_users[order_udp].port && strcmp(udp_users[i].channel_id, udp_users[order_udp].channel_id) == 0){
                    bytes_sent = sendto(udp_users[i].socket, udp_buffer, sent_size, 0, (struct sockaddr *)&udp_users[i].address, sizeof(server_addr_udp));
                    if(bytes_sent < 0){
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", udp_users[i].user_ip, udp_users[i].port);
                }
            }else{
                if(strcmp(tcp_users[order_tcp].channel_id, udp_users[i].channel_id) == 0){
                    bytes_sent = sendto(udp_users[i].socket, udp_buffer, sent_size, 0, (struct sockaddr *)&udp_users[i].address, sizeof(server_addr_udp));
                    if(bytes_sent < 0){
                        perror("Failed to send message");
                    }
                    printf("SENT %s:%s | MSG\n", udp_users[i].user_ip, udp_users[i].port);
                }
            }
            
            sleep(1);
        }
        
        memset(tcp_buffer, 0, BUFFER_SIZE);
        memset(udp_buffer, 0, BUFFER_SIZE);
    }
    
    
}

/**
 * @brief Function for processing TCP 
 * 
 * @param user Struct of tcp users
 * @param order Order of the client
 * @param tokens Array of tokens, which represents message splitted by spaces 
 * @param num_tokens Number of tokens
 * @return Return Code, Error or Success
 */
int tcp_communication(struct UserTCP user[], int order, char *tokens[], int num_tokens){
    char send_buf[BUFFER_SIZE] = {0};
    int bytes_sent;
    if(strcmp(tokens[0], "AUTH") == 0){
        strcpy(user[order].state, "open");
        strcpy(user[order].user_name, tokens[1]);
        strcpy(user[order].display_name, tokens[3]);
        strcpy(user[order].secret, tokens[5]);

        printf("RECV %s:%s | AUTH\n", user[order].user_ip, user[order].port);
        
        // crafting message for user
        strcpy(send_buf, "REPLY OK IS Auth success.");
        strcat(send_buf, "\r\n");

        bytes_sent = send(user[order].socket, send_buf, strlen(send_buf), 0);
        if (bytes_sent == -1) {
            perror("Failed to send message");
        }

        printf("SENT %s:%s | REPLY\n", user[order].user_ip, user[order].port);
        memset(send_buf, 0, BUFFER_SIZE);

        sleep(1);

        strcpy(send_buf, "MSG FROM Server IS ");
        strcat(send_buf, user[order].display_name);
        strcat(send_buf, " joined default");
        strcat(send_buf, "\r\n");
        strcpy(user[order].channel_id, "default");

        bytes_sent = send(user[order].socket, send_buf, strlen(send_buf), 0);
        if (bytes_sent == -1) {
            perror("Failed to send message");
        }

        printf("SENT %s:%s | MSG\n", user[order].user_ip, user[order].port);

        memset(send_buf, 0, BUFFER_SIZE);

        if(tcp_clients > 1 || udp_clients >= 1){
            send_to_all("", "auth", order, -1, 1);
        }


    }
    else if(strcmp(tokens[0], "MSG") == 0){
        char who_send[DNAME] = {0};
        char who_port[10] = {0};
        strcpy(who_send, tokens[2]);
        printf("RECV %s:%s | MSG\n", user[order].user_ip, user[order].port);

        for (int i = 0; i < tcp_clients; i++){
            if(strcmp(user[i].display_name, who_send) == 0){
                strcpy(who_port, user[i].port);
                break;
            }
        }
        
        if(strcmp(tokens[4], "show") == 0){
            fprintf(stdout, "YOUR CURRENT CHANNEL: %s\n", user[order].channel_id);
        }
        
        for(int j = 4; j < num_tokens; j++){
            strcat(send_buf, tokens[j]);
            if(num_tokens - j == 1){
                break;
            }
            strcat(send_buf, " ");
            
        }

        send_to_all(send_buf, "msg", order, -1, 1);

    }
    else if(strcmp(tokens[0], "BYE") == 0){
        fprintf(stdout, "RECV %s:%s | BYE\n", user[order].user_ip, user[order].port);

        if(tcp_clients > 1 || udp_clients >= 1){
            send_to_all("", "bye" , order, -1, 1);
        }

    }
    else if(strcmp(tokens[0], "JOIN") == 0){

        fprintf(stdout, "RECV %s:%s | JOIN\n", user[order].user_ip, user[order].port);

        char new_channel[CHANNELID] = {0};
        strcpy(new_channel, tokens[1]);
        int same_channel = 0;
        strcpy(send_buf, "MSG FROM Server IS ");
        strcat(send_buf, user[order].display_name);
        strcat(send_buf, " left ");
        strcat(send_buf, user[order].channel_id);
        strcat(send_buf, "\r\n");


        send_to_all("", "join1", order, -1, 1);

        memset(send_buf, 0, BUFFER_SIZE);

        strcpy(send_buf, "REPLY OK IS Join success.");
        strcat(send_buf, "\r\n");

        bytes_sent = send(user[order].socket, send_buf, strlen(send_buf), 0);
        if (bytes_sent == -1) {
            perror("Failed to send message");
        }

        fprintf(stdout, "SENT %s:%s | REPLY\n", user[order].user_ip, user[order].port);

        memset(send_buf, 0, BUFFER_SIZE);

        strcpy(user[order].channel_id, new_channel);

        sleep(1);

        strcpy(send_buf, "MSG FROM Server IS ");
        strcat(send_buf, user[order].display_name);
        strcat(send_buf, " joined ");
        strcat(send_buf, user[order].channel_id);
        strcat(send_buf, "\r\n");

        bytes_sent = send(user[order].socket, send_buf, strlen(send_buf), 0);
        if (bytes_sent == -1) {
            perror("Failed to send message");
        }

        fprintf(stdout, "SENT %s:%s | MSG\n", user[order].user_ip, user[order].port);

        send_to_all("", "join2", order, -1, 1);

        memset(send_buf, 0, BUFFER_SIZE);
    }


    return 0;
}

/**
 * @brief Function for printing connected TCP users 
 * 
 * @param user Struct of tcp users
 * @param order Order of the client
 */
void identify_user(struct UserTCP user[], int order){
    char info_buf[BUFFER_SIZE] = {0};
    strcpy(info_buf, "TCP CONNECT");
    strcat(info_buf, ":");
    strcat(info_buf, user[order].port);
    strcat(info_buf, "\n");

    fprintf(stderr, "%d: NEW CONNECTION: %s",order, info_buf);

    memset(info_buf, 0, BUFFER_SIZE);
}


/**
 * @brief Function for processing some edge cases before TCP communication
 * 
 * @param user Struct of tcp users
 * @param input_buffer Received message from client
 * @param order Order of the client
 * @param socket Socket of a client
 * @return Return Code, Error or Success
 */
void connection(struct UserTCP user[], char *input_buffer, int order, int socket){
    char* delimiter = " ";
    char *tokens[10] = {0};
    int num_tokens = 0;

    char *token = strtok(input_buffer, delimiter);

    while (token != NULL && num_tokens < 10) {
        tokens[num_tokens] = token;
        num_tokens++;
        token = strtok(NULL, delimiter);
    }

    if(strcmp(user[order].state, "accept") == 0){
        if(strcmp(tokens[0], "AUTH") == 0){
            if(check_received_message(num_tokens, tokens)){
                strcpy(user[order].state, "auth");
                int bytes_sent = send(user[order].socket, "REPLY NOK IS invalid input data\r\n", strlen("REPLY NOK IS invalid input data\r\n"), 0);
                if (bytes_sent == -1) {
                    perror("Failed to send message");
                }

                fprintf(stdout, "SENT %s:%s | REPLY\n", user[order].user_ip, user[order].port);

                return;
            }
        }else if(strcmp(tokens[0], "BYE") == 0){
            fprintf(stdout, "RECV %s:%s | BYE\n", user[order].user_ip, user[order].port);
            return;
        }
        else{
            fprintf(stdout, "RECV %s:%s | %s\n", user[order].user_ip, user[order].port, tokens[0]);

            int bytes_sent = send(user[order].socket, "ERROR FROM Server IS invalid input data\r\n", strlen("ERROR FROM Server IS invalid input data\r\n"), 0);
            if (bytes_sent == -1) {
                perror("Failed to send message");
            }

            fprintf(stdout, "SENT %s:%s | ERR\n", user[order].user_ip, user[order].port);

            sleep(1);

            bytes_sent = send(user[order].socket, "BYE\r\n", strlen("BYE\r\n"), 0);
            if (bytes_sent == -1) {
                perror("Failed to send message");
            }
            fprintf(stdout, "SENT %s:%s | BYE\n", user[order].user_ip, user[order].port);

            return;
        }
    }else if(strcmp(user[order].state, "auth") == 0){
        if(strcmp(tokens[0], "AUTH") == 0){
            if(check_received_message(num_tokens, tokens)){
                strcpy(user[order].state, "auth");
                int bytes_sent = send(user[order].socket, "REPLY NOK IS invalid input data\r\n", strlen("REPLY NOK IS invalid input data\r\n"), 0);
                if (bytes_sent == -1) {
                    perror("Failed to send message");
                }

                fprintf(stdout, "SENT %s:%s | REPLY\n", user[order].user_ip, user[order].port);

                return;
            }
        }
        else if(strcmp(tokens[0], "BYE") == 0){
            int bytes_sent = send(user[order].socket, "BYE\r\n", strlen("BYE\r\n"), 0);
            if (bytes_sent == -1) {
                perror("Failed to send message");
            }
            fprintf(stdout, "SENT %s:%s | BYE\n", user[order].user_ip, user[order].port);

            return;
        }
        else{
            fprintf(stdout, "RECV %s:%s | %s\n", user[order].user_ip, user[order].port, tokens[0]);

            int bytes_sent = send(user[order].socket, "ERR FROM Server IS invalid input data\r\n", strlen("ERR FROM Server IS invalid input data\r\n"), 0);
            if (bytes_sent == -1) {
                perror("Failed to send message");
            }

            fprintf(stdout, "SENT %s:%s | ERR\n", user[order].user_ip, user[order].port);

            sleep(1);

            bytes_sent = send(user[order].socket, "BYE\r\n", strlen("BYE\r\n"), 0);
            if (bytes_sent == -1) {
                perror("Failed to send message");
            }
            fprintf(stdout, "SENT %s:%s | BYE\n", user[order].user_ip, user[order].port);

            return;
        }
    }else if(strcmp(user[order].state, "open") == 0){
        if(strcmp(tokens[0], "AUTH") == 0){
            fprintf(stdout, "RECV %s:%s | %s\n", user[order].user_ip, user[order].port, tokens[0]);

            int bytes_sent = send(user[order].socket, "ERR FROM Server IS you can't pass auth twice\r\n", strlen("ERR FROM Server IS you can't pass auth twice\r\n"), 0);
            if (bytes_sent == -1) {
                perror("Failed to send message");
            }

            fprintf(stdout, "SENT %s:%s | ERR\n", user[order].user_ip, user[order].port);

            sleep(1);

            bytes_sent = send(user[order].socket, "BYE\r\n", strlen("BYE\r\n"), 0);
            if (bytes_sent == -1) {
                perror("Failed to send message");
            }
            fprintf(stdout, "SENT %s:%s | BYE\n", user[order].user_ip, user[order].port);

            return;
        }
    }
    

    //checking displayname
    if(strcmp(tokens[0], "MSG") == 0){
        if(strcmp(tokens[2], user[order].display_name) != 0 && user[order].socket == socket){
            strcpy(user[order].display_name, tokens[2]);
        }
    }

    //checking username
    if(strcmp(tokens[0], "AUTH") == 0 && tcp_clients > 1){
        for (int i = 0; i < tcp_clients; i++){
            if(strcmp(user[i].user_name, tokens[1]) == 0){
                int bytes_sent = send(user[order].socket, "ERR FROM Server IS This username is already in use\r\n", strlen("ERR FROM Server IS This username is already in use\r\n"), 0);
                if (bytes_sent == -1) {
                    perror("Failed to send message");
                }

                fprintf(stdout, "SENT %s:%s | ERR\n", user[order].user_ip, user[order].port);

                sleep(1);

                bytes_sent = send(user[order].socket, "BYE\r\n", strlen("BYE\r\n"), 0);
                if (bytes_sent == -1) {
                    perror("Failed to send message");
                }
                fprintf(stdout, "SENT %s:%s | BYE\n", user[order].user_ip, user[order].port);

                return;
            }
        }
        
    }

    
    if(tcp_communication(user, order, tokens, num_tokens)){
        close(user[order].socket);
    }
    memset(input_buffer, 0, BUFFER_SIZE);
        
    
}
struct sockaddr_in address;
struct sockaddr_in new_server_addr;

/**
 * @brief Function for processing some edge cases for UDP 
 * 
 * @param order Order of the client
 * @param buffer Received message from a client
 * @return Return Code, Error or Success
 */
int process_udp_message(int order, char* buffer){
    char con_buff[3] = {0};
    char send_err[50] = {0};
    char message_id[2] = {0};
    if(strcmp(udp_users[order].state, "accept") == 0){
        if((buffer[0] == '\x02' || buffer[0] == '\xff') && !parse_message(buffer)){
            return 0;
        }else{
            fprintf(stdout, "RECV %s:%s | UNEXPECTED\n", udp_users[order].user_ip, udp_users[order].port);

            message_id[0] = udp_users[order].serv_id >> 8;
            message_id[1] = udp_users[order].serv_id;
            udp_users[order].serv_id++;

            send_err[0] = '\xfe';
            send_err[1] = message_id[0];
            send_err[2] = message_id[1];
            int sent_size = 3;
            strcpy(send_err + sent_size, "Server");
            sent_size += strlen("Server") + 1;
            strcpy(send_err + sent_size, "Unexpected operation type");
            sent_size += strlen("Unexpected operation type") + 1;

            int bytes_sent = sendto(udp_users[order].socket, send_err, sent_size, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
            if(bytes_sent < 0){
                perror("Failed to send message");
            }

            fprintf(stdout, "SENT %s:%s | ERR\n", udp_users[order].user_ip, udp_users[order].port);

            sleep(1);

            message_id[0] = udp_users[order].serv_id >> 8;
            message_id[1] = udp_users[order].serv_id;
            udp_users[order].serv_id++;

            char send_bye[10] = {0};

            send_bye[0] = '\xff';
            send_bye[1] = message_id[0];
            send_bye[2] = message_id[1];

            bytes_sent = sendto(udp_users[order].socket, send_bye, 3, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
            if(bytes_sent < 0){
                perror("Failed to send message");
            }

            fprintf(stdout, "SENT %s:%s | BYE\n", udp_users[order].user_ip, udp_users[order].port);

            return 1;
        }
    }
    else if(strcmp(udp_users[order].state, "auth") == 0){
        if((buffer[0] == '\x02' || buffer[0] == '\xff') && !parse_message(buffer)){
            return 0;
        }else{
            fprintf(stdout, "RECV %s:%s | UNEXPECTED\n", udp_users[order].user_ip, udp_users[order].port);


            message_id[0] = udp_users[order].serv_id >> 8;
            message_id[1] = udp_users[order].serv_id;
            udp_users[order].serv_id++;

            send_err[0] = '\xfe';
            send_err[1] = message_id[0];
            send_err[2] = message_id[1];
            int sent_size = 3;
            strcpy(send_err + sent_size, "Server");
            sent_size += strlen("Server") + 1;
            strcpy(send_err + sent_size, "Unexpected operation type");
            sent_size += strlen("Unexpected operation type") + 1;

            int bytes_sent = sendto(udp_users[order].socket, send_err, sent_size, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
            if(bytes_sent < 0){
                perror("Failed to send message");
            }

            fprintf(stdout, "SENT %s:%s | ERR\n", udp_users[order].user_ip, udp_users[order].port);

            sleep(1);

            message_id[0] = udp_users[order].serv_id >> 8;
            message_id[1] = udp_users[order].serv_id;
            udp_users[order].serv_id++;

            char send_bye[10] = {0};

            send_bye[0] = '\xff';
            send_bye[1] = message_id[0];
            send_bye[2] = message_id[1];

            bytes_sent = sendto(udp_users[order].socket, send_bye, 3, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
            if(bytes_sent < 0){
                perror("Failed to send message");
            }

            fprintf(stdout, "SENT %s:%s | BYE\n", udp_users[order].user_ip, udp_users[order].port);

            return 1;
        }
    }
    else if(strcmp(udp_users[order].state, "open") == 0){
        if((buffer[0] == '\x02' || buffer[0] == '\x01') || parse_message(buffer)){
            fprintf(stdout, "RECV %s:%s | UNEXPECTED\n", udp_users[order].user_ip, udp_users[order].port);


            message_id[0] = udp_users[order].serv_id >> 8;
            message_id[1] = udp_users[order].serv_id;
            udp_users[order].serv_id++;

            send_err[0] = '\xfe';
            send_err[1] = message_id[0];
            send_err[2] = message_id[1];
            int sent_size = 3;
            strcpy(send_err + sent_size, "Server");
            sent_size += strlen("Server") + 1;
            strcpy(send_err + sent_size, "Unexpected operation type");
            sent_size += strlen("Unexpected operation type") + 1;

            int bytes_sent = sendto(udp_users[order].socket, send_err, sent_size, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
            if(bytes_sent < 0){
                perror("Failed to send message");
            }

            fprintf(stdout, "SENT %s:%s | ERR\n", udp_users[order].user_ip, udp_users[order].port);

            sleep(1);

            message_id[0] = udp_users[order].serv_id >> 8;
            message_id[1] = udp_users[order].serv_id;
            udp_users[order].serv_id++;

            char send_bye[10] = {0};

            send_bye[0] = '\xff';
            send_bye[1] = message_id[0];
            send_bye[2] = message_id[1];

            bytes_sent = sendto(udp_users[order].socket, send_bye, 3, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
            if(bytes_sent < 0){
                perror("Failed to send message");
            }

            fprintf(stdout, "SENT %s:%s | BYE\n", udp_users[order].user_ip, udp_users[order].port);

            return 1;
        }
    }

    memset(send_err, 0, 30);

    return 0;
}

/**
 * @brief Function for processing UDP 
 * 
 * @param udp_server_socket Socket of udp server
 * @param buffer Received message from a client
 * @param port_str Port
 * @return Return Code, Error or Success
 */
int handle_udp_message(int udp_server_socket, char* buffer, char* port_str){
    int order;

    int exists = 0;
    if(udp_clients > 0){
        for (int i = 0; i < udp_clients; i++){
            if(strcmp(udp_users[i].port, port_str) == 0){
                exists = 1;
                order = i;
                break;
            }
        }
        if(exists == 0){
            udp_users[udp_clients].active = 1;
            strcpy(udp_users[udp_clients].user_ip, inet_ntoa(address.sin_addr));
            strcpy(udp_users[udp_clients].port, port_str);
            udp_users[udp_clients].active = 1;
            udp_users[udp_clients].address = address;
            strcpy(udp_users[udp_clients].state, "accept");
            udp_users[udp_clients].order = udp_clients;
            order++;
            udp_clients++;
        }
    }
    else{
        udp_users[udp_clients].active = 1;
        strcpy(udp_users[udp_clients].user_ip, inet_ntoa(address.sin_addr));
        strcpy(udp_users[udp_clients].port, port_str);
        udp_users[udp_clients].address = address;
        udp_users[udp_clients].active = 1;
        strcpy(udp_users[udp_clients].state, "accept");
        udp_users[udp_clients].order = udp_clients;
        order = 0;
        udp_clients++;
    }
    
    char con_buff[3] = {0};
    char info_buf[BUFFER_SIZE] = {0};
    char send_buf[BUFFER_SIZE] = {0};
    char message_id_char[2] = {0};
    udp_users[order].serv_id = 0;

    int step = 0;
    char sender[ID] = {0};

    if(process_udp_message(order, buffer)){
        return 0;
    }


    //checking displayname
    if(buffer[0] == '\x04'){
        char tmp_dname[DNAME] = {0};
        int step = 0;
        for (int i = 3; buffer[i] != '\x00'; i++){
            tmp_dname[step] = buffer[i];
            step++;
        }

        if(strcmp(tmp_dname, udp_users[order].display_name) != 0 && udp_users[order].socket == udp_server_socket){
            strcpy(udp_users[order].display_name, tmp_dname);
        }
    }


    if(buffer[0] == '\x02'){ //auth
        strcpy(udp_users[order].state, "auth");

        fprintf(stdout, "RECV %s:%s | AUTH\n", udp_users[order].user_ip, udp_users[order].port);

        con_buff[0] = '\x00';
        con_buff[1] = buffer[1];
        con_buff[2] = buffer[2];
        int result = sendto(udp_server_socket, con_buff, 3, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
        if(result < 0){
            fprintf(stderr, "ERROR: unable to send reguest\n");
            return EXIT_FAILURE;
        }
        memset(con_buff, 0, 3);
        fprintf(stdout, "SENT %s:%s | CONFIRM\n", udp_users[order].user_ip, udp_users[order].port);

        int new_port = udp_users[order].address.sin_port; 
        udp_users[order].socket = socket(AF_INET, SOCK_DGRAM, 0);

        memcpy(&new_server_addr, &server_addr_udp, sizeof(server_addr_udp));
        new_server_addr.sin_port = htons(new_port);

        if (bind(udp_users[order].socket, (struct sockaddr *)&new_server_addr, sizeof(new_server_addr)) == -1) {
            perror("Error binding new socket");
            exit(EXIT_FAILURE);
        }

        if(parse_message(buffer)){
            message_id_char[0] = udp_users[order].serv_id >> 8;
            message_id_char[1] = udp_users[order].serv_id;
            udp_users[order].serv_id++;

            send_buf[0] = '\x01';
            send_buf[1] = message_id_char[0];
            send_buf[2] = message_id_char[1];
            send_buf[3] = '\x00';
            send_buf[4] = buffer[1];
            send_buf[5] = buffer[2];
            strcpy(send_buf + 6, "Try again. Data are invalid.");
            result = sendto(udp_users[order].socket, send_buf, 35, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return EXIT_FAILURE;
            }
            memset(send_buf, 0, 35);

            fprintf(stdout, "SENT %s:%s | REPLY\n", udp_users[order].user_ip, udp_users[order].port);    
        }
        else{
            strcpy(udp_users[order].state, "open");
            message_id_char[0] = udp_users[order].serv_id >> 8;
            message_id_char[1] = udp_users[order].serv_id;
            udp_users[order].serv_id++;

            send_buf[0] = '\x01';
            send_buf[1] = message_id_char[0];
            send_buf[2] = message_id_char[1];
            send_buf[3] = '\x01';
            send_buf[4] = buffer[1];
            send_buf[5] = buffer[2];
            strcpy(send_buf + 6, "Auth success.");
            result = sendto(udp_users[order].socket, send_buf, 20, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return EXIT_FAILURE;
            }
            memset(send_buf, 0, 20);

            fprintf(stdout, "SENT %s:%s | REPLY\n", udp_users[order].user_ip, udp_users[order].port);


            for (int i = 3; buffer[i] != '\x00'; i++){
                sender[step] = buffer[i];
                step++;
            }
            strcpy(udp_users[order].user_name, sender);

            step = 0;
            memset(sender, 0, ID);

            for (int i = 3 + strlen(udp_users[order].user_name) + 1; buffer[i] != '\x00'; i++){
                sender[step] = buffer[i];
                step++;
            }
            strcpy(udp_users[order].display_name, sender);
            step = 0;
            memset(sender, 0, ID);

            sleep(1);

            message_id_char[0] = udp_users[order].serv_id >> 8;
            message_id_char[1] = udp_users[order].serv_id;
            udp_users[order].serv_id++;

            send_buf[0] = '\x04';
            send_buf[1] = message_id_char[0];
            send_buf[2] = message_id_char[1];
            int sent_size = 3;
            strcpy(send_buf + sent_size, "Server");
            sent_size += strlen("Server") + 1;
            strcpy(send_buf + sent_size, udp_users[order].display_name);
            sent_size += strlen(udp_users[order].display_name);
            strcpy(send_buf + sent_size, " joined default");
            sent_size += strlen(" joined default") + 1;
            result = sendto(udp_users[order].socket, send_buf, sent_size, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return EXIT_FAILURE;
            }
            memset(send_buf, 0, sent_size);

            fprintf(stdout, "SENT %s:%s | MSG\n", udp_users[order].user_ip, udp_users[order].port);
            strcpy(udp_users[order].channel_id, "default");
            
            send_to_all("", "auth", -1, order, 2);
        }
    }
    else if(buffer[0] == '\x00'){ //confirm
        fprintf(stdout, "RECV %s:%s | CONFIRM\n", udp_users[order].user_ip, udp_users[order].port);
    }
    else if(buffer[0] == '\x04'){ //msg
        fprintf(stdout, "RECV %s:%s | MSG\n", udp_users[order].user_ip, udp_users[order].port);

        con_buff[0] = '\x00';
        con_buff[1] = buffer[1];
        con_buff[2] = buffer[2];
        int result = sendto(udp_users[order].socket, con_buff, 3, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
        if(result < 0){
            fprintf(stderr, "ERROR: unable to send reguest\n");
            return EXIT_FAILURE;
        }
        memset(con_buff, 0, 3);
        fprintf(stdout, "SENT %s:%s | CONFIRM\n", udp_users[order].user_ip, udp_users[order].port);

        for (int i = 3 + strlen(udp_users[order].display_name) + 1; buffer[i] != '\x00'; i++){
            sender[step] = buffer[i];
            step++;
        }

        send_to_all(sender, "msg", -1, order, 2);
    }
    else if(buffer[0] == '\xff'){ //bye
        fprintf(stdout, "RECV %s:%s | BYE\n", udp_users[order].user_ip, udp_users[order].port);

        con_buff[0] = '\x00';
        con_buff[1] = buffer[1];
        con_buff[2] = buffer[2];
        int result = sendto(udp_users[order].socket, con_buff, 3, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
        if(result < 0){
            fprintf(stderr, "ERROR: unable to send reguest\n");
            return EXIT_FAILURE;
        }
        memset(con_buff, 0, 3);
        fprintf(stdout, "SENT %s:%s | CONFIRM\n", udp_users[order].user_ip, udp_users[order].port);

        send_to_all("", "bye" , -1, order, 2);
    }else if(buffer[0] == '\x03'){ //join
        fprintf(stdout, "RECV %s:%s | JOIN\n", udp_users[order].user_ip, udp_users[order].port);

        con_buff[0] = '\x00';
        con_buff[1] = buffer[1];
        con_buff[2] = buffer[2];
        int result = sendto(udp_users[order].socket, con_buff, 3, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
        if(result < 0){
            fprintf(stderr, "ERROR: unable to send reguest\n");
            return EXIT_FAILURE;
        }
        memset(con_buff, 0, 3);
        fprintf(stdout, "SENT %s:%s | CONFIRM\n", udp_users[order].user_ip, udp_users[order].port);

        
        send_to_all("", "join1", -1, order, 2);
        
        
        sleep(1);

        message_id_char[0] = udp_users[order].serv_id >> 8;
        message_id_char[1] = udp_users[order].serv_id;
        udp_users[order].serv_id++;

        send_buf[0] = '\x01';
        send_buf[1] = message_id_char[0];
        send_buf[2] = message_id_char[1];
        send_buf[3] = '\x01';
        send_buf[4] = buffer[1];
        send_buf[5] = buffer[2];
        strcpy(send_buf + 6, "Join success.");
        result = sendto(udp_users[order].socket, send_buf, 20, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
        if(result < 0){
            fprintf(stderr, "ERROR: unable to send reguest\n");
            return EXIT_FAILURE;
        }
        memset(send_buf, 0, 20);

        fprintf(stdout, "SENT %s:%s | REPLY\n", udp_users[order].user_ip, udp_users[order].port);

        char new_channel[CHANNELID] = {0};
        for (int i = 3; buffer[i] != '\x00'; i++){
            new_channel[step] = buffer[i];
            step++; 
        }
        
        strcpy(udp_users[order].channel_id, new_channel);

        sleep(1);

        message_id_char[0] = udp_users[order].serv_id >> 8;
        message_id_char[1] = udp_users[order].serv_id;
        udp_users[order].serv_id++;

        send_buf[0] = '\x04';
        send_buf[1] = message_id_char[0];
        send_buf[2] = message_id_char[1];
        int sent_size = 3;
        strcpy(send_buf + sent_size, "Server");
        sent_size += strlen("Server") + 1;
        strcpy(send_buf + sent_size, udp_users[order].display_name);
        sent_size += strlen(udp_users[order].display_name);
        strcpy(send_buf + sent_size, " joined ");
        sent_size += strlen(" joined ");
        strcpy(send_buf + sent_size, udp_users[order].channel_id);
        sent_size += strlen(udp_users[order].channel_id);
        result = sendto(udp_users[order].socket, send_buf, sent_size, 0, (struct sockaddr *)&udp_users[order].address, sizeof(server_addr_udp));
        if(result < 0){
            fprintf(stderr, "ERROR: unable to send reguest\n");
            return EXIT_FAILURE;
        }
        memset(send_buf, 0, sent_size);

        fprintf(stdout, "SENT %s:%s | MSG\n", udp_users[order].user_ip, udp_users[order].port);

        
        send_to_all("", "join2", -1, order, 2);
        
    }

    memset(buffer, 0, BUFFER_SIZE);
    return 0;
}

/**
 * @brief Main function
 * 
 * @param argc
 * @param argv
 * @return End program with passed return code
 */
int main(int argc, char** argv){    
    signal(SIGINT, handle_sigint);
    struct InputData input_data;

    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    if(argc == 2 || argc >= 10 || argc == 6){
        printHelp();
        exit(SUCCESS);
    }

    input_data = parseArguments(argc, argv);

    // Creating server socket for TCP
    if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("ERR: cannot create socket");
        exit(EXIT_FAILURE);
    }

     // Create UDP server socket    
    if ((udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Error in creating UDP socket");
        exit(EXIT_FAILURE);
    }

    // Setting up a server address
    memset(&server_addr_tcp, 0, sizeof(server_addr_tcp));
    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_addr.s_addr = inet_addr(input_data.host_name);
    server_addr_tcp.sin_port = htons((unsigned short)input_data.port);

    memset(&server_addr_udp, 0, sizeof(server_addr_udp));
    server_addr_udp.sin_family = AF_INET;
    server_addr_udp.sin_addr.s_addr = inet_addr(input_data.host_name);
    server_addr_udp.sin_port = htons((unsigned short)input_data.port);

    // Binding server cocket to ip and port
    if (bind(tcp_server_socket, (struct sockaddr *)&server_addr_tcp, sizeof(server_addr_tcp)) == -1) {
        perror("ERR: cannot bind socket");
        exit(EXIT_FAILURE);
    }

    // Bind the UDP server socket
    if (bind(udp_server_socket, (struct sockaddr*)&server_addr_udp, sizeof(server_addr_udp)) == -1) {
        perror("Error in binding UDP socket");
        exit(EXIT_FAILURE);
    }

    // Listening
    if (listen(tcp_server_socket, MAX_USER) == -1) {
        perror("ERR: error while listening to socket");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Server is running. Waiting for connection...\n");

    int num_clients = 0;
    

    int max_fd = tcp_server_socket + 1;
    int max_sd;
    char port_str[6] = {0};
    while (!flag) {
        FD_ZERO(&read_fds);
        FD_SET(tcp_server_socket, &read_fds);
        FD_SET(udp_server_socket, &read_fds);
        max_sd = (tcp_server_socket > udp_server_socket) ? tcp_server_socket : udp_server_socket;

        // Add client sockets to set
        for (int i = 0; i < MAX_USER; i++) {
            if (tcp_users[i].socket > 0) {
                FD_SET(tcp_users[i].socket, &read_fds);
                if (tcp_users[i].socket > max_sd) {
                    max_sd = tcp_users[i].socket;
                }
            }
        }

        // Add client sockets to set
        for (int i = 0; i < udp_clients; i++) {
            if (udp_users[i].socket > 0) {
                FD_SET(udp_users[i].socket, &read_fds);
                if (udp_users[i].socket > max_sd) {
                    max_sd = udp_users[i].socket;
                }
            }
        }

        // Add UDP server socket to set
        if (udp_server_socket > max_sd) {
            max_sd = udp_server_socket;
        }

        fd_set tmp_fds = read_fds;
        if (select(max_sd + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
            perror("ERR: error with select");
            exit(0);
        }

        // Checking new connections
        if (FD_ISSET(tcp_server_socket, &tmp_fds)) {
            client_addr_len = sizeof(client_addr);
            int client_socket = accept(tcp_server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_socket == -1) {
                perror("ERR: cannot accept connection");
                continue;
            }

            if (num_clients < MAX_USER) {
                tcp_users[num_clients++].socket = client_socket;
                fprintf(stdout, "New client has connected. Number of clients: %d\n", num_clients);
                FD_SET(client_socket, &read_fds);
                if (client_socket + 1 > max_fd) {
                    max_fd = client_socket + 1;
                }
                sprintf(port_str, "%hu", ntohs(client_addr.sin_port));
            
                strcpy(tcp_users[tcp_clients].port, port_str);
                strcpy(tcp_users[tcp_clients].user_ip, inet_ntoa(client_addr.sin_addr));
                tcp_users[tcp_clients].order = tcp_clients;
                strcpy(tcp_users[tcp_clients].state, "accept");

                identify_user(tcp_users, tcp_clients);
                tcp_clients++;
            } else {
                fprintf(stdout, "Cannot accept new user\n");
                close(client_socket);
            }
        }

        // Checking connections from clients
        for (int i = 0; i < num_clients; i++) {
            int client_socket = tcp_users[i].socket;
            if (FD_ISSET(client_socket, &tmp_fds)) {
                ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
                if (bytes_received <= 0) {
                    // client disconnected
                    fprintf(stderr, "Client disconnected %s\n", tcp_users[i].port);

                    close(client_socket);
                    FD_CLR(client_socket, &read_fds);
                    tcp_users[i].user_name[0] = '\0';
                    tcp_users[i].display_name[0] = '\0';
                    tcp_users[i].secret[0] = '\0';
                    tcp_users[i].channel_id[0] = '\0';
                    tcp_users[i].order= 0;
                    tcp_users[i].port[0] = '\0';
                    tcp_users[i].socket = 0;
                    for (int j = i; j < num_clients - 1; j++) {
                        tcp_users[j] = tcp_users[j + 1];
                    }
                    num_clients--;
                    tcp_clients--;
                    continue;
                }
                buffer[bytes_received - 2] = '\0';

                connection(tcp_users, buffer, i, client_socket);

            }
        }

        // If UDP socket has activity, handle the message

        if (FD_ISSET(udp_server_socket, &tmp_fds)) {
            char buffer[BUFFER_SIZE] = {0};
            ssize_t bytes_received;
            
            socklen_t address_len = sizeof(address);

            // Receive data from client
            bytes_received = recvfrom(udp_server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&address, &address_len);
            if (bytes_received < 0) {
                perror("Error in receiving data");
                return 1;
            }
            char port_str[6] = {0};
            sprintf(port_str, "%hu", ntohs(address.sin_port));

            if(handle_udp_message(udp_server_socket, buffer, port_str)){
                continue;
            }
            memset(buffer, 0, BUFFER_SIZE);

        }

        for (int i = 0; i < udp_clients; i++){
            if (FD_ISSET(udp_users[i].socket, &tmp_fds)) {
                char buffer[BUFFER_SIZE] = {0};
                ssize_t bytes_received;
                socklen_t address_len = sizeof(new_server_addr);

                // Receive data from client
                bytes_received = recvfrom(udp_users[i].socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&new_server_addr, &address_len);
                if (bytes_received < 0) {
                    perror("Error in receiving data");
                    return 1;
                }
                char port_str[6] = {0};
                sprintf(port_str, "%hu", ntohs(new_server_addr.sin_port));

                if(handle_udp_message(udp_users[i].socket, buffer, port_str)){
                    close(udp_users[i].socket);
                }
                memset(buffer, 0, BUFFER_SIZE);

            }

        }
        

    }

    close(tcp_server_socket);
    return 0;
}
