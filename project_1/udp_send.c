/**
* @file udp_send.c
* @brief Impelementation of crafting and sending message to a remote server
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#include "udp_send.h"

extern int my_udp_socket;
extern int udp_state;
extern uint16_t message_id;
extern struct sockaddr_in serv_addr;
extern char myDispName[DNAME];
extern int udp_send;

int send_length = 0;
/**
 * @brief Function for crafting the message to a remote server
 * 
 * @param start_buf Buffer for saving the crafted message
 * @param tokens Array of tokens, which represents message splitted by spaces
 * @param num_tokens Number of tokens(words)
 */
void concat_udp_message(char* start_buf, char* tokens[], int num_tokens){
    char tmp_buf[CONTENT];
    memset(tmp_buf, 0, CONTENT);
    if(strcmp(tokens[0], "/auth") == 0){ //AUTH
        char message_id_char[2];
        memset(message_id_char, 0, 2);
        message_id_char[0] = message_id >> 8; 
        message_id_char[1] = message_id; 
        strcpy(myDispName, tokens[3]);
        start_buf[0] = '\x02'; 
        start_buf[1] = message_id_char[0];
        start_buf[2] = message_id_char[1];
        strcpy(start_buf + 3, tokens[1]);
        send_length += 3 + strlen(tokens[1]) + 1;
        strcpy(start_buf + 3 + strlen(tokens[1]) + 1, tokens[3]);
        send_length += strlen(tokens[3]) + 1;
        strcpy(start_buf + 3 + strlen(tokens[1]) + 1 + strlen(tokens[3]) + 1, tokens[2]);
        send_length += strlen(tokens[2]) + 1;
        
        udp_state = STATE_AUTH;
    }
    else if(strcmp(tokens[0], "/join") == 0){ // JOIN
        char message_id_char[2];
        memset(message_id_char, 0, 2);
        message_id_char[0] = message_id >> 8; 
        message_id_char[1] = message_id; 
        start_buf[0] = '\x03';
        start_buf[1] = message_id_char[0];
        start_buf[2] = message_id_char[1];
        
        strcpy(start_buf + 3, tokens[1]);
        send_length += 3 + strlen(tokens[1]) + 1;
        strcpy(start_buf + 3 + strlen(tokens[1]) + 1, myDispName);
        send_length += strlen(myDispName) + 1;
    }
    else{ // MSG
        char message_id_char[2];
        memset(message_id_char, 0, 2);
        message_id_char[0] = message_id >> 8; 
        message_id_char[1] = message_id; 
        start_buf[0] = '\x04';
        start_buf[1] = message_id_char[0];
        start_buf[2] = message_id_char[1];
        strcpy(start_buf + 3, myDispName);
        send_length += 3 + strlen(myDispName) + 1;
        for(int i = 0; i < num_tokens; i++){
            strcat(tmp_buf, tokens[i]);
            if(num_tokens - i == 1){
                break;
            }
            strcat(tmp_buf, " ");
        }
        
        strcpy(start_buf + 3 + strlen(myDispName) + 1, tmp_buf);
        send_length += strlen(tmp_buf) + 1;
    }
    message_id++;

    memset(tmp_buf, 0, CONTENT);
    printf("%s", tmp_buf);
}

/**
 * @brief Function for checking the buffer/tokens before sending to a remote server
 * 
 * @param tokens Array of tokens, which represents message splitted by spaces
 * @param num_tokens NUmber of tokens(words)
 * @return Return Code, Error, Success or Try_Again(in case of wrong user input)
 */
int check_udp_buffer(char* tokens[], int num_tokens){
    if(tokens[0][0] == '/' && udp_state == STATE_OPEN){
        if(strcmp(tokens[0], "/auth") == 0 && num_tokens == 4){
            fprintf(stderr, "ERR: you have already passed the authentication.\n");
            return TRY_AGAIN;
        }
        else if(strcmp(tokens[0], "/join") == 0 && num_tokens == 2){
            udp_send = 1;
            if(!(strlen(tokens[1]) <= CHANNELID)){
                udp_send = 0;
                fprintf(stderr, "ERR: Length of the Channel is too long. Try again or type /help to get help\n");
                return TRY_AGAIN;
            }
            for (int j = 0; j < strlen(tokens[1]); j++){
                if(!((tokens[1][j] >= 97 && tokens[1][j] <= 122)||(tokens[1][j] >= 65 && tokens[1][j] <= 90 )||(tokens[1][j] >= 48 && tokens[1][j] <= 57)||tokens[1][j] == 45 || tokens[1][j] == 46)){
                    udp_send = 0;
                    fprintf(stderr, "ERR: CHANNELID contains forbidden charachters. Try again.\n");
                    return TRY_AGAIN;
                }
            }
            
        }
        else if(strcmp(tokens[0], "/rename") == 0 && num_tokens == 2){
            if(!(strlen(tokens[1]) <= DNAME)){
                fprintf(stderr, "ERR: Length of the new Display Name is too long. Try again or type /help to get help\n");
                return TRY_AGAIN;
            }
            for (int j = 0; j < strlen(tokens[1]); j++){
                if(!(tokens[1][j] >= 33 && tokens[1][j] <= 126)){
                    fprintf(stderr, "ERR: DISPLAYNAME contains forbidden charachters. Try again.\n");
                    return TRY_AGAIN;
                }
            }
            strcpy(myDispName, tokens[1]);
            return TRY_AGAIN;
        }
        else if(strcmp(tokens[0], "/help") == 0){
            printf("=====================IPK24-CHAT PROTOCOL=====================\n\n");
            printf("/auth {Username} {Secret} {DisplayName}. Pass authentication.\n");
            printf("/join {ChannelID}. Join to a channel or change the channel.\n");
            printf("/rename {DisplayName}. Change display name.\n");
            printf("/help. Printing useful information\n\n");
            printf("=====================IPK24-CHAT PROTOCOL=====================\n");
            return TRY_AGAIN;
        }
        else{
            fprintf(stderr, "ERR: This command does not exists or you try to run it with wrong parameters. Try again.\n");
            return TRY_AGAIN;
        }
    }else if(tokens[0][0] == '/' && (udp_state == STATE_START || udp_state == STATE_AUTH)){
        if(strcmp(tokens[0], "/auth") == 0 && num_tokens == 4){
            if(udp_state == STATE_START){
                udp_send = 1;
                udp_state = STATE_AUTH;
            }
            if(!(strlen(tokens[1]) <= ID && strlen(tokens[2]) <= SECRET && strlen(tokens[3]) <= DNAME)){
                udp_send = 0;
                fprintf(stderr, "ERR: Length of the channel is too long. Try again or type /help to get help\n");
                return TRY_AGAIN;
            }
            for (int i = 0; i < num_tokens; i++){
                for (int j = 0; j < strlen(tokens[i]); j++){
                    if(i == 1 || i == 2){
                        if(!((tokens[i][j] >= 97 && tokens[i][j] <= 122)||(tokens[i][j] >= 65 && tokens[i][j] <= 90 )||(tokens[i][j] >= 48 && tokens[i][j] <= 57)||tokens[i][j] == 45)){
                            udp_send = 0;
                            fprintf(stderr, "ERR: ID or SECRET contain forbidden charachters. Try again.\n");
                            return TRY_AGAIN;
                        }
                    }else if(i == 3){
                        if(!(tokens[i][j] >= 33 && tokens[i][j] <= 126)){
                            udp_send = 0;
                            fprintf(stderr, "ERR: DISPLAYNAME contains forbidden charachters. Try again.\n");
                            return TRY_AGAIN;
                        }
                    }
                }
            }
        }
        else if(strcmp(tokens[0], "/help") == 0){
            printf("=====================IPK24-CHAT PROTOCOL=====================\n\n");
            printf("/auth {Username} {Secret} {DisplayName}. Pass authentication.\n");
            printf("/join {ChannelID}. Join to a channel or change the channel.\n");
            printf("/rename {DisplayName}. Change display name.\n");
            printf("/help. Printing useful information\n\n");
            printf("=====================IPK24-CHAT PROTOCOL=====================\n");
            return TRY_AGAIN;
        
        }else{
            fprintf(stderr,"ERR: You need to pass the authentication first! Try again.\n");
            return TRY_AGAIN;
        }
    }
    else{
        if(udp_state == STATE_START && num_tokens != 4 && strcmp(tokens[0], "/auth") != 0){
            fprintf(stderr,"ERR: You need to pass the authentication first! Try again.\n");
            return TRY_AGAIN;
        }
        if(udp_state == STATE_AUTH && num_tokens != 4 && strcmp(tokens[0], "/auth") != 0){
                fprintf(stderr,"ERR: You need to pass the authentication first! Try again.\n");
                return TRY_AGAIN;
        }
    }

    return SUCCESS;
}

/**
 * @brief Function for sending the message to a remote server
 * 
 * @param tokens Array of tokens, which represents message splitted by spaces
 * @param n Number of bytes of the sending message
 * @param num_tokens NUmber of tokens(words)
 * @return Returns Error or Success
 */
int send_message(char* tokens[], int n, int num_tokens){
    int result;
    char start_buf[BUFFER_SIZE];

    int ret_code = check_udp_buffer(tokens, num_tokens);

    if(ret_code == TRY_AGAIN){
        return SUCCESS;
    }else if(ret_code){
        return LEXICAL_ERROR;
    }

    memset(start_buf, 0, BUFFER_SIZE);
    concat_udp_message(start_buf, tokens, num_tokens);
    result = sendto(my_udp_socket, start_buf, send_length, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(result < 0){
        fprintf(stderr, "ERROR: unable to send reguest\n");
        return INTERNAL_ERROR;
    }
    send_length = 0;

    memset(start_buf, 0, BUFFER_SIZE);
    memset(tokens, 0, BUFFER_SIZE);
    return SUCCESS;
}
