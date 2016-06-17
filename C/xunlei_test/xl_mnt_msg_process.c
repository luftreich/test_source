/******************************************************************************
Copyright (C), 2003-2013, Xunlei Network Tech. Ltd.
FileName: xl_mnt_msg_process.c
Author: hanshaoyang
version: V1.0
Description: 本文件用来做MNT模块的消息处理，包括消息编码、解码、发送等
Date 2013/12/04
History:
        1. Date:  2013-12-04
           Author: hanshaoyang
           Modification: create this file
******************************************************************************/

#include "xl_common.h"
#include "xl_mnt_msg_process.h"
#include "xl_mnt_mng.h"

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
static int xl_decode_u32(CHAR* pcBuf,ULONG *pulData)
{
        CHAR * pcIn = pcBuf;
        ULONG ulData = 0;
        ulData = pcIn[0];
        ulData = (ulData<<8) + pcIn[1];
        ulData = (ulData<<8) + pcIn[2];
        ulData = (ulData<<8) + pcIn[3];
        *pulData = ulData;
        return 0;
}

static int xl_encode_u32(char * pcBuf,ULONG ulData,int ulLen)
{
        char * pcOut = pcBuf;
        pcOut[0] = (ulData>>24)&0xff;
        pcOut[1] = (ulData>>16)&0xff;
        pcOut[2] = (ulData>>8)&0xff;
        pcOut[3] = ulData&0xff;
        return 0;
}
int mnt_usb_msg_init(void)
{
    char *msg_path = "/tmp/.mnt_usb_queue_test";
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
    if((key = ftok(".", 512)) == -1)
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
Function: mnt_usb_msg_send
Description: Send infomation to usb module msg queue
Input: pData : Point to the data will be sent
       nLen: the length of the msg data
Output:
Return: EN_OK :success
Others:
History:
************************************************************/
EN_ERROR_NUM mnt_usb_msg_send(char *pData,int nLen)
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
Function: mnt_usb_msg_rcv
Description: rcv infomation from usb module msg queue
Input: NONE
Output: pData : Point to the data will be received
        pnLen : The length of the data
Return: EN_OK :success
Others:
History:
************************************************************/
EN_ERROR_NUM mnt_usb_msg_rcv(char *pData,int *pnLen)
{
    EN_ERROR_NUM eError;
    struct message  msg;
    char *pRcvData;

    XL_CHECK_NULL(pData);
    XL_CHECK_NULL(pnLen);

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
        xl_decode_u32((CHAR *)pRcvData,(ULONG *)&nLen);
        pRcvData += sizeof(U32);
        memcpy(pData,pRcvData,nLen);
        *pnLen = nLen;
    }
    return eError;
}


