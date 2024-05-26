/**
* @file tcp_receive.c
* @brief Impelementation of receiving and parsing messages from a remote server
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#include "tcp_receive.h"

extern int actual_state;
extern int can_send;
extern char dispName[DNAME];

/**
 * @brief Function for checking incoming message from the remote server
 * 
 * @param token Array of tokens, which represents message splitted by spaces
 * @param num_token Number of tokens(words)
 * @param my_socket Socket
 * @param new_buffer Buffer for writing and printing info on stderr
 * @return Return Code, Error or Success
 */
int checkIncomingMsg(char* token[], int num_tokens, int my_socket, char* new_buffer){
    int send_res;
    char send_buf[BUFFER_SIZE] = {0};
    if(actual_state == STATE_AUTH){
        if(strcasecmp(token[0], "REPLY") == 0){
            if(strcasecmp(token[1], "OK") == 0){
                strcpy(new_buffer, "Success:");
                for (int i = 3; i < num_tokens; i++){
                    strcat(new_buffer, " ");
                    strcat(new_buffer, token[i]);
                }
                fprintf(stderr, "%s", new_buffer);
                bzero(new_buffer, BUFFER_SIZE);
                can_send = 0;
                actual_state = STATE_OPEN;
            }else if(strcasecmp(token[1], "NOK") == 0){
                can_send = 0;
                strcpy(new_buffer, "Failure:");
                for(int i = 3; i < num_tokens; i++){
                    strcat(new_buffer, " ");
                    strcat(new_buffer, token[i]);
                }
                fprintf(stderr, "%s", new_buffer);
                bzero(new_buffer, BUFFER_SIZE);
                actual_state = STATE_AUTH;
            }
        }
        else if(strcasecmp(token[0], "ERR") == 0){
            strcpy(send_buf, "BYE");
            strcat(send_buf, "\r\n"); 
            send_res = send(my_socket, send_buf, strlen(send_buf), 0);
    
            if (send_res < 0) {
                fprintf(stderr, "ERR: cannot send\n");
                return (INTERNAL_ERROR);
            }
            bzero(send_buf, 10);

            strcpy(new_buffer, "ERR FROM ");
            strcat(new_buffer, token[2]);
            strcat(new_buffer, ":");
            
            for(int i = 4; i < num_tokens; i++){
                strcat(new_buffer, " ");
                strcat(new_buffer, token[i]);
            }
            
            fprintf(stderr, "%s",new_buffer);
            return EXIT_FAILURE;
        }
        else{
            actual_state = STATE_ERROR;
            strcpy(new_buffer, "ERR FROM ");
            strcat(new_buffer, dispName);
            strcat(new_buffer, "IS");
            strcat(new_buffer, "Unexpected identifiers");
            strcat(new_buffer, "\r\n");
            
            send_res = send(my_socket, new_buffer, strlen(new_buffer), 0);
            if (send_res < 0) {
                fprintf(stderr, "ERR: cannot send\n");
                return (INTERNAL_ERROR);
            }

            sleep(1);

            strcpy(send_buf, "BYE");
            strcat(send_buf, "\r\n"); 
            
            send_res = send(my_socket, send_buf, strlen(send_buf), 0);
            if (send_res < 0) {
                fprintf(stderr, "ERR: cannot send\n");
                return (INTERNAL_ERROR);
            }
            bzero(send_buf, 10);
            return EXIT_FAILURE;
            
        } 
    }
    else if(actual_state == STATE_OPEN){
        if(strcasecmp(token[0], "REPLY") == 0){
            if(strcasecmp(token[1], "OK") == 0){
                strcpy(new_buffer, "Success:");
                for (int i = 3; i < num_tokens; i++){
                    strcat(new_buffer, " ");
                    strcat(new_buffer, token[i]);
                }
                fprintf(stderr, "%s", new_buffer);
                bzero(new_buffer, BUFFER_SIZE);
                can_send = 0;
                actual_state = STATE_OPEN;
            }else if(strcasecmp(token[1], "NOK") == 0){
                can_send = 0;
                strcpy(new_buffer, "Failure:");
                for(int i = 3; i < num_tokens; i++){
                    strcat(new_buffer, " ");
                    strcat(new_buffer, token[i]);
                }
                fprintf(stderr, "%s", new_buffer);
                bzero(new_buffer, BUFFER_SIZE);
                actual_state = STATE_OPEN;
            } 
        }
        else if(strcasecmp(token[0], "MSG") == 0 && strcasecmp(token[2], "Server") == 0 ){
            if(strcasecmp(token[5], "joined") == 0 || strcasecmp(token[5], "left") == 0){
                strcpy(new_buffer, token[2]);
                strcat(new_buffer, ":");
                for(int i = 4; i < num_tokens; i++){
                    strcat(new_buffer, " ");
                    strcat(new_buffer, token[i]);
                }
                
                fprintf(stdout, "%s", new_buffer);

                bzero(new_buffer, BUFFER_SIZE);

                actual_state = STATE_OPEN;
            }
        }
        else if(strcasecmp(token[0], "MSG") == 0){
            strcpy(new_buffer, token[2]);
            strcat(new_buffer, ":");
            for(int i = 4; i < num_tokens; i++){
                strcat(new_buffer, " ");
                strcat(new_buffer, token[i]);
            }
            fprintf(stdout, "%s", new_buffer);

            bzero(new_buffer, BUFFER_SIZE);
            actual_state = STATE_OPEN;
        }
        else if(strcasecmp(token[0], "ERR") == 0){
            strcpy(send_buf, "BYE");
            strcat(send_buf, "\r\n"); 
            send_res = send(my_socket, send_buf, strlen(send_buf), 0);
    
            if (send_res < 0) {
                fprintf(stderr, "ERR: cannot send\n");
                return (INTERNAL_ERROR);
            }
            bzero(send_buf, 10);

            strcpy(new_buffer, "ERR FROM ");
            strcat(new_buffer, token[2]);
            strcat(new_buffer, ":");
            
            for(int i = 4; i < num_tokens; i++){
                strcat(new_buffer, " ");
                strcat(new_buffer, token[i]);
            }

            fprintf(stderr, "%s",new_buffer);
            return EXIT_FAILURE;
        }
        else if((strcasecmp(token[0], "BYE") == 0)){
            return EXIT_SUCCESS;
        }
        else{
            actual_state = STATE_ERROR;
            strcpy(new_buffer, "ERR FROM ");
            strcat(new_buffer, dispName);
            strcat(new_buffer, " IS ");
            strcat(new_buffer, "Unexpected identifiers");
            strcat(new_buffer, "\r\n");
            
            send_res = send(my_socket, new_buffer, strlen(new_buffer), 0);
            if (send_res < 0) {
                fprintf(stderr, "ERR: cannot send\n");
                return (INTERNAL_ERROR);
            }
            bzero(new_buffer, strlen(new_buffer));

            strcpy(new_buffer, "ERR: ");
            strcat(new_buffer, "Unexpected identifiers");
            strcat(new_buffer, "\n");

            fprintf(stderr, "%s", new_buffer);

            sleep(1);

            strcpy(send_buf, "BYE");
            strcat(send_buf, "\r\n"); 
            
            send_res = send(my_socket, send_buf, strlen(send_buf), 0);
            if (send_res < 0) {
                fprintf(stderr, "ERR: cannot send\n");
                return (INTERNAL_ERROR);
            }
            bzero(send_buf, 10);
            return EXIT_FAILURE;
            
        }
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Function for receiving messages from the remote server
 * 
 * @param buffer Buffer for saving the message
 * @param result Amount of receiving data
 * @param my_socket Socket
 * @return Return Code, Error or Success
 */
int parseMsg(char* buffer, int result, int my_socket){
    result = recv(my_socket, buffer, BUFFER_SIZE, 0);
    if (result < 0) {
        fprintf(stderr, "ERR: cannot receive\n");
        return EXIT_FAILURE;
    } else if (result == 0) {
        fprintf(stderr, "ERR: Connection closed by server\n");
        return INTERNAL_ERROR;
    }
    buffer[result - 2] = '\n';
    buffer[result - 1] = '\0';

    // splitting the received message into the sequence of tokens
    //******************************************************
    char* delimiter = " ";                                 
    char *tokens[MAX_TOKENS];                              
    int num_tokens = 0;                                      
    char *token = strtok(buffer, delimiter);               
                                                           
    while (token != NULL && num_tokens < MAX_TOKENS) {     
        tokens[num_tokens] = token;                        
        num_tokens++;                                       
        token = strtok(NULL, delimiter);                   
    }                                                      
    //********************************************************

    char new_buffer[BUFFER_SIZE];
    
    if(checkIncomingMsg(tokens, num_tokens, my_socket, new_buffer)){
        return LEXICAL_ERROR;
    }

    bzero(buffer, BUFFER_SIZE);
    
    return EXIT_SUCCESS;
}