#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
*/
 
#define HELLO_WORLD_SERVER_PORT    6666 
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
 
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: ./%s ServerIPAddress port\n",argv[0]);
        exit(1);
    }
 
    //设置一个socket地址结构client_addr,代表客户机internet地址, 端口
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr)); //把一段内存区的内容全部设置为0
    client_addr.sin_family = AF_INET;    //internet协议族
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
    client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
    //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket
    int client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    
    struct timeval timeout={10,0};//3s
    setsockopt(client_socket,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
    setsockopt(client_socket,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
    
    //把客户机的socket和客户机的socket地址结构联系起来
    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n"); 
        exit(1);
    }
 
    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(argv[1],&server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(atoi(argv[2]));
    socklen_t server_addr_length = sizeof(server_addr);
    //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect To %s!\n",argv[1]);
        exit(1);
    }
 
    printf("Please Input Send data to Server:\t");
     
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    scanf("%s",buffer);
    
    //向服务器发送buffer中的数据
    int length = send(client_socket,buffer,strlen(buffer)+1,0);
    printf("send %d length ,[%s]\n",length,buffer);
    #if 1
    bzero(buffer,BUFFER_SIZE);
    while( length = recv(client_socket,buffer,BUFFER_SIZE,0))
    {
        if(length < 0)
        {
            printf("Recieve Data From Server %s Failed!\n", argv[1]);
            break;
        }
//        int write_length = write(fp, buffer,length);
        printf ("receive form server :[%s]\n",buffer);
        bzero(buffer,BUFFER_SIZE);   
        break; 
    }
     
    #endif
    //关闭socket
    close(client_socket);
    return 0;
}
