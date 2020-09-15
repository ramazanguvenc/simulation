#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <errno.h> 
#include <ctype.h>
#include <semaphore.h>
#include <sys/file.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
int accept(int
sockfd, struct sockaddr *addr, socklen_t *addrlen);
#define _GNU_SOURCE
            /* See feature_test_macros(7) */
#include <sys/socket.h>
int accept4(int
sockfd, struct sockaddr *addr,            socklen_t *addrlen, int flags);


#define SA struct sockaddr 

int is_connected(int socket_fd);
void* handle_connection(void *vargp); 

struct thread_param{
    int connfd;
    pthread_t thread_id;
};



int main(){
    struct sockaddr_in servaddr, cli; 
    int  connfd,sockfd, len;
    int port = 12300;
    int thread_param_index = 0;
    struct thread_param params[30];
    srand((unsigned int)time(NULL));

    
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed\n");
        exit(1); 
    } 
    bzero(&servaddr, sizeof(servaddr)); 

    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port); 
    
    if((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0){
        printf("socket binding failed\n");
        exit(1);
    }
     if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    
    while(1){
        connfd = accept4(sockfd, (SA*)&cli, &len, SOCK_NONBLOCK); 
        if (connfd < 0) { 
            printf("request accept failed\n");
        }
        else{
            params[thread_param_index].connfd = connfd;
            pthread_create(&(params[thread_param_index].thread_id), NULL, handle_connection, (void*) &params[thread_param_index]);
            thread_param_index++;
            if(thread_param_index == 29) thread_param_index = 0;
        }
    }
   
    
    return 0;
}

int is_connected(int socket_fd)
{
    int error = 0;
    socklen_t len = sizeof (error);
    int retval = getsockopt (socket_fd, SOL_SOCKET, SO_ERROR, &error, &len);
    if (retval != 0) {
        /* there was a problem getting the error code */
        fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
    return 0;
    }
    return 1;
}


void* handle_connection(void *vargp){
    struct thread_param* this;
    this = (struct thread_param*) vargp;
    int connfd = this->connfd;
    char str[30];
    float x = 0.0,y =0.0,z = 0.0;
    float xincrease = 0.2;
    float yincrease = 0.5;
    float zincrease = -1.0;
    int flag = 1;
    char buf[1];
    do{
     nanosleep((const struct timespec[]){{0, 200000000L}}, NULL);
     if(y == 5.0) yincrease = yincrease * -1;
     if(z <= -5.0) zincrease = 1.0;
     if(z >= 5.0) zincrease = -1.0;
     if(x <= -5.0) xincrease = 0.2;
     if(x >= 5.0) xincrease = -0.2;
     sprintf(str, "%f %f %f", x, y, z);
     y += yincrease;
     
     z += zincrease;
     x += xincrease;

   

  
     write(connfd, str, 30);
     if(y < 0.4){
        yincrease = 0.5;
        y = 0.0;
     }
     read(connfd, buf, 1);
     if(buf[0] == 'e'){
         char msg[128];
         sprintf(msg, "client exited\n");
         write(1,msg,strlen(msg));
         close(connfd);
         return NULL;
     }
    }while(is_connected(connfd));
    
    close(connfd);
    char msg[128];
    sprintf(msg, "client exited\n");
    write(1,msg,strlen(msg));
    
    return NULL;
}
