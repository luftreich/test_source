/********************client.c*************************/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#define MAXDATASIZE 100
#include <fcntl.h>
#include "xl_common.h"
int main(int argc,char *argv[]){
    
    int sockfd;
    struct hostent *host;
    struct sockaddr_in serv_addr;
    
    if(argc<3){
        fprintf(stderr,"Please enter the server's hostname!\n");
        exit(1);
    }

    if((host=gethostbyname(argv[1]))==NULL){
        herror("gethostbyname error!");
        exit(1);
    }

    if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1){
        perror("socket create error!");
        exit(1);
    }


    int server_port = atoi(argv[2]);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(server_port);
    serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(serv_addr.sin_zero),8);

    struct timeval tv = {3,0};

    if (argc == 4)
        tv.tv_sec=atoi(argv[3]);

    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval)); 
    int n = connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr));
    if (n <0)
    {
        if (errno == EINPROGRESS)
        {
            perror("timeout");
        }
        else
        {
            perror("connect");
        }
    }
    else
    {
        printf("connect [%s] port [%s] success\n",argv[1],argv[2]);
    }

    close(sockfd);
    return 0;
}
