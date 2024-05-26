/**
* @file server_udp.c
* @brief Implementation of UDP server
*
* @author Alisher Mazhirinov (xmazhi00)
* https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Stubs/cpp/DemoUdp
*/
#include "server_udp.h"

int comm_socket_udp = -1;

void sigintUDP(int sig_num)
{
    fprintf(stderr, "\nOk, I'm out.\n");
    if (comm_socket_udp != -1)
    {
        close(comm_socket_udp);
    }
    exit(EXIT_SUCCESS);
}

/**
 * @brief Creating UDP server
 * 
 * @param port
 * @param hostname
 * @return Return Code, Error or Success
 */
int udp_server(char* port, char* hostname){
    signal(SIGINT, sigintUDP);
    char rest[RECEIVE_BUFSIZE];
    char buff[RECEIVE_BUFSIZE];
    double result;
    struct sockaddr_in server_address, client_addr;
    struct hostent *server;
    int optval = 1;
    int errno;


    comm_socket_udp = socket(AF_INET, SOCK_DGRAM, 0); // IPv4, UDP, Protocol 0
    CHECK_PERROR(comm_socket_udp == -1, "Socket");

    if (setsockopt(comm_socket_udp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        perror("Setsockopt");
    }
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;                 // IPv4
    server_address.sin_addr.s_addr = INADDR_ANY;         // Listen on all interfaces, i.e. 0.0.0.0
    server_address.sin_port = htons(atoi(port)); // Port

    CHECK_PERROR(bind(comm_socket_udp, (struct sockaddr *)&server_address, sizeof(server_address)) == -1, "Bind");
    printf("INFO: Server listening on port %d\n", atoi(port));


    socklen_t client_len = sizeof(client_addr);
    char main_buffer[RECEIVE_BUFSIZE] = {0};
    char con_buff[3] = {0};
    char info_buf[RECEIVE_BUFSIZE] = {0};
    char send_buf[RECEIVE_BUFSIZE] = {0};

    char displayName[20] = {0};
    char username[20] = {0};
    char secret[128] = {0};


    int step = 0;
    char message_id_char[2] = {0};
    int message_id = 0;
    //message_id_char[0] = message_id >> 8;
    //message_id_char[1] = message_id;
    int length_sent = 0;

    while(1){
        while(1){
            int received_bytes = recvfrom(comm_socket_udp, main_buffer, RECEIVE_BUFSIZE, 0, (struct sockaddr *)&client_addr, &client_len);
            if (received_bytes <= 0)
            {
                fprintf(stderr, "Client timeout!\n");
                break;
            }
            char port_str[6] = {0};
            sprintf(port_str, "%hu", ntohs(client_addr.sin_port));
            length_sent = 0;
            if(main_buffer[0] == '\x02'){ // AUTH
                //*****************SENDING CONFIRM*****************
                
                con_buff[0] = '\x00';
                con_buff[1] = main_buffer[1];
                con_buff[2] = main_buffer[2];
                result = sendto(comm_socket_udp, con_buff, 3, 0, (struct sockaddr *)&client_addr, sizeof(server_address));
                if(result < 0){
                    fprintf(stderr, "ERROR: unable to send reguest\n");
                    return EXIT_FAILURE;
                }
                memset(con_buff, 0, 3);
                
                strcpy(info_buf, "RECV ");
                strcat(info_buf, inet_ntoa(client_addr.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "AUTH\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);

                //***********************************************
                sleep(0.5);
                //*****************SENDING REPLY*****************

                send_buf[0] = '\x01';
                message_id_char[0] = message_id >> 8;
                message_id_char[1] = message_id;
                send_buf[1] = message_id_char[0];
                send_buf[2] = message_id_char[1];
                send_buf[3] = '\x01';
                send_buf[4] = main_buffer[1];
                send_buf[5] = main_buffer[2];
                strcpy(send_buf + 6, "OK");
                result = sendto(comm_socket_udp, send_buf, 9, 0, (struct sockaddr *)&client_addr, sizeof(server_address));
                if(result < 0){
                    fprintf(stderr, "ERROR: unable to send reguest\n");
                    return EXIT_FAILURE;
                }
                memset(send_buf, 0, 9);
                message_id++;
                
                strcpy(info_buf, "SENT ");
                strcat(info_buf, inet_ntoa(client_addr.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "REPLY\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
                //***************************************************
                sleep(0.5);
                //******************SENDING MESSAGE******************

                send_buf[0] = '\x04';
                message_id_char[0] = message_id >> 8;
                message_id_char[1] = message_id;
                send_buf[1] = message_id_char[0];
                send_buf[2] = message_id_char[1];
                strcpy(send_buf + 3, "Server");
                length_sent += 3 + strlen("Server") + 1;


                for (int i = 3; main_buffer[i] != '\x00'; i++){
                    username[step] = main_buffer[i];
                    step++;
                }

                step = 0;

                for (int i = 4 + strlen(username); main_buffer[i] != '\x00'; i++)
                {
                    displayName[step] = main_buffer[i];
                    step++;
                }

                step = 0;

                for (int i = 5 + strlen(username) + strlen(displayName); main_buffer[i] != '\x00'; i++)
                {
                    secret[step] = main_buffer[i];
                    step++;
                }
                
                step = 0;

                char tmp_buf[50] = {0};
                strcpy(tmp_buf, displayName);
                strcat(tmp_buf, " joined default");

                strcpy(send_buf + length_sent, tmp_buf);
                length_sent += strlen(tmp_buf) + 1;
                result = sendto(comm_socket_udp, send_buf, length_sent, 0, (struct sockaddr *)&client_addr, sizeof(server_address));
                if(result < 0){
                    fprintf(stderr, "ERR: unable to send reguest\n");
                    return EXIT_FAILURE;
                }
                memset(send_buf, 0, length_sent);
                message_id++;

                strcpy(info_buf, "SENT ");
                strcat(info_buf, inet_ntoa(client_addr.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "MSG\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
                //***************************************************
            }
            else if(main_buffer[0] == '\x03'){ // JOIN
                //*****************SENDING CONFIRM*****************
                con_buff[0] = '\x00';
                con_buff[1] = main_buffer[1];
                con_buff[2] = main_buffer[2];
                result = sendto(comm_socket_udp, con_buff, 3, 0, (struct sockaddr *)&client_addr, sizeof(server_address));
                if(result < 0){
                    fprintf(stderr, "ERR: unable to send reguest\n");
                    return EXIT_FAILURE;
                }
                memset(con_buff, 0, 3);
                
                strcpy(info_buf, "RECV ");
                strcat(info_buf, inet_ntoa(client_addr.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "JOIN\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);

                //***********************************************
                sleep(0.5);
                //*****************SENDING REPLY*****************

                send_buf[0] = '\x01';
                message_id_char[0] = message_id >> 8;
                message_id_char[1] = message_id;
                send_buf[1] = message_id_char[0];
                send_buf[2] = message_id_char[1];
                send_buf[3] = '\x01';
                send_buf[4] = main_buffer[1];
                send_buf[5] = main_buffer[2];
                strcpy(send_buf + 6, "OK");
                result = sendto(comm_socket_udp, send_buf, 9, 0, (struct sockaddr *)&client_addr, sizeof(server_address));
                if(result < 0){
                    fprintf(stderr, "ERROR: unable to send reguest\n");
                    return EXIT_FAILURE;
                }
                memset(send_buf, 0, length_sent);
                message_id++;
                
                strcpy(info_buf, "SENT ");
                strcat(info_buf, inet_ntoa(client_addr.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "REPLY\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
                //***************************************************
                sleep(0.5);
                length_sent = 0;
                //******************SENDING MESSAGE******************

                send_buf[0] = '\x04';
                message_id_char[0] = message_id >> 8;
                message_id_char[1] = message_id;
                send_buf[1] = message_id_char[0];
                send_buf[2] = message_id_char[1];
                strcpy(send_buf + 3, "Server");
                length_sent += 3 + strlen("Server") + 1;

                char channelId[20] = {0};
                
                for (int i = 3; main_buffer[i] != '\x00'; i++){
                    channelId[step] = main_buffer[i];
                    step++;
                }

                step = 0;
                
                char tmp_buf[30] = {0};
                strcpy(tmp_buf, displayName);
                strcat(tmp_buf, " joined ");
                strcat(tmp_buf, channelId);

                strcpy(send_buf + length_sent, tmp_buf);
                length_sent += strlen(tmp_buf) + 1;

                result = sendto(comm_socket_udp, send_buf, length_sent, 0, (struct sockaddr *)&client_addr, sizeof(server_address));
                if(result < 0){
                    fprintf(stderr, "ERROR: unable to send reguest\n");
                    return EXIT_FAILURE;
                }
                memset(send_buf, 0, length_sent);
                message_id++;

                strcpy(info_buf, "SENT ");
                strcat(info_buf, inet_ntoa(client_addr.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "MSG\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
                //***************************************************
            }else if(main_buffer[0] == '\xff' || main_buffer[0] == '\xFF'){ //BYE
                //*****************SENDING CONFIRM*****************
                con_buff[0] = '\x00';
                con_buff[1] = main_buffer[1];
                con_buff[2] = main_buffer[2];
                result = sendto(comm_socket_udp, con_buff, 3, 0, (struct sockaddr *)&client_addr, sizeof(server_address));
                if(result < 0){
                    fprintf(stderr, "ERR: unable to send reguest\n");
                    return EXIT_FAILURE;
                }
                memset(con_buff, 0, 3);
                
                strcpy(info_buf, "RECV ");
                strcat(info_buf, inet_ntoa(client_addr.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "BYE\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
                break;
            }else if(main_buffer[0] == '\x00'){ // CONFIRM
                continue;
            }
            else{
                //*****************SENDING CONFIRM*****************
                con_buff[0] = '\x00';
                con_buff[1] = main_buffer[1];
                con_buff[2] = main_buffer[2];
                result = sendto(comm_socket_udp, con_buff, 3, 0, (struct sockaddr *)&client_addr, sizeof(server_address));
                if(result < 0){
                    fprintf(stderr, "ERROR: unable to send reguest\n");
                    return EXIT_FAILURE;
                }
                memset(con_buff, 0, 3);
                strcpy(info_buf, "RECV ");
                strcat(info_buf, inet_ntoa(client_addr.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "MSG\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
                memset(send_buf, 0, RECEIVE_BUFSIZE);
            
            }
            
            memset(main_buffer, 0, RECEIVE_BUFSIZE);
        }

        memset(main_buffer, 0, RECEIVE_BUFSIZE);
        printf("INFO: Client served\n");
    }


    return 0;
}