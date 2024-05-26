/**
* @file server.c
* @brief Main function for parsing of CLI arguments and starting communication
*
* @author Alisher Mazhirinov (xmazhi00)
*
*/
#include "server.h"


int main(int argc, char** argv){
    // ./server -t tcp -s 0.0.0.0 -p 4567
    char* protocol = {0};
    char* hostname = {0};
    char* port = {0};
    int opt;
    if(strcmp(argv[2], "udp") == 0){
        while((opt = getopt(argc, argv, "t:s:p:")) != -1){
            switch (opt)
            {
            case 't':       
                protocol = optarg;     
                
                break;
            case 's':
                hostname= optarg;
                break;
            case 'p':
                port = optarg;
                break;
            }
        }
    }
    else if(strcmp(argv[2], "tcp") == 0){
        while((opt = getopt(argc, argv, "t:s:p:")) != -1){
            switch (opt)
            {
            case 't':       
                protocol = optarg;     
                
                break;
            case 's':
                hostname = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            }
        }
    }else{
        printf("wrong input\n");
        return EXIT_FAILURE;
    }

    if(strcmp(protocol, "tcp") == 0){
        printf("tcp\n");
        if(!tcp_server(port, hostname)){
            return EXIT_FAILURE;
        }
    }else{
        printf("udp\n");
        if(!udp_server(port, hostname)){
            return EXIT_FAILURE;
        }
    }


    return 0;
}