/**
* @file tcp_send.c
* @brief Impelementation of crafting and sending message to a remote server
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#include "tcp_send.h"

extern int actual_state;
extern int can_send;
char dispName[DNAME];

/**
 * @brief Function for checking the buffer/tokens before sending to a remote server
 * 
 * @param token Array of tokens, which represents message splitted by spaces
 * @param num_tokens NUmber of tokens(words)
 * @return Return Code, Error, Success or Try_Again(in case of wrong user input)
 */
int checkBuffer(char* token[], int num_tokens){
    if(token[0][0] == '/' && actual_state == STATE_OPEN){
        if(strcmp(token[0], "/auth") == 0 && num_tokens == 4){
            fprintf(stderr, "ERR: You have already passed the authentication.\n");
            return TRY_AGAIN;
        }
        else if(strcmp(token[0], "/join") == 0 && num_tokens == 2){
            can_send = 1;
            if(!(strlen(token[1]) <= CHANNELID)){
                can_send = 0;
                fprintf(stderr, "ERR: Length of the channel is too long. Try /help to get help\n");
                return TRY_AGAIN;
            }
            for (int j = 0; j < strlen(token[1]); j++){
                if(!((token[1][j] >= 97 && token[1][j] <= 122)||(token[1][j] >= 65 && token[1][j] <= 90 )||(token[1][j] >= 48 && token[1][j] <= 57)||token[1][j] == 45 || token[1][j] == 46)){
                    can_send = 0;
                    fprintf(stderr, "ERR: CHANNELID contains forbidden charachters. Try again.\n");
                    return TRY_AGAIN;
                }
            }
            
        }else if(strcmp(token[0], "/rename") == 0 && num_tokens == 2){
            if(!(strlen(token[1]) <= DNAME)){
                fprintf(stderr, "ERR: Length of the Display Name is too long. Try /help to get help\n");
                return TRY_AGAIN;
            }
            for (int j = 0; j < strlen(token[1]); j++){
                if(!(token[1][j] >= 33 && token[1][j] <= 126)){
                    fprintf(stderr, "ERR: DISPLAYNAME contains forbidden charachters. Try again.\n");
                    return TRY_AGAIN;
                }
            }
            strcpy(dispName, token[1]);
            return TRY_AGAIN;
        }else if(strcmp(token[0], "/help") == 0){
            printf("=====================IPK24-CHAT PROTOCOL=====================\n\n");
            printf("/auth {Username} {Secret} {DisplayName}. Pass authentication.\n");
            printf("/join {ChannelID}. Join to a channel or change the channel.\n");
            printf("/rename {DisplayName}. Change display name.\n");
            printf("/help. Printing useful information\n\n");
            printf("=====================IPK24-CHAT PROTOCOL=====================\n");
            return TRY_AGAIN;
        
        }else{
            fprintf(stderr, "ERR: This command does not exists or you try to run it with wrong parameters. Try again.\n");
            return TRY_AGAIN;
        }
    }
    else if(token[0][0] == '/' && (actual_state == STATE_START || actual_state == STATE_AUTH)){
        if(strcmp(token[0], "/auth") == 0 && num_tokens == 4){
            if(actual_state == STATE_START){
                can_send = 1;
                actual_state = STATE_AUTH;
            }
            if(!(strlen(token[1]) <= ID && strlen(token[2]) <= SECRET && strlen(token[3]) <= DNAME)){
                can_send = 0;
                fprintf(stderr, "ERR: You passed wrong arguments. Try /help to get help\n");
                return TRY_AGAIN;
            }
            for (int i = 0; i < num_tokens; i++){
                for (int j = 0; j < strlen(token[i]); j++){
                    if(i == 1 || i == 2){
                        if(!((token[i][j] >= 97 && token[i][j] <= 122)||(token[i][j] >= 65 && token[i][j] <= 90 )||(token[i][j] >= 48 && token[i][j] <= 57)||token[i][j] == 45)){
                            can_send = 0;
                            fprintf(stderr, "ERR: ID or SECRET contain forbidden charachters. Try again.\n");
                            return TRY_AGAIN;
                        }
                    }else if(i == 3){
                        if(!(token[i][j] >= 33 && token[i][j] <= 126)){
                            can_send = 0;
                            fprintf(stderr, "ERR: DISPLAYNAME contains forbidden charachters. Try again.\n");
                            return TRY_AGAIN;
                        }
                    }
                }
            }    
        }
        else if(strcmp(token[0], "/help") == 0){
            printf("=====================IPK24-CHAT PROTOCOL=====================\n\n");
            printf("/auth {Username} {Secret} {DisplayName}. Pass authentication.\n");
            printf("/join {ChannelID}. Join to a channel or change the channel.\n");
            printf("/rename {DisplayName}. Change display name.\n");
            printf("/help. Printing useful information\n\n");
            printf("=====================IPK24-CHAT PROTOCOL=====================\n");
            return TRY_AGAIN;
        }
        else{
            fprintf(stderr, "ERR: You need to pass the authentication first! Try again.\n");
            return TRY_AGAIN;
        }
    }
    else{
        if(actual_state == STATE_START && num_tokens != 4 && strcmp(token[0], "/auth") != 0){
            fprintf(stderr, "ERR: You need to pass the authentication first! Try again.\n");
            return TRY_AGAIN;
        }
        if(actual_state == STATE_AUTH && num_tokens != 4 && strcmp(token[0], "/auth") != 0){
                fprintf(stderr, "ERR: You need to pass the authentication first! Try again.\n");
                return TRY_AGAIN;
        }
    }

    return SUCCESS;
}

/**
 * @brief Function for crafting the message to a remote server
 * 
 * @param tokens Array of tokens, which represents message splitted by spaces
 * @param new_buffer Buffer for saving the crafted message
 * @param num_tokens Number of tokens(words)
 */
void concatMessage(char* tokens[], char* new_buffer, int num_tokens){
    if(strcmp(tokens[0], "/auth") == 0){
        strcpy(dispName, tokens[3]);

        strcpy(new_buffer, "AUTH ");
        strcat(new_buffer, tokens[1]);

        strcat(new_buffer, " AS ");
        strcat(new_buffer, tokens[3]);

        strcat(new_buffer, " USING ");
        strcat(new_buffer, tokens[2]); 
        
        strcat(new_buffer, "\r\n"); 
    }
    else if(strcmp(tokens[0], "/join") == 0){
        strcpy(new_buffer, "JOIN ");
        strcat(new_buffer, tokens[1]);

        strcat(new_buffer, " AS ");
        strcat(new_buffer, dispName); 
        
        strcat(new_buffer, "\r\n"); 
    }
    else{
        strcpy(new_buffer, "MSG FROM ");
        strcat(new_buffer, dispName);
        strcat(new_buffer, " IS");
        
        for(int i = 0; i < num_tokens; i++){
            strcat(new_buffer, " ");
            strcat(new_buffer, tokens[i]);
        }

        strcat(new_buffer, "\r\n"); 
    }
}

/**
 * @brief Function for sending the message to a remote server
 * 
 * @param buffer Buffer contains user input message
 * @param result Amount of sending data
 * @param my_socket Socket
 * @return Returns Error or Success
 */
int sendToServer(char* buffer, int result, int my_socket){  

    // splitting the received message into the sequence of tokens
    //********************************************************
    char* delimiter = " ";
    char *tokens[MAX_TOKENS];
    int num_tokens = 0;

    buffer[strlen(buffer) - 1] = ' ';

    char *token = strtok(buffer, delimiter);

    while (token != NULL && num_tokens < MAX_TOKENS) {
        tokens[num_tokens] = token;
        num_tokens++;
        token = strtok(NULL, delimiter);
    }
    //********************************************************
    
    int ret_code = checkBuffer(tokens, num_tokens);

    if(ret_code == TRY_AGAIN){
        bzero(buffer, BUFFER_SIZE);
        return EXIT_SUCCESS;
    }else if(ret_code){
        return LEXICAL_ERROR;
    }

    char new_buffer[BUFFER_SIZE];
    
    concatMessage(tokens, new_buffer, num_tokens);
    
    result = send(my_socket, new_buffer, strlen(new_buffer), 0);
    
    if (result < 0) {
        perror("ERROR in send");
        exit(EXIT_FAILURE);
    }

    bzero(buffer, BUFFER_SIZE);
    bzero(new_buffer, BUFFER_SIZE);
    return EXIT_SUCCESS;
}
