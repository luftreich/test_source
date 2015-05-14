#include    <stdio.h>
#include    <stdlib.h>
#include    <stdbool.h>
#include    <sys/types.h>
#include    <sys/ipc.h>
#include    <sys/msg.h>
#include    <sys/vfs.h>
#include    <sys/mount.h>
#include    <sys/socket.h>
#include    <sys/ioctl.h>
#include    <sys/un.h>
#include    <sys/stat.h>
#include    <sys/socket.h>
#include    <sys/time.h>
#include    <linux/netlink.h>
#include    <arpa/inet.h>
#include    <netinet/in.h>
#include    <dirent.h>
#include    <sys/inotify.h>
#include    <limits.h>
#include    <net/if.h>
#include    <fcntl.h>
#include    <unistd.h>
#include    <string.h>
#include    <errno.h>
#include    <signal.h>
#include    <semaphore.h>
#include    <pthread.h>

#define BUF_LEN (10*sizeof(struct inotify_event))
#define SPEED_LOG_FILE "/var/speedtest.txt"
static void do_parse_the_file(char * file);
static void do_parse_device_speed(const char * buf,int * pSpeed,char *pIpStr);
int main(void)
{
    int fd;
    int wd;
    char buf[BUF_LEN];
    int nRead = 0;

    struct inotify_event *event;
    
    fd = inotify_init();
    if ((-1) == fd)
    {
        printf("Error notyfy init\n");
        return 0;
    }
    
    wd = inotify_add_watch(fd,SPEED_LOG_FILE,IN_CLOSE_WRITE);
    
    while (1)
    {
        nRead = read(fd,buf,BUF_LEN);
        if ((-1) == nRead)
        {
            printf ("read fd error\n");
        }
        printf("Write over event occured \n");
        do_parse_the_file(SPEED_LOG_FILE);        
    }
    inotify_rm_watch(fd,wd);
    return 0;
}

/*
Download speed:
0 KB/s  0 packets/s 192.168.111.100
Upload speed:
0 KB/s  0 packets/s 192.168.111.100
*/
#define DOWN_LOAD_STR "Download speed:"
#define UP_LOAD_STR     "Upload speed:"
static void do_parse_the_file(char * file)
{
    FILE *fd = NULL;
    char read_buf[1024];
    int nState = 0;
    int nSpeed = 0;
    char DeviceIp[16] = {0};
    char tmpbuf[256] = {0};
    char *ptr = NULL;

    fd = fopen(file,"r");
    if (NULL == fd)
    {
        perror("Can't open file\n");
    }

    memset (read_buf,0,sizeof(read_buf));
    while (NULL != fgets(read_buf,sizeof(read_buf),fd))
    {
        if (NULL != strstr(read_buf,DOWN_LOAD_STR))
        {
            nState = 1;//获取下载速度
            continue;
        }

        if (NULL != strstr(read_buf,UP_LOAD_STR))
        {
            nState = 2;//获取上传速度
            continue;
        }
        
        do_parse_device_speed(read_buf,&nSpeed,(char *)&DeviceIp);
        if (nState == 1)
        {
            printf("Get the down load speed is :");
        }
        if (nState == 2)
        {
            printf("Get the up load speed is :");
        }

        printf(" %d Bytes/s, Device ip is %s \n",nSpeed,DeviceIp);
        memset (read_buf,0,sizeof(read_buf));
        memset (DeviceIp,0,sizeof(DeviceIp));
    }
    printf("----------------------------\r\n\n");
    fclose(fd);
    return ;
}

static void do_parse_device_speed(const char * buf,int * pSpeed,char *pIpStr)
{
    char speed_H[16] = {0};
    char speed_L[16] = {0};
    char ip[16]={0};
    char *ptr1,*ptr2; 
    char tmpstr[256] = {0};
    int i = 0;
    int break_flag = 0;

    ptr1 = buf;
    while(1)
    {
        memset(tmpstr,0,sizeof(tmpstr));
        ptr2 = ptr1;
        ptr1 = strchr(ptr2,' ');
        if (NULL == ptr1)
        {
            break_flag = 1;
            ptr1 = buf+strlen(buf);
        }

        memcpy(tmpstr,ptr2,(ptr1-ptr2));
        printf(" %d : %s \n",i,tmpstr);
        if (0 == i)
        {
            char *p = NULL;
            p = strchr(tmpstr,'.');
            if (p == NULL)
            {
                *pSpeed = atoi(tmpstr);
            }
            else
            {
                printf(" p . %s \n",p);
                memcpy (speed_H,tmpstr,(p-tmpstr));
                memcpy (speed_L,p+1,3);
                *pSpeed = 1000*(atoi(speed_H))+atoi(speed_L);
            }
            
        }
        if(4 == i)
        {
            strncpy(pIpStr,tmpstr,16);
            pIpStr[16]='\0';
        }
        if (1 == break_flag)
            break;

        i++;
        while(' ' == (*ptr1))
        {
            ptr1 ++;
        }
    }
}


