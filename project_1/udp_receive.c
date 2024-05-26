/**
* @file udp_receive.c
* @brief Impelementation of receiving and parsing messages from a remote server
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#include "udp_receive.h"

extern int my_udp_socket;
extern int udp_state;
extern uint16_t message_id;
extern struct sockaddr_in serv_addr;
extern char myDispName[DNAME];
extern int udp_send;

/**
 * @brief Function for receiving messages from a remote server
 * 
 * @param buffer Buffer for saving the message
 * @return Return Code, Error or Success
 */
int receive_message(char* buffer){
    int step = 0;

    char confirm_buf[3];
    memset(confirm_buf, 0, 3);

    char bye_buf[3];
    memset(bye_buf, 0, 3);

    char sender[DNAME];
    memset(sender, 0, DNAME);

    char display_name[DNAME];
    memset(display_name, 0, DNAME);

    char message_id_char[2];
    memset(message_id_char, 0, 2);

    char print_stderr[BUFFER_SIZE];
    memset(print_stderr, 0, BUFFER_SIZE);

    char tmp_buf[BUFFER_SIZE];
    memset(tmp_buf, 0, BUFFER_SIZE);

    message_id_char[0] = message_id >> 8;
    message_id_char[1] = message_id; 
    
    int result;
    MessageType msg_type = buffer[0];
    socklen_t recv_len = sizeof(serv_addr);
    if(udp_state == STATE_AUTH){
        if(msg_type == REPLY){ //REPLY
            confirm_buf[0] = '\x00';
            confirm_buf[1] = buffer[1];
            confirm_buf[2] = buffer[2];
            if(buffer[3] == '\x01'){
                strcpy(print_stderr, "Success: ");
                for (int i = 6; buffer[i] != '\x00'; i++){
                    tmp_buf[step] = buffer[i];
                    step++;
                }
                step = 0;
                strcat(print_stderr, tmp_buf);
                fprintf(stderr, "%s\n", print_stderr);
                udp_send = 0;
                udp_state = STATE_OPEN;

                result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
                if(result < 0){
                    fprintf(stderr, "ERROR: unable to send reguest\n");
                    return INTERNAL_ERROR;
                }

            }else if(buffer[3] == '\x00'){
                udp_send = 0;
                strcpy(print_stderr, "Failure: ");
                for (int i = 6; buffer[i] != '\x00'; i++){
                    tmp_buf[step] = buffer[i];
                    step++;
                }
                step = 0;
                strcat(print_stderr, tmp_buf);
                fprintf(stderr, "%s\n", print_stderr);

                udp_state == STATE_AUTH;

                result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
                if(result < 0){
                    fprintf(stderr, "ERROR: unable to send reguest\n");
                    return INTERNAL_ERROR;
                }
            }

            memset(confirm_buf, 0, 3);
        }
        else if(msg_type == '\xfe' || msg_type == '\xFE'){ // ERR
            int step = 0;

            char err_buf[CONTENT];
            memset(err_buf, 0, CONTENT);

            char content_buf[CONTENT];
            memset(content_buf, 0, CONTENT);

            confirm_buf[0] = CONFIRM;
            confirm_buf[1] = buffer[1];
            confirm_buf[2] = buffer[2];
            
            result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }

            memset(confirm_buf, 0, 3);

            //receiving senders name
            for(int i = 3; buffer[i] != '\x00'; i++){
                sender[step] = buffer[i];
                step++;
            }
            step = 0;

            strcpy(err_buf, "ERR FROM ");
            strcat(err_buf, sender);
            strcat(err_buf, ": ");
            for(int i = 4 + strlen(sender); buffer[i] != '\x00'; i++){
                content_buf[step] = buffer[i];
                step++;
            }  
            step = 0;
            strcat(err_buf, content_buf);
            strcat(err_buf, "\n");
            fprintf(stderr, "%s", err_buf);

            bye_buf[0] = '\xFF';
            bye_buf[1] = message_id_char[0];
            bye_buf[2] = message_id_char[1];
 
            result = sendto(my_udp_socket, bye_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }

            memset(bye_buf, 0, 3);
        }
        else if(msg_type == CONFIRM){ // CONFIRM
            return SUCCESS;
        }
        else{ // Unexpected identifiers
            char send_err[BUFFER_SIZE];
            memset(send_err, 0, BUFFER_SIZE);
            int err_length = 0;

            confirm_buf[0] = '\x00';
            confirm_buf[1] = buffer[1];
            confirm_buf[2] = buffer[2];
            
            result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }
            
            send_err[0] = '\xFE';
            send_err[1] = message_id_char[0];
            send_err[2] = message_id_char[1];
            strcpy(send_err + 3, myDispName);   
            err_length += 3 + strlen(myDispName) + 1;
            strcpy(send_err + 3 + strlen(myDispName) + 1, "Unexpected identifiers");
            err_length += strlen("Unexpected identifiers") + 1;

            fprintf(stderr, "ERR: Unexpected identifiers\n");

            result = sendto(my_udp_socket, send_err, err_length, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }


            message_id++;
            message_id_char[0] = message_id >> 8;
            message_id_char[1] = message_id; 

            memset(send_err, 0, err_length);

            bye_buf[0] = '\xFF';
            bye_buf[1] = message_id_char[0];
            bye_buf[2] = message_id_char[1];

            result = sendto(my_udp_socket, bye_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }

            memset(bye_buf, 0, 3);
            return EXIT_FAILURE;
            
        }
    }
    else if(udp_state = STATE_OPEN){
        if(msg_type == REPLY){
            confirm_buf[0] = '\x00';
            confirm_buf[1] = buffer[1];
            confirm_buf[2] = buffer[2];

            if(buffer[3] == '\x01'){ // REPLY OK
                udp_send = 0;
                strcpy(print_stderr, "Success: ");
                for (int i = 6; buffer[i] != '\x00'; i++){
                    tmp_buf[step] = buffer[i];
                    step++;
                }
                step = 0;
                strcat(print_stderr, tmp_buf);
                fprintf(stderr, "%s\n", print_stderr);

                udp_state = STATE_OPEN;

                result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
                if(result < 0){
                    fprintf(stderr, "ERROR: unable to send reguest\n");
                    return INTERNAL_ERROR;
                }

            }else if(buffer[3] == '\x00'){ // REPLY NOK
                udp_send = 0;
                strcpy(print_stderr, "Failure: ");
                for (int i = 6; buffer[i] != '\x00'; i++){
                    tmp_buf[step] = buffer[i];
                    step++;
                }
                step = 0;
                strcat(print_stderr, tmp_buf);
                fprintf(stderr, "%s\n", print_stderr);
                udp_state == STATE_AUTH;

                result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
                if(result < 0){
                    fprintf(stderr, "ERROR: unable to send reguest\n");
                    return INTERNAL_ERROR;
                }
            }
        }
        else if(msg_type == MSG){ //MSG
            int step = 0;
            confirm_buf[0] = '\x00';
            confirm_buf[1] = buffer[1];
            confirm_buf[2] = buffer[2];

            // SENDING CONFIRM
            result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }

            memset(confirm_buf, 0, 3);
            

            bzero(sender, DNAME);
            for(int i = 3; buffer[i] != '\x00'; i++){
                sender[step] = buffer[i];
                step++;
            }
            step = 0;
            if(strcasecmp(sender, "Server") == 0){
                for(int i = 4 + strlen(sender); buffer[i] != '\x20'; i++){
                    display_name[step] = buffer[i];
                    step++;
                }
                step = 0;
                strcpy(print_stderr ,"Server: ");
                for(int i = 4 + strlen(sender); buffer[i] != '\x00'; i++){
                    tmp_buf[step] = buffer[i];
                    step++;
                }   
                step = 0;
                strcat(print_stderr, tmp_buf);
                strcat(print_stderr, "\n");
                fprintf(stdout, "%s", print_stderr);
            }else{
                step = 0;
                strcpy(print_stderr, sender);
                strcat(print_stderr ,": ");
                for(int i = 4 + strlen(sender); buffer[i] != '\x00'; i++){
                    tmp_buf[step] = buffer[i];
                    step++;
                }   
                step = 0;
                strcat(print_stderr, tmp_buf);
                strcat(print_stderr, "\n");
                fprintf(stdout, "%s", print_stderr);
            }
        }
        else if(msg_type == '\xfe' || msg_type == '\xFE'){ // ERR
            int step = 0;
            confirm_buf[0] = '\x00';
            confirm_buf[1] = buffer[1];
            confirm_buf[2] = buffer[2];
            
            result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }

            memset(confirm_buf, 0, 3);

            //receiving senders name
            for(int i = 3; buffer[i] != '\x00'; i++){
                sender[step] = buffer[i];
                step++;
            }
            step = 0;
            char err_buf[CONTENT];
            char content_buf[CONTENT];


            strcpy(err_buf, "ERR FROM ");
            strcat(err_buf, sender);
            strcat(err_buf, ": ");
            for(int i = 4 + strlen(sender); buffer[i] != '\x00'; i++){
                content_buf[step] = buffer[i];
                step++;
            }  
            step = 0;
            strcat(err_buf, content_buf); 
            strcat(err_buf, "\n");

            fprintf(stderr, "%s", err_buf);

            bye_buf[0] = '\xFF';
            bye_buf[1] = message_id_char[0];
            bye_buf[2] = message_id_char[1];
 
            result = sendto(my_udp_socket, bye_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }

            memset(bye_buf, 0, 3);

            return EXIT_FAILURE;
        }
        else if(msg_type == '\xff' || msg_type == '\xFF'){ // BYE
            confirm_buf[0] = '\x00';
            confirm_buf[1] = buffer[1];
            confirm_buf[2] = buffer[2];
            
            result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }

            memset(confirm_buf, 0, 3);
            
            return SUCCESS;
        }     
        else if(msg_type == CONFIRM){   //CONFIRM
            return SUCCESS;
        }
        else{// Unexpected identifiers
            char send_err[BUFFER_SIZE];
            memset(send_err, 0, BUFFER_SIZE);
            int err_length = 0;

            confirm_buf[0] = '\x00';
            confirm_buf[1] = buffer[1];
            confirm_buf[2] = buffer[2];
            
            result = sendto(my_udp_socket, confirm_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }

            memset(confirm_buf, 0, 3);

            send_err[0] = '\xFE';
            send_err[1] = message_id_char[0];
            send_err[2] = message_id_char[1];
            strcpy(send_err + 3, myDispName);   
            err_length += 3 + strlen(myDispName) + 1;
            strcpy(send_err + 3 + strlen(myDispName) + 1, "Unexpected identifiers");
            err_length += strlen("Unexpected identifiers") + 1;

            fprintf(stderr, "ERR: Unexpected identifiers\n");

            result = sendto(my_udp_socket, send_err, err_length, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }
            
            message_id++;
            message_id_char[0] = message_id >> 8;
            message_id_char[1] = message_id; 

            memset(send_err, 0, err_length);

            bye_buf[0] = '\xFF';
            bye_buf[1] = message_id_char[0];
            bye_buf[2] = message_id_char[1];

            result = sendto(my_udp_socket, bye_buf, 3, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(result < 0){
                fprintf(stderr, "ERROR: unable to send reguest\n");
                return INTERNAL_ERROR;
            }

            memset(bye_buf, 0, 3);

        
            return EXIT_FAILURE;
            
        }
        
    }

    return SUCCESS;
}
