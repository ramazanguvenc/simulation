
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h> 


//#define DEFAULT_PORT 6666
//#define DEFAULT_PORT 12300
//#define DEFAULT_SERVER_ADDRESS "127.0.0.1"
//#define DEFAULT_SERVER_ADDRESS "192.168.1.109"
#define DEFAULT_BUF_LENGTH 30

#define SA struct sockaddr 

int sockfd;

int open_port(char* IP, int PORT){
    struct sockaddr_in servaddr, cli; 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        return -1;
    } 

    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(IP); 
    servaddr.sin_port = htons(PORT); 

    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        return -1; 
    } 

    return 0;
}

char* get_data_from_server(){
    char* recvbuf;
    
    recvbuf = (char*) malloc(DEFAULT_BUF_LENGTH);
    if(recvbuf == NULL) return NULL;
    int byte = read(sockfd, recvbuf, DEFAULT_BUF_LENGTH);
    if(byte < 1) return NULL;
    else return recvbuf;

}

int close_port(){
    write(sockfd, "e", 1);
    int flag = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(int));
    sockfd = -1;
    return 0;
    
}