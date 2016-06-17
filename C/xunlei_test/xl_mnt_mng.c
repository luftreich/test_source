/**********************************************************
Copyright (C), 2003-2013, Xunlei Network Tech. Ltd.
File Name:xl_mnt_mng.c
Author:     hanshaoyang
Version:    1.0
Date:        2013-11-18
Description:  mnt ģ��������main���������ļ�
History:
    1. Date:2013-11-18
       Author:hanshaoyang
       Modification: Creat this file
***********************************************************/

#include    "xl_common.h"
#include    "xl_mnt_command.h"
#include    "xl_mnt_mng.h"
#include    <linux/netlink.h>
#include    <linux/rtnetlink.h>
#include    <linux/if.h>

/* --- local variable --- */

static pthread_t tid_w = 0,tid_r = 0;
static pthread_t sdtid_w = 0,sdtid_r = 0;
static int tid_w_break=0,tid_r_break=0;
static int sdtid_w_break=0,sdtid_r_break=0;
/* --- local functions --- */
static int mnt_main(void);
//static void trigger_the_netlink_state(void);

int usb_init(void)
{
    mnt_usb_msg_init();
    mnt_main();
    return 0;
}

int   timeval_subtract(struct   timeval*   result,   struct   timeval*   x,   struct   timeval*   y);

int time_diff(struct timeval start,struct timeval stop)
{
    struct   timeval   diff;
    timeval_subtract(&diff,&start,&stop);
    //printf("�ܼ���ʱ:%d ΢��\n",(int)(diff.tv_sec*1000000+diff.tv_usec));
    return (diff.tv_sec*1000*1000)+diff.tv_usec;
}

  /**
      *   ��������ʱ���ļ������õ�ʱ����
      *   @param   struct   timeval*   resule   ���ؼ���������ʱ��
      *   @param   struct   timeval*   x             ��Ҫ������ǰһ��ʱ��
      *   @param   struct   timeval*   y             ��Ҫ�����ĺ�һ��ʱ��
      *   return   -1   failure   ,0   success
  **/
  int   timeval_subtract(struct   timeval*   result,   struct   timeval*   x,   struct   timeval*   y)
  {

        if ( x->tv_sec > y->tv_sec )
            return   -1;

        if ( (x->tv_sec == y->tv_sec) && (x->tv_usec>y->tv_usec))
            return   -1;

        result->tv_sec = (y->tv_sec) - (x->tv_sec);
        result->tv_usec = (y->tv_usec) - (x->tv_usec);


        if (result->tv_usec<0)
        {
            result->tv_sec--;
            result->tv_usec+=1000000;
        }

        printf("start: sec %d ,us %d \n",(int)x->tv_sec,(int)x->tv_usec);
        printf("stop: sec %d ,us %d \n",(int)y->tv_sec,(int)y->tv_usec);

        return 0;
  }


static int mnt_main(void)
{
    int nRet = -1;
    int nInitFlag = 0;


    /*������ģ���ĳ�ʼ��*/
    if (EN_OK != mnt_usb_init())
    {
        nInitFlag |= (1<<0);
    }

    return nRet;
}

static void print_usage_sdcard(FILE *stream)
{
    fprintf(stream,"Exit : please input 'q','quit',or 'exit' \n");
    fprintf(stream,"Input \"read\" to read the sdcard\n");
    fprintf(stream,"Input \"allread\" to read sdcard always\n");
    fprintf(stream,"Input \"write\" to write the sdcard\n");
    fprintf(stream,"Input \"allwrite\" to write sdcard always\n");
}

static void print_usage(FILE *stream)
{
    fprintf(stream,"Exit : please input 'q','quit',or 'exit' \n");
    fprintf(stream,"Input \"read\" to read the usb disk\n");
    fprintf(stream,"Input \"allread\" to read usb disk always\n");
    fprintf(stream,"Input \"write\" to write the usb disk\n");
    fprintf(stream,"Input \"allwrite\" to write usb disk always\n");
}


static St_xl_partition_info *pDestData = NULL;
static St_xl_partition_info *pSDDestData = NULL;
int mnt_usb_init_data(void)
{
    int disk_id = ALL_USB_DISK;
    int part_cnt = 0;
    mnt_usb_read_partition(disk_id,(U32 *)&part_cnt,&pDestData);
    if(NULL == pDestData)
    {
        return -1;
    }
    else
    {
        fprintf(stdout,"\n usb disk found \n");
        fprintf(stdout,"usb mount path: %s \n",pDestData->szMountDir);
        fprintf(stdout,"usb space: %lld MB\n",pDestData->u64Capacity/(1024*1024));
        fprintf(stdout,"usb used: %lld MB\n\n",pDestData->u64Used/(1024*1024));
    }
    return 0;
}

int mnt_usb_term_data(void)
{
    tid_r_break=1;
    tid_w_break =1;
    if (NULL != pDestData)
    {
        free(pDestData);
    }
    if(0!=tid_w)
    {
            pthread_cancel(tid_w);
            pthread_join(tid_w,NULL);
            tid_w=0;
    }

    if(0!=tid_r)
    {
            pthread_cancel(tid_r);
            pthread_join(tid_r,NULL);
            tid_r=0;
    }
    return 0;
}

void do_write_usb_once(void)
{
        char usb_path[256] = {0};
        char command[1024] = {0};
        sprintf(usb_path,"%s",pDestData->szMountDir);
        sprintf(command,"dd if=/dev/zero of=%s/.testdata bs=100k count=1000",usb_path);
        struct timeval start,stop;
        gettimeofday(&start,0);
        system(command);
        gettimeofday(&stop,0);
        int time_us = time_diff(start,stop);
        fprintf(stdout,"Cost %d us ,size %d KB, READ Write speed is %f MB/s \n",time_us,100*1024*1000,(float)((double)(100*1000)/(double)(time_us/1000)));
}

void do_write_sdcard_once(void)
{
        char usb_path[256] = {0};
        char command[1024] = {0};
        sprintf(usb_path,"%s",pSDDestData->szMountDir);
        sprintf(command,"dd if=/dev/zero of=%s/.testdata bs=100k count=1000",usb_path);
        struct timeval start,stop;
        gettimeofday(&start,0);
        system(command);
        gettimeofday(&stop,0);
        int time_us = time_diff(start,stop);
        fprintf(stdout,"Cost %d us ,size %d KB, SDCARD Write speed is %f MB/s \n",time_us,100*1024*1000,(float)((double)(100*1000)/(double)(time_us/1000)));
}
int mnt_ubus_usb_plug_notify(void)
{
     return 0;
}

static int CreateMyFile(char * szFileName,int nFileLength)
{
   FILE* fp = fopen(szFileName, "wb+"); // �����ļ�
   if(fp==NULL)
   {
           printf("creat file error\n");
       return -1;
   }
   else
   {
	   fseek(fp, nFileLength-1, SEEK_SET);    // ���ļ���ָ�� ���� ָ����С��λ��
	   fputc(32, fp);                        // ��Ҫָ����С�ļ���ĩβ������һ������
	   fclose(fp);
           return 0;
   }
}

void do_read_usb_once(void)
{
        char usb_path[256] = {0};
        char command[1024] = {0};
        char file_name[1024] = {0};

        sprintf(usb_path,"%s",pDestData->szMountDir);

        sprintf(file_name,"%s/.testdata",usb_path);
        if (access(file_name,F_OK) != 0)
        {
              CreateMyFile(file_name,100*1024*1000);
        }
        sprintf(command,"dd if=/%s/.testdata of=/dev/null bs=100k count=1000",usb_path);
        struct timeval start,stop;
        gettimeofday(&start,0);
        system(command);
        gettimeofday(&stop,0);
        int time_us = time_diff(start,stop);
        fprintf(stdout,"Cost %d us ,size %d KB, USB Read speed is %f MB/s \n",time_us,100*1024*1000,(float)((double)(100*1000)/(double)(time_us/1000)));
}

void do_read_sdcard_once(void)
{
        char usb_path[256] = {0};
        char command[1024] = {0};
        char file_name[1024] = {0};

        sprintf(usb_path,"%s",pSDDestData->szMountDir);

        sprintf(file_name,"%s/.testdata",usb_path);
        if (access(file_name,F_OK) != 0)
        {
              CreateMyFile(file_name,100*1024*1000);
        }
        sprintf(command,"dd if=/%s/.testdata of=/dev/null bs=100k count=1000",usb_path);
        struct timeval start,stop;
        gettimeofday(&start,0);
        system(command);
        gettimeofday(&stop,0);
        int time_us = time_diff(start,stop);
        fprintf(stdout,"Cost %d us ,size %d KB, SDCARD Read speed is %f MB/s \n",time_us,100*1024*1000,(float)((double)(100*1000)/(double)(time_us/1000)));
}


static void * write_usb_all_thread(void)
{
    char usb_path[256] = {0};
    char command[1024] = {0};
    char file_name[1024] = {0};
    int i=0;
    while (pDestData == NULL)
    {
        fprintf(stderr,"can't get partition data \n");
        sleep(5);
    }
    sprintf(usb_path,"%s",pDestData->szMountDir);

    sprintf(file_name,"/tmp/.usbwrite");
    if (access(file_name,F_OK) != 0)
    {
          CreateMyFile(file_name,100*1000);
    }
    sprintf(command,"cp  /tmp/.usbwrite %s/.usbwrite ",usb_path);
    fprintf(stdout,"start write to %s\n",usb_path);
    tid_w_break = 0;
    while(1)
    {
        printf("usb write : %d\r",i);
        if(i==100)
                i=0;
        i++;
        system(command);
        if(tid_w_break)
        {
                break;
        }

        fflush(stdout);
   }
    return NULL;
}

static void * write_sdcard_all_thread(void)
{
    char usb_path[256] = {0};
    char command[1024] = {0};
    char file_name[1024] = {0};
    int i=0;
    sprintf(usb_path,"%s",pSDDestData->szMountDir);

    sprintf(file_name,"/tmp/.sdcardwrite");
    if (access(file_name,F_OK) != 0)
    {
          CreateMyFile(file_name,100*1000);
    }
    sprintf(command,"cp  /tmp/.sdcardwrite %s/.sdcardwrite ",usb_path);
    fprintf(stdout,"start write to %s\n",usb_path);
    sdtid_w_break = 0;
    while(1)
    {
        printf("sdcard write : %d\r",i);
        if(i==100)
                i=0;
        i++;
        system(command);
        if(sdtid_w_break)
        {
                break;
        }

        fflush(stdout);
   }
    return NULL;
}
static void * read_usb_all_thread(void)
{
    char usb_path[256] = {0};
    char command[1024] = {0};
    char file_name[1024] = {0};
    int i=0;
    sprintf(usb_path,"%s",pDestData->szMountDir);

    sprintf(file_name,"%s/.usbread",usb_path);
    if (access(file_name,F_OK) != 0)
    {
          CreateMyFile(file_name,100*1000);
    }
    sprintf(command,"cp  -a %s/.usbread /tmp/.usbread",usb_path);
    fprintf(stdout,"start read from %s\n",usb_path);
    tid_r_break=0;
    while(1)
    {
        printf("usb read : %d\r",i);
        i++;
        if (i==100)
                i=0;
        system(command);
        system("rm -rf /tmp/.usbread");
        if(tid_r_break)
        {
                break;
        }
        fflush(stdout);
   }
    return NULL;
}

static void * read_sdcard_all_thread(void)
{
    char usb_path[256] = {0};
    char command[1024] = {0};
    char file_name[1024] = {0};
    int i=0;
    sprintf(usb_path,"%s",pSDDestData->szMountDir);

    sprintf(file_name,"%s/.sdcardread",usb_path);
    if (access(file_name,F_OK) != 0)
    {
          CreateMyFile(file_name,100*1000);
    }
    sprintf(command,"cp  -a %s/.sdcardread /tmp/.sdcardread",usb_path);
    fprintf(stdout,"start read from %s\n",usb_path);
    sdtid_r_break=0;
    while(1)
    {
        printf("sdcard read : %d\r",i);
        i++;
        if (i==100)
                i=0;
        system(command);
        system("rm -rf /tmp/.sdcardread");
        if(sdtid_r_break)
        {
                break;
        }
        fflush(stdout);
   }
    return NULL;
}
void do_write_usb_all(void)
{
        if (tid_w == 0)
        {
             pthread_create(&tid_w,NULL,(void *)write_usb_all_thread,NULL);
        }
}

void do_read_usb_all(void)
{
        if (tid_r == 0)
        {
             pthread_create(&tid_r,NULL,(void *)read_usb_all_thread,NULL);
        }
}
void do_write_sdcard_all(void)
{
        if (sdtid_w == 0)
        {
             pthread_create(&sdtid_w,NULL,(void *)write_sdcard_all_thread,NULL);
        }
}

void do_read_sdcard_all(void)
{
        if (sdtid_r == 0)
        {
             pthread_create(&sdtid_r,NULL,(void *)read_sdcard_all_thread,NULL);
        }
}

int usb_module_test(void)
{
     char input[256] = {0};

     int ret = mnt_usb_init_data();
     if (ret != 0)
     {
         fprintf(stdout,"USB disk not found\n");
         fprintf(stdout,"Please insert the usb disk first\n");
         return ret;
     }
     print_usage(stdout);

     while (1)
     {
         printf("usb >>> ");
         memset(input,0,sizeof(input));
         fgets(input,256,stdin);
         if (!strncmp("q",input,1)||!strncmp("quit",input,4))
         {
             printf("quit usb test \n");
             mnt_usb_term_data();
             break;
         }

         if (!strncmp("write",input,5))
         {
             do_write_usb_once();
         }
         else if (!strncmp("read",input,4))
         {
             do_read_usb_once();
         }
         else if(!strncmp("allwrite",input,8))
         {
             do_write_usb_all();
         }
         else if(!strncmp("allread",input,7))
         {
             do_read_usb_all();
         }
         else if(!strncmp("wrusb",input,5))
         {
             do_read_usb_all();
             do_write_usb_all();
         }
         else if(!strncmp("help",input,4)||!strncmp("h",input,1))
         {
             print_usage(stdout);
         }
     }
     return 0;
}


int mnt_sdcard_init_data(void)
{
    int disk_id = ALL_USB_DISK;
    int part_cnt = 0;
    sdtid_r_break =0;
    sdtid_w_break = 0;
    mnt_sdcard_read_partition(disk_id,(U32 *)&part_cnt,&pSDDestData);
    if(NULL == pSDDestData)
    {
        printf("%d \n ",__LINE__);
        return -1;
    }
    else
    {
        fprintf(stdout,"\nsdcard  found \n");
        fprintf(stdout,"sdcard mount path: %s \n",pSDDestData->szMountDir);
        fprintf(stdout,"sdcard space: %lld MB\n",pSDDestData->u64Capacity/(1024*1024));
        fprintf(stdout,"sdcard used: %lld MB\n\n",pSDDestData->u64Used/(1024*1024));
    }
    return 0;
}

int mnt_sdcard_term_data(void)
{
    sdtid_r_break=1;
    sdtid_w_break =1;
    if (NULL != pSDDestData)
    {
        free(pSDDestData);
    }
    if(0!=sdtid_w)
    {
            pthread_cancel(sdtid_w);
            pthread_join(sdtid_w,NULL);
            tid_w=0;
    }

    if(0!=sdtid_r)
    {
            pthread_cancel(sdtid_r);
            pthread_join(sdtid_r,NULL);
            tid_r=0;
    }
    return 0;
}

int sdcard_module_test(void)
{
     char input[256] = {0};

     int ret = mnt_sdcard_init_data();
     if (ret != 0)
     {
         fprintf(stdout,"SDCARD not found\n");
         fprintf(stdout,"Please insert the SDCARD first\n");
         return ret;
     }
     print_usage_sdcard(stdout);

     while (1)
     {
         printf("sdcard >>> ");
         memset(input,0,sizeof(input));
         fgets(input,256,stdin);
         if (!strncmp("q",input,1)||!strncmp("quit",input,4))
         {
             printf("quit usb test \n");
             mnt_sdcard_term_data();
             break;
         }

         if (!strncmp("write",input,5))
         {
             do_write_sdcard_once();
         }
         else if (!strncmp("read",input,4))
         {
             do_read_sdcard_once();
         }
         else if(!strncmp("allwrite",input,8))
         {
             do_write_sdcard_all();
         }
         else if(!strncmp("allread",input,7))
         {
             do_read_sdcard_all();
         }
         else if (!strncmp("wrsdcard",input,8))
         {
             do_read_sdcard_all();
             do_write_sdcard_all();
         }
         else if(!strncmp("help",input,4)||!strncmp("h",input,1))
         {
             print_usage_sdcard(stdout);
         }
     }
     return 0;
}

/***********************************************************
Function:mnt_get_ip_addr
Description: To obtain the ip address of ethernet device
Input:pEthName: the eth name such as "eth0"
Output:pMac:To store the mac address of pEthName
Return: 0:success
        1:failed
Others:
History:
************************************************************/
EN_ERROR_NUM device_get_ip(char* ip,const char *pEthName)
{
    int sock ;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sock= socket(AF_INET,SOCK_DGRAM,0);
    if (sock == -1)
    {
        return -1;
    }

    strncpy(ifr.ifr_name,pEthName,8);
    ifr.ifr_name[8-1]=0;

    if (ioctl(sock,SIOCGIFADDR,&ifr) < 0)
    {
        return -1;
    }

    memcpy(&sin,&ifr.ifr_addr,sizeof(sin));

    sprintf(ip,"%s",inet_ntoa(sin.sin_addr));
    return EN_OK;
}


/***********************************************************
Function: net_detect
Description: Detect if the eth line is pluged in
Input: net_name :such as "eth0","eth1"
Output: NONE
Return: 1: plug in
        0: plug out
Others:
History:
************************************************************/
int net_detect(char* net_name)
{
    int skfd = 0;
    struct ifreq ifr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        printf("%s:%d Open socket error!\n", __FILE__, __LINE__);
        return -1;
    }

    strcpy(ifr.ifr_name, net_name);
    if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 )
    {
        printf("%s:%d IOCTL error!\n", __FILE__, __LINE__);
        printf("Maybe ethernet inferface %s is not valid!", ifr.ifr_name);
        close(skfd);
        return -1;
    }

    if(ifr.ifr_flags & IFF_RUNNING)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void print_sdcard_log(void)
{
    printf(" >>>> SDCARD INFO:\n");
    if (sdtid_r != 0)
    {
            printf("sdcard read is doing \n");
    }
    else
    {
            printf(" *** sdcard read error !!! ***\n");
    }

    if (sdtid_w != 0)
    {
            printf("sdcard write is doing \n");
    }
    else
    {
            printf(" *** sdcard write error !!! ***\n");
    }

    char usb_path[1024] = {0};
    char file_name[1024]={0};
    char command[1024] = {0};
    int ret = 0;

    sprintf(usb_path,"%s",pSDDestData->szMountDir);
    sprintf(file_name,"%s/.sdcard_detect",usb_path);
    sprintf(command,"rm -rf  %s ",file_name);
    system(command);
    memset(command,0,sizeof(command));
    sprintf(command,"touch %s ",file_name);
    ret = system(command);
    if(0!=ret)
    {
         printf(" *** sdcard write error ,can't write file \n");
    }

    printf("SDCARD INFO <<<<\n");
}

static void print_usb_log(void)
{
    printf(" >>>> USB INFO:\n");
    if (tid_r != 0)
    {
            printf("usb read is doing \n");
    }
    else
    {
            printf(" *** usb read error !!! ***\n");
    }

    if (tid_w != 0)
    {
            printf("usb write is doing \n");
    }
    else
    {
            printf(" *** usb write error !!! ***\n");
    }

    char usb_path[1024] = {0};
    char file_name[1024]={0};
    char command[1024] = {0};
    int ret = 0;

    sprintf(usb_path,"%s",pDestData->szMountDir);
    sprintf(file_name,"%s/.usb_detect",usb_path);
    sprintf(command,"rm -rf  %s ",file_name);
    system(command);
    memset(command,0,sizeof(command));
    sprintf(command,"touch %s ",file_name);
    ret = system(command);
    if(0!=ret)
    {
         printf(" *** usb write error ,can't write file \n");
    }
    printf("USB INFO <<<<\n");
}

static int usb_start_flag = 0;
void usb_start_run()
{
    if (0 == usb_start_flag)
    {
        while (mnt_is_usb_insert()==false)
        {
            sleep(1);
        }
        mnt_usb_init_data();
        do_write_usb_all();
        do_read_usb_all();
        usb_start_flag = 1;
    }
    print_usb_log();
}

void usb_stop_run()
{
    {
        mnt_usb_term_data();
        usb_start_flag = 0;
    }

}

static int sdcard_start_flag = 0;
void sdcard_start_run()
{
    if (0 == sdcard_start_flag)
    {
        mnt_sdcard_init_data();
        do_write_sdcard_all();
        do_read_sdcard_all();
        sdcard_start_flag = 1;
    }
    print_sdcard_log();
}

void sdcard_stop_run()
{
    {
        mnt_sdcard_term_data();
        sdcard_start_flag = 0;
    }
}
