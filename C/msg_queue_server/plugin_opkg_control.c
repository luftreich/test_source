/**********************************************************
Copyright (C), 2003-2013, Xunlei Network Tech. Ltd.
File Name: plugin_opkg_control.c
Author:     hanshaoyang
Version:    1.0
Date:        2015-09-26
Description: control the opkg operation
History:
***********************************************************/

#include    "xl_common.h"
/* --- local macro --- */
#define   BUFFER_SIZE   (256)
#define   MSG_TYPE      (1)
/* --- local struct --- */
struct message
{
    long msg_type;
    char msg_text[BUFFER_SIZE];
};

/* --- local variable --- */
static int s_qid = 0;

/* --- local function --- */

int msg_queue_init(void)
{
    char *msg_path = "/var/run/.plugin_queue";
    key_t  key;

    if( access(msg_path, F_OK)!=0 )
    {
        XL_DEBUG(EN_PRINT_ERROR,"can't access path %s\n",msg_path);
        if (0>creat(msg_path, 0666))
        {
            XL_DEBUG(EN_PRINT_ERROR,"ftok return error: %s\n",strerror(errno));
        }
        if( access(msg_path, F_OK)==0 )
        {
            XL_DEBUG(EN_PRINT_INFO," access path %s ok\n",msg_path);
        }
    }

    /*根据不同的路径和关键字产生标准的key*/
    if((key = ftok("/tmp", 1024)) == -1)
    {
        perror("ftok");
        exit(1);
    }

    /*创建消息队列*/
    if((s_qid = msgget(key, IPC_CREAT|0666)) == -1)
    {
        perror("msgget");
        exit(1);
    }

    return 0;
}


/***********************************************************
Function: msg_send_command
Description: Send opkg command to plugin module msg queue
Input: pData : Point to the data will be sent
       nLen: the length of the msg data
Output:
Return: EN_OK :success
Others:
History:
************************************************************/
EN_ERROR_NUM msg_send_command(char *pData,int nLen)
{
    struct message  msg;
    EN_ERROR_NUM eError;
    char * pSndData;

    if ((BUFFER_SIZE - sizeof(U32)) < nLen)
    {
        return EN_ERROR_FAILED;
    }
    memset(&msg,0,sizeof(msg));
    pSndData = msg.msg_text;
    xl_encode_u32(pSndData, nLen,sizeof(msg.msg_text));
    pSndData += sizeof(U32);
    memcpy(pSndData,pData,nLen);
    msg.msg_type = MSG_TYPE;
    /*添加消息到消息队列中*/
    if((msgsnd(s_qid, &msg, BUFFER_SIZE, 0)) <0)
    {
        perror("message posted");
        eError = EN_ERROR_FAILED;
    }
    else
    {
        eError = EN_OK;
    }
    return eError;
}

/***********************************************************
Function: msg_recv_command
Description: rcv infomation from usb module msg queue
Input: NONE
Output: pData : Point to the data will be received
        pnLen : The length of the data
Return: EN_OK :success
Others:
History:
************************************************************/
EN_ERROR_NUM msg_recv_command(char *pData,int *pnLen)
{
    EN_ERROR_NUM eError;
    struct message  msg;
    char *pRcvData;

    memset(&msg,0,sizeof(msg));
    do
    {
        if((msgrcv(s_qid, (void*)&msg, BUFFER_SIZE, MSG_TYPE, 0)) <0)
        {
            eError = EN_ERROR_FAILED;
            if (EINTR == errno)
            {
                //XL_DEBUG(EN_PRINT_ERROR,"Interrupt by SIGCHLD handler: %s\n",strerror(errno));
                continue;
            }
            else
            {
                break;
            }
            perror("msgrcv");
        }
        else
        {
            XL_DEBUG(EN_PRINT_INFO,"RCV the msg in usb module success\n");
            eError = EN_OK;
            break;
        }
    }while(1);

    if (EN_OK == eError)
    {
        int nLen;
        pRcvData = msg.msg_text;
        xl_decode_u32(pRcvData,&nLen);
        pRcvData += sizeof(U32);
        memcpy(pData,pRcvData,nLen);
        *pnLen = nLen;
    }
    return eError;
}


static void * msg_control_process(void * args)
{
    char recv_data[256];
    int len;
    EN_ERROR_NUM eError;

    while(1)
    {
        memset(recv_data,0,sizeof(recv_data));

        eError = msg_recv_command(recv_data,&len);
        if (EN_OK != eError)
        {
            continue;
        }
        printf ("received data  %s\n",recv_data);
        sleep(10);
    }
    return NULL;
}

static void * msg_send_data_thread(void* args)
{
    char send_data[256] = {0};
    while (1)
    {
        printf ("please input msg info: \n");
        memset(send_data,0,sizeof(send_data));
        fgets(send_data,256,stdin);
        msg_send_command(send_data,strlen(send_data));
    }
    return NULL;
}

int main(void)
{
    pthread_t recv_pid = 0;
    pthread_t send_pid = 0;
    msg_queue_init();
    pthread_create(&recv_pid,NULL,msg_control_process,NULL);
    pthread_create(&send_pid,NULL,msg_send_data_thread,NULL);
    while(1)
    {
        sleep(60);
    }
    return 0;
}
