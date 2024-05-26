/**
* @file server_tcp.c
* @brief Implementation of TCP server
*
* @author Alisher Mazhirinov (xmazhi00)
* https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Stubs/cpp/DemoTcp
*/
#include "server_tcp.h"


int comm_socket = -1;
int wsocket = -1;

void sigintHandler(int sig_num)
{
	fprintf(stderr, "\nOk, I'm out.\n");
	if (wsocket != -1)
	{
		close(wsocket);
	}
	if (comm_socket != -1)
	{
		close(comm_socket);
	}
	exit(0);
}
/**
 * @brief Creating TCP server
 * 
 * @param port
 * @param hostname
 * @return Return Code, Error or Success
 */
int tcp_server(char* port, char* hostname){
    signal(SIGINT, sigintHandler);

    char main_buffer[RECEIVE_BUFSIZE] = {0};
    int server_port = atoi(port);
    struct sockaddr_in sa;
	struct sockaddr_in sa_client;
    socklen_t sa_client_len=sizeof(sa_client);
    struct hostent *server;

    wsocket = socket(AF_INET, SOCK_STREAM, 0);
    CHECK_PERROR(wsocket == -1, "Socket");


    memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;				 // IPv4
	sa.sin_addr.s_addr = INADDR_ANY;		 // Listen on all interfaces, i.e. 0.0.0.0
	sa.sin_port = htons(server_port); // Port
        
        
    CHECK_PERROR(bind(wsocket, (struct sockaddr*)&sa, sizeof(sa)) == -1, "Bind");

    CHECK_PERROR(listen(wsocket, 1) == -1, "Listen");

    printf("INFO: Server listening on port %d\n", server_port);
	printf("INFO: Receive buffer size: %d\n", RECEIVE_BUFSIZE);
    fd_set read_fds; 
    int result;
    while(1){
        comm_socket = accept(wsocket, (struct sockaddr *)&sa_client, &sa_client_len);
		if (comm_socket == -1)
		{
			perror("Accept");
			continue;
		}
        printf("INFO: Client connected: %s:%d\n", inet_ntoa(sa_client.sin_addr), ntohs(sa_client.sin_port));

        while(1){
            int received_bytes = recv(comm_socket, main_buffer, RECEIVE_BUFSIZE, 0);
            if (received_bytes <= 0)
            {
                fprintf(stderr, "No data received from the client!\n");
                break;
            }

            char info_buf[RECEIVE_BUFSIZE] = {0};

            char port_str[6] = {0};

            main_buffer[received_bytes - 2] = '\0';

            char* delimiter = " ";
            char *tokens[10] = {0};
            int num_tokens = 0;

            char *token = strtok(main_buffer, delimiter);

            while (token != NULL && num_tokens < 10) {
                tokens[num_tokens] = token;
                num_tokens++;
                token = strtok(NULL, delimiter);
            }
            
        
            char send_buf[RECEIVE_BUFSIZE] = {0};

            sprintf(port_str, "%hu", ntohs(sa_client.sin_port));

            if(strcmp(tokens[0], "AUTH") == 0){

                strcpy(info_buf, "RECV ");
                strcat(info_buf, inet_ntoa(sa_client.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "AUTH\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);

                // crafting message for user
                strcpy(send_buf, "REPLY OK IS Auth success");
                strcat(send_buf, "\r\n");

                int bytes_sent = send(comm_socket, send_buf, strlen(send_buf), 0);
                if (bytes_sent == -1) {
                    perror("Failed to send message");
                }

                memset(send_buf, 0, RECEIVE_BUFSIZE);

                strcpy(info_buf, "SENT ");
                strcat(info_buf, inet_ntoa(sa_client.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "REPLY\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);

                sleep(1);

                strcpy(send_buf, "MSG FROM Server IS ");
                strcat(send_buf, tokens[3]);
                strcat(send_buf, " joined default");
                strcat(send_buf, "\r\n");

                bytes_sent = send(comm_socket, send_buf, strlen(send_buf), 0);
                if (bytes_sent == -1) {
                    perror("Failed to send message");
                }
                memset(send_buf, 0, RECEIVE_BUFSIZE);

                strcpy(info_buf, "SENT ");
                strcat(info_buf, inet_ntoa(sa_client.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "MSG\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
            }
            else if(strcmp(tokens[0], "JOIN") == 0){

                strcpy(info_buf, "RECV ");
                strcat(info_buf, inet_ntoa(sa_client.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "JOIN\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);

                strcpy(send_buf, "REPLY OK IS Join success");
                strcat(send_buf, "\r\n");


                int bytes_sent = send(comm_socket, send_buf, strlen(send_buf), 0);
                if (bytes_sent == -1) {
                    perror("Failed to send message");
                }

                memset(send_buf, 0, RECEIVE_BUFSIZE);

                strcpy(info_buf, "SENT ");
                strcat(info_buf, inet_ntoa(sa_client.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "REPLY\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);

                sleep(1);

                strcpy(send_buf, "MSG FROM Server IS ");
                strcat(send_buf, tokens[3]);
                strcat(send_buf, " joined ");
                strcat(send_buf, tokens[1]);
                strcat(send_buf, "\r\n");

                bytes_sent = send(comm_socket, send_buf, strlen(send_buf), 0);
                if (bytes_sent == -1) {
                    perror("Failed to send message");
                }
                memset(send_buf, 0, RECEIVE_BUFSIZE);

                strcpy(info_buf, "SENT ");
                strcat(info_buf, inet_ntoa(sa_client.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "MSG\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
            }
            else if(strcmp(tokens[0], "BYE") == 0){
                strcpy(info_buf, "RECV ");
                strcat(info_buf, inet_ntoa(sa_client.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "BYE\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
                break;
            }
            else{
                strcpy(info_buf, "RECV ");
                strcat(info_buf, inet_ntoa(sa_client.sin_addr));
                strcat(info_buf, ":");
                strcat(info_buf, port_str);
                strcat(info_buf, " | ");
                strcat(info_buf, "MSG\n");
                
                printf("%s", info_buf);

                memset(info_buf, 0, RECEIVE_BUFSIZE);
                memset(send_buf, 0, RECEIVE_BUFSIZE);
            }
           
            
        }



        memset(main_buffer, 0, RECEIVE_BUFSIZE);
        close(comm_socket);		   // Close the communication socket
		printf("INFO: Client disconnected\n");
    }


    return 0;
}