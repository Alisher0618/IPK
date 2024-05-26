/**
* @file receive_msg.c
* @brief Function for receiving messages from users
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#include "receive_msg.h"

/**
 * @brief Function for processing some edge cases for TCP
 * 
 * @param num_tokens Number of tokens
 * @param token Array of tokens, which represents message splitted by spaces
 * @return Return Code, Error, Success or Try_Again(in case of wrong user input)
 */
int check_received_message(int num_tokens, char *tokens[]){
    char check_buf[CONTENT] = {0};
    int content_length = 0;
    if(strcmp(tokens[0], "AUTH") == 0){
        if(num_tokens != 6){
            return EXIT_FAILURE;
        }

        if(!(strlen(tokens[1]) <= ID && strlen(tokens[5]) <= SECRET && strlen(tokens[3]) <= DNAME)){
            fprintf(stderr, "ERR: Length of received data is not valid.\n");
            return EXIT_FAILURE;
        }

        for (int i = 0; i < num_tokens; i++){
            for (int j = 0; j < strlen(tokens[i]); j++){
                if(i == 1 || i == 5){
                    if(!((tokens[i][j] >= 97 && tokens[i][j] <= 122)||(tokens[i][j] >= 65 && tokens[i][j] <= 90 )||(tokens[i][j] >= 48 && tokens[i][j] <= 57)||tokens[i][j] == 45)){
                        fprintf(stderr, "ERR: ID or SECRET contain forbidden charachters.\n");
                        return EXIT_FAILURE;
                    }
                }else if(i == 3){
                    if(!(tokens[i][j] >= 33 && tokens[i][j] <= 126)){
                        fprintf(stderr, "ERR: DISPLAYNAME contains forbidden charachters.\n");
                        return EXIT_FAILURE;
                    }
                }
            }
        } 
    }
    else if(strcmp(tokens[0], "MSG") == 0){
        for (int i = 4; i < num_tokens; i++){
            content_length += strlen(tokens[i]);
        }

        if(content_length > 1400){
            return EXIT_FAILURE;
        }
        
    }
    else if(strcmp(tokens[0], "JOIN") == 0){
        if(num_tokens != 4){
            return EXIT_FAILURE;
        }

        if(strlen(tokens[1]) > CHANNELID){
            return EXIT_FAILURE;
        }
        
    }
    else if(strcmp(tokens[0], "ERR") == 0){
        for (int i = 3; i < num_tokens; i++){
            content_length += strlen(tokens[i]);
        }

        if(content_length > 1400){
            return EXIT_FAILURE;
        }
        
    }

    return SUCCESS;

}


/**
 * @brief Function for processing some edge cases for UDP
 * 
 * @param buffer Received message from a client
 * @return Return Code, Error, Success or Try_Again(in case of wrong user input)
 */
int parse_message(char *buffer){
    char con_buff[3] = {0};
    char info_buf[BUFFER_SIZE] = {0};
    char send_buf[BUFFER_SIZE] = {0};
    int username = 0, secret = 0, dname = 0, content = 0, channelid = 0;
    if(buffer[0] == '\x02'){
        for (int i = 3; buffer[i] != '\x00'; i++){
            username++;
        }
        if(username >= ID){
            return EXIT_FAILURE;
        }

        for (int i = 3 + username + 1; buffer[i] != '\x00'; i++){
            dname++;
        }
        if(dname >= DNAME){
            return EXIT_FAILURE;
        }

        for (int i = 3 + username + dname + 2; buffer[i] != '\x00'; i++){
            secret++;
        }
        if(secret >= SECRET){
            return EXIT_FAILURE;
        }
    }
    else if(buffer[0] == '\x04'){
        for (int i = 3; buffer[i] != '\x00'; i++){
            dname++;
        }
        if(dname >= DNAME){
            return EXIT_FAILURE;
        }

        for (int i = 3 + dname + 1; buffer[i] != '\x00'; i++){
            content++;
        }
        if(content >= CONTENT){
            return EXIT_FAILURE;
        }
    }
    else if(buffer[0] == '\x03'){
        for (int i = 3; buffer[i] != '\x00'; i++){
            channelid++;
        }
        if(channelid >= CHANNELID){
            return EXIT_FAILURE;
        }

        for (int i = 3 + channelid + 1; buffer[i] != '\x00'; i++){
            dname++;
        }
        if(dname >= DNAME){
            return EXIT_FAILURE;
        }
    }


    return SUCCESS;
}