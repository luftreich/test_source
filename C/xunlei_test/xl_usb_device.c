/**********************************************************
Copyright (C), 2003-2013, Xunlei Network Tech. Ltd.
File Name:xl_usb_device.c
Author:     hanshaoyang
Version:    1.0
Date:        2013-11-11
Description:  实现迅雷软件的linux平台usb检测功能
History:
    1. Date:2013-11-11
       Author:hanshaoyang
       Modification:初步实现功能
***********************************************************/

#include "xl_common.h"
#include "xl_mnt_mng.h"
#include "xl_mnt_usb.h"

#define UEVENT_BUFFER_SIZE 2048
#define USB_DEV_NAME "/dev/sd"
#define DEV_CUT_NAME "sd"

/* --- local struct --- */
typedef enum tagEN_USB_DETECT_STATE
{
	EN_USB_STANDBY = 0,
	EN_USB_GET_ACTION =1,
	EN_USB_IDENTIFY_DEVICE,
	EN_USB_GET_DEV_NAME,
	EN_USB_DETECT_END
}EN_USB_DETECT_STATE;

struct luther_gliethttp {
    const char *action;
    const char *path;
    const char *subsystem;
    const char *firmware;
    const char *devname;
    const char * physdevdriver;
    const char * product;
    int major;
    int minor;
    int type;
};

typedef struct  tagSt_xl_USB_Dev
{
    St_xl_storage_info m_sStorageInfo;
    void * m_pNext;
}St_xl_USB_Dev;

/* --- local  variable --- */
static char g_DevID[128] = {0};
St_xl_USB_Dev * g_psUSBDevHead = NULL;

/* --- local functions --- */
static void parse_event(const char *msg, struct luther_gliethttp *luther_gliethttp);
static EN_ERROR_NUM mnt_usb_insert_dev_node(St_xl_storage_info * psUSBInfo);
static EN_ERROR_NUM mnt_usb_delete_dev_node(St_xl_storage_info * psUSBInfo);
static EN_ERROR_NUM mnt_usb_get_action (struct luther_gliethttp *luther_gliethttp,EN_ACTION * action);
static EN_ERROR_NUM  mnt_usb_remove_dev_node(void);

/***********************************************************
Function: init_hotplug_sock
Description: initialize the hotplug socket
Input: NONE
Output:
Return: return a socket descriptor.
Others:
History:
************************************************************/
static int init_hotplug_sock(void)
{
    const int buffersize = 2048;
    int ret;

    struct sockaddr_nl snl;
    bzero(&snl, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

    int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s == -1)
    {
        perror("socket");
        return -1;
    }

    setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

    ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
    if (ret < 0)
    {
        perror("bind");
        close(s);
        return -1;
    }

    return s;
}
static void * mnt_usb_probe_device_thread(void * tid)
{
    int ret = 1;
    int probe_cnt = 50;
    
    printf("Entry mnt_usb_probe_device_thread \n");
    while(1)
    {
        ret = mnt_usb_device_probe();
        if (ret == 0 )
        {
            printf("device probe usb disk over,return 0 \n");
            break;
        }
        else
        {
            if (probe_cnt > 0)
            {
                usleep(1000*100);
                probe_cnt--;
                continue;
            }
        }
    }
    *(pthread_t *)tid = 0;
    printf(" Mnt_usb_probe_device_thread exit \n");
    return NULL;
}
/***********************************************************
Function: mnt_usb_detect_thread
Description:
Input:
Output:
Return:
Others:
History:
************************************************************/
void * mnt_usb_detect_thread(void)
{
    int hotplug_sock = init_hotplug_sock();
    struct luther_gliethttp luther_gliethttp;
    static pthread_t tid = 0;    
    static EN_USB_DETECT_STATE eSTATE = EN_USB_GET_ACTION;
    static EN_ACTION eAction_now = EN_USB_CHG,eAction_pre = EN_USB_CHG;
    St_xl_USB_Info sUSBInfo;
    EN_ERROR_NUM eError ;
    static int probe_cnt = 10;
    int ret = 0;

    memset(&sUSBInfo,0,sizeof(sUSBInfo));
    XL_DEBUG(EN_PRINT_INFO,"wait here to detect the usb hotplug,eState %d\n",eSTATE);
    
    while(1)
    {
        /* Netlink message buffer */
        char buf[UEVENT_BUFFER_SIZE * 2] = {0};
        memset (&luther_gliethttp,0,sizeof(luther_gliethttp));
        recv(hotplug_sock, &buf, sizeof(buf), 0);
        XL_DEBUG(EN_PRINT_INFO,"Start to detect the usb hotplug,eState %d\n",eSTATE);

       
#if 0
        int i; 
        printf("\n -------------------------------------------------------\n");
        for (i =0;i<sizeof(buf);i++)
        {
            printf("%c",buf[i]);
        }
        printf("\n-------------------------------------------------------end \n");
#endif
        /*1.前一次ACTION 是 ADD,后一次变成了Remove，则进入发送UNPLUG的消息*/
        /*2.如果 devmajor_pre == devmajor_now ,但是devminor_pre != devminor_now,证明新一轮的检测开始了，进入EN_USB_GET_ACTION*/
        parse_event(buf,&luther_gliethttp);

        eError = mnt_usb_get_action(&luther_gliethttp,&eAction_now);
        if (EN_OK == eError)
        {
            if ( EN_USB_CHG == eAction_now)
            {
                eAction_now = EN_USB_ADD;
            }            
            /*如果ACTION变了说明新的一次插入、或拔出开始了*/
            if(eAction_now != eAction_pre )
            {
                eSTATE = EN_USB_GET_ACTION;
            }
        }
        else
        {
            XL_DEBUG(EN_PRINT_WARN,"USB detect thread can't get the usb plug action \n");
            continue;
        }

        switch (eSTATE)
        {
            /*第一步先判断是插入还是拔出*/
            case EN_USB_GET_ACTION:
            {
                memset(&sUSBInfo,0,sizeof(sUSBInfo));
                eSTATE = EN_USB_IDENTIFY_DEVICE;
                eAction_pre = eAction_now;
            }
            break;
            case EN_USB_IDENTIFY_DEVICE:
            {
                
                if (EN_USB_ADD == eAction_now)
                {
                    if (tid == 0)
                    {
                        pthread_create(&tid,NULL,(void*)mnt_usb_probe_device_thread,(void *)&tid);
                        eSTATE = EN_USB_STANDBY;
                    }
                }
                
            
                if (EN_USB_RM == eAction_now)
                {
                    if (tid != 0)
                    {
                        pthread_cancel(tid);
                        pthread_join(tid,NULL);
                        tid = 0;
                    }

		    ret = mnt_usb_remove_dev_node();
		    if (ret == 0 )
		    {
                        eSTATE = EN_USB_STANDBY;
		    }
		    else
                    {
                        if (probe_cnt > 0)
                        {
                            probe_cnt--;
                            usleep(1000*100);
			}
			else
			{
                            eSTATE = EN_USB_STANDBY;
			    probe_cnt = 10;
			}
		    }
		}
            }
            break;
            default:
            break;
        }

    }
    return 0;
}

/***********************************************************
Function: parse_event
Description: To Parse the system msg from kernel about the usb hotplug event.
             The message is also record in /sys/block/sda/uevent
Input: msg: usb msg from kernel
Output: luther_gliethttp:restore the infomation we need from system msg
Return: none
Others:
History:
************************************************************/
static void parse_event(const char *msg, struct luther_gliethttp *luther_gliethttp)
{
    luther_gliethttp->action = "";
    luther_gliethttp->path = "";
    luther_gliethttp->subsystem = "";
    luther_gliethttp->firmware = "";
    luther_gliethttp->major = -1;
    luther_gliethttp->minor = -1;
    luther_gliethttp->product = "";

    while (*msg)
    {
        if (!strncmp(msg, "ACTION=", 7))
        {
            msg += 7;
            luther_gliethttp->action = msg;
        }
        else if (!strncmp(msg, "SUBSYSTEM=", 10))
        {
            msg += 10;
            luther_gliethttp->subsystem = msg;
        }
        else if (!strncmp(msg,"PHYSDEVDRIVER=",14))
        {
            msg += 14;
            luther_gliethttp->physdevdriver = msg;
        }
        else if (!strncmp(msg,"DEVNAME=",8))
        {
            msg += 8;
            luther_gliethttp->devname = msg;
            XL_DEBUG(EN_PRINT_DEBUG," echo devname from kernel info %s \n",luther_gliethttp->devname);
        }
        else if (!strncmp(msg,"PRODUCT=",8))
        {
            msg += 8;
            luther_gliethttp->product = msg;
        }
        while(*msg++);
    }
}

static EN_ERROR_NUM mnt_usb_get_action (struct luther_gliethttp *luther_gliethttp,EN_ACTION * action)
{
	if(!strcmp("add",luther_gliethttp->action))
	{
        *action = EN_USB_ADD;
		return EN_OK;
	}

	if(!strcmp("remove",luther_gliethttp->action))
	{
        *action = EN_USB_RM;
		return EN_OK;
	}

	if(!strcmp("change",luther_gliethttp->action))
	{
        *action = EN_USB_CHG;
		return EN_OK;
	}

    return EN_ERROR_UNKNOW;
}
#if 0
static EN_USB_DETECT_STATE mnt_usb_info_analysis(struct luther_gliethttp *luther_gliethttp,St_xl_USB_Info * psUSBInfo,EN_ACTION eAction_now)
{
    static char subsystem_now[32]={0};
    static char subsystem_pre[32]={0};
    static  char szProduct[32] = {0};
    EN_USB_DETECT_STATE eState = EN_USB_IDENTIFY_DEVICE;
    bool bIsEnableSnd = TRUE;
    strcpy(subsystem_now,luther_gliethttp->subsystem);
    strcpy (szProduct,luther_gliethttp->product);
#if 0
    mnt_usb_get_device_id(szProduct,&(psUSBInfo->m_nVid),&(psUSBInfo->m_nPid),&(psUSBInfo->m_nDid));
#endif
    if (EN_USB_RM == eAction_now)
    {
        XL_DEBUG(EN_PRINT_DEBUG ," RM: subsystem : %s \n",subsystem_now);
    }

    if ( EN_USB_ADD == eAction_now )
    {
    	/*判断是否是storage设备*/
    	if(!strcmp ("usb",subsystem_now))
    	{
    		strcpy(subsystem_pre,subsystem_now);
    	}
    	else if(!strcmp ("scsi_host",subsystem_now)&&!strcmp ("usb",subsystem_pre))
    	{
    		eState = EN_USB_GET_DEV_NAME;
    		bIsEnableSnd = TRUE;
    	}
    	else if(!strcmp ("usb_device",subsystem_now)&&!strcmp ("usb",subsystem_pre))
    	{
    		eState = EN_USB_STANDBY;
    		bIsEnableSnd = TRUE;
    	}
    	strcpy(subsystem_pre,subsystem_now);
    }

    if ( EN_USB_RM == eAction_now )
    {
        printf("\n");
        printf("  - - - - - - - >subsystem_now %s ,subsystem_pre %s <- - - - - - - -\n",subsystem_now,subsystem_pre);
        printf("\n");
    	
//    	if(!strcmp ("block",subsystem_now)&&!strcmp ("scsi_disk",subsystem_pre))
    	{
    		eState = EN_USB_GET_DEV_NAME;
    		bIsEnableSnd = TRUE;
    	}
/*
        if(!strcmp ("usb_device",subsystem_pre)&&!strcmp ("usb",subsystem_now))
    	{
    		eState = EN_USB_STANDBY;
    		bIsDiskDev = FALSE;
    		bIsEnableSnd = TRUE;
    	}
*/
    	strcpy(subsystem_pre,subsystem_now);
    }

    if (TRUE == bIsEnableSnd)
    {
    	psUSBInfo->eAction = eAction_now;
        /*
        if (FALSE == bIsDiskDev)
    	{
    		psUSBInfo->eType= EN_USB_DEVICE_UNKNOW;
            mnt_msg_send_usbinfo_all(psUSBInfo);
            printf("plug out unknow device\n");
    	}
    	else 
        */
    	{
            psUSBInfo->eType= EN_USB_DEVICE_STORAGE;
    	}
    	XL_DEBUG(EN_PRINT_INFO,"XL_DBG: Print USB info\n");
    	mnt_usb_print_usbinfo(psUSBInfo);
    }
    return eState;
}
#endif
static int mnt_usb_create_disk_id(void)
{
    int i=0;
    for(i = i;i<128;i++)
    {
        if (g_DevID[i] == FALSE)
        {
            g_DevID[i] = TRUE;
            break;
        }
    }

    if (128 <= i)
    {
        return -1;
    }
    return i+1;
}
static void  mnt_usb_destroy_disk_id(int nDevId)
{
    int n;
    n = nDevId;
    if (g_DevID[n-1] == TRUE)
    {
        g_DevID[n-1] = FALSE;
    }
}

#if 0
static void mnt_usb_get_device_id(char * szProduct,int * pnVid,int * pnPid,int * pnDid)
{
    char des[8] = {0};
    int len = 0,i=0;
    int nValue[3];
    char *pos, *str;

    if (!strcmp("",szProduct))
    {
        return ;
    }
    str = szProduct;
    while(str)
    {
        memset(des,0,sizeof(des));
        pos = strchr(str,'/');
        if (NULL == pos)
        {
            memcpy(des,str,strlen(str));
            sscanf(des,"%x",&nValue[2]);
            break;
        }
        len = pos -str;
        memcpy(des,str,len);
        sscanf(des,"%x",&nValue[i]);
        i++;
        str = pos+1;
    }
    *pnVid = nValue[0];
    *pnPid = nValue[1];
    *pnDid = nValue[2];
}

static void mnt_usb_print_usbinfo(St_xl_USB_Info * sUSBInfo)
{
    printf(" fun %s   start ... ... ... \n",__FUNCTION__);
    printf("        ------------------------>action %d type %d nDiskId %d \n",sUSBInfo->eAction,sUSBInfo->eType,sUSBInfo->nDiskId);
    printf(" fun %s   end ... ... ... \n",__FUNCTION__);
}

static EN_ERROR_NUM  mnt_usb_create_storage(struct luther_gliethttp *pluther_gliethttp,St_xl_storage_info * pStorageInfo)
{
    St_xl_storage_info * p_sStorageInfo = NULL;

    XL_CHECK_NULL(pluther_gliethttp);
    XL_CHECK_NULL(pStorageInfo);
    p_sStorageInfo = pStorageInfo;
    memset(p_sStorageInfo,0,sizeof(St_xl_storage_info));
    if (EN_OK == mnt_usb_filter_disk_name(pluther_gliethttp,pStorageInfo->szDiskName))
    {
        p_sStorageInfo = pStorageInfo;
        p_sStorageInfo->nDiskId= mnt_usb_create_disk_id();
        XL_DEBUG(EN_PRINT_DEBUG,"p_sStorageInfo->nDiskId  %d \n",p_sStorageInfo->nDiskId);
    }
    else
    {
        return EN_ERROR_UNKNOW;
    }
    return EN_OK;
}

/***********************************************************
Function: mnt_usb_filter_disk_name
Description: filter disk name from kernel log message
Input:pluther_gliethttp:the kernel msg filted infomation context
Output:szDiskName:To store the usb disk name
Return:
Others:
History:
************************************************************/
static EN_ERROR_NUM mnt_usb_filter_disk_name(struct luther_gliethttp *pluther_gliethttp, char * szDiskName)
{
    
    XL_CHECK_NULL(pluther_gliethttp);

    if (pluther_gliethttp->devname)
    {
        if (strlen(pluther_gliethttp->devname))
        {
            if(!strncmp(pluther_gliethttp->devname,"sd",2))
            {
                strncpy(szDiskName,pluther_gliethttp->devname,3);
                return EN_OK;
            }
            else
            {
                return EN_ERROR_UNKNOW;
            }
        }
        else
        {
            return EN_ERROR_UNKNOW;
        }
    }
    else
    {
        return EN_ERROR_UNKNOW;
    }
}

/***********************************************************
Function: mnt_usb_find_dev_node
Description: To find a USB info node from list g_psUSBDevHead
Input: szDevName :USBDisk Name, such as "/dev/sda"
Output: psStorage: get the sStorage info from list
Return: EN_OK :success
        others :failed
Others:
History:
************************************************************/
static EN_ERROR_NUM  mnt_usb_find_dev_node(char *szDevName,St_xl_storage_info *psStorage)
{
    St_xl_USB_Dev * psUSBDevItem;
    EN_ERROR_NUM eError = EN_ERROR_FAILED;

    XL_CHECK_NULL(psStorage);
    XL_CHECK_NULL(szDevName);

    psUSBDevItem = g_psUSBDevHead;
    if (NULL == psUSBDevItem)
    {
        XL_DEBUG(EN_PRINT_ERROR,"the USB Dev list is NULL \n");
        eError = EN_ERROR_FAILED;
    }

    while(psUSBDevItem)
    {
        XL_DEBUG(EN_PRINT_DEBUG,"\n");
        if (!strcmp(szDevName,psUSBDevItem->m_sStorageInfo.szDiskName))
        {
            XL_DEBUG(EN_PRINT_INFO,"Found a usb info node from list \n");
            psStorage->nDiskId = psUSBDevItem->m_sStorageInfo.nDiskId;
            strcpy(psStorage->szDiskName,psUSBDevItem->m_sStorageInfo.szDiskName);
            eError = EN_OK;
            break;
        }
        else
        {
            psUSBDevItem = psUSBDevItem->m_pNext;
            XL_DEBUG(EN_PRINT_INFO,"Finding a usb info node from list, DevName  %s \n",szDevName);
        }
    }
    XL_DEBUG(EN_PRINT_DEBUG,"\n");
    return eError;
}
#endif
/***********************************************************
Function: mnt_usb_insert_dev_node
Description: Insert a usb info node into list g_psUSBDevHead
Input: psUSBInfo: a point to a struct St_xl_USB_Info
Output: NONE
Return: EN_OK :successfully
        others: failed
Others:
History:
************************************************************/
static EN_ERROR_NUM mnt_usb_insert_dev_node(St_xl_storage_info * psStorageInfo)
{
    St_xl_USB_Dev *psTmpUSBDev;
    St_xl_USB_Dev *psUSBDevItem;

    XL_CHECK_NULL(psStorageInfo);


    psTmpUSBDev = malloc(sizeof(St_xl_USB_Dev));
    if (NULL == psTmpUSBDev)
    {
        XL_DEBUG(EN_PRINT_ERROR,"No memory here\n");
        return EN_ERROR_NO_MEMORY;
    }
    else
    {
        memset(psTmpUSBDev,0,sizeof(St_xl_USB_Dev));
    }

    psTmpUSBDev->m_sStorageInfo.nDiskId = psStorageInfo->nDiskId;
    if (NULL != psStorageInfo->szDiskName)
    {
        strcpy(psTmpUSBDev->m_sStorageInfo.szDiskName,psStorageInfo->szDiskName);
    }

    if (NULL == g_psUSBDevHead)
    {
        g_psUSBDevHead = psTmpUSBDev;
    }
    else
    {
        psUSBDevItem = g_psUSBDevHead;
        while(psUSBDevItem->m_pNext)
        {
            psUSBDevItem = psUSBDevItem->m_pNext;
        }
        psUSBDevItem->m_pNext = psTmpUSBDev;
    }
    return EN_OK;
}
static EN_ERROR_NUM mnt_usb_delete_dev_node(St_xl_storage_info *psStorageInfo)
{
    St_xl_USB_Dev *psUSBDevItem,*psUSBDevPreItem;
    EN_ERROR_NUM eError;
    XL_CHECK_NULL(psStorageInfo);


    if (NULL == g_psUSBDevHead)
    {
       return EN_OK;
    }
    else
    {
        psUSBDevItem = g_psUSBDevHead;
        do{
            if (psStorageInfo->nDiskId == psUSBDevItem->m_sStorageInfo.nDiskId)
            {
                mnt_usb_destroy_disk_id(psStorageInfo->nDiskId);
                if (g_psUSBDevHead == psUSBDevItem)
                {
                    g_psUSBDevHead = psUSBDevItem->m_pNext;
                }
                else
                {
                    psUSBDevPreItem->m_pNext = psUSBDevItem->m_pNext;

                }
                XL_DEBUG(EN_PRINT_DEBUG,"\n");
                free(psUSBDevItem);
                eError = EN_OK;
                break;
            }
            else
            {
                psUSBDevPreItem = psUSBDevItem;
                psUSBDevItem = psUSBDevItem->m_pNext;
                eError = EN_ERROR_FAILED;
            }
        }while(psUSBDevItem);
        XL_DEBUG(EN_PRINT_DEBUG,"\n");

    }


    return eError;
}

EN_ERROR_NUM mnt_usb_get_disk_name(U32 nUSBDiskID,char * szDiskName)
{
    St_xl_USB_Dev * psUSBDevItem;

    XL_CHECK_NULL(szDiskName);

    if (0 > nUSBDiskID)
    {
        return EN_ERROR_FAILED;
    }

    if (NULL == g_psUSBDevHead)
    {
        return EN_ERROR_FAILED;
    }
    else
    {
        psUSBDevItem = g_psUSBDevHead;
        do
        {
            if (psUSBDevItem->m_sStorageInfo.nDiskId == nUSBDiskID)
            {
                strcpy(szDiskName,psUSBDevItem->m_sStorageInfo.szDiskName);
                break;
            }
            else
            {
                psUSBDevItem = psUSBDevItem->m_pNext;
            }
        }while(psUSBDevItem);

        if (NULL == psUSBDevItem)
        {
            return EN_ERROR_FAILED;
        }
    }
    return EN_OK;

}


/***********************************************************
Function:mnt_usb_check_disk_id
Description:To check if the usb disk has been inserted into board
Input: nUSBDiskID
Output: none
Return: EN_OK:The usb disk has been inserted
        EN_ERROR_FAILED: no such usbdisk
Others:
History:
************************************************************/
EN_ERROR_NUM mnt_usb_check_disk_id(U32 nUSBDiskID)
{
    St_xl_USB_Dev * psUSBDevItem;
    EN_ERROR_NUM eError = EN_ERROR_FAILED;

    if (NULL == g_psUSBDevHead)
    {
        return EN_ERROR_FAILED;
    }
    else
    {
        psUSBDevItem = g_psUSBDevHead;
        do
        {
            if (ALL_USB_DISK == nUSBDiskID)
            {
                if (psUSBDevItem->m_sStorageInfo.nDiskId > 0)
                {
                    eError = EN_OK;
                    break;
                }
                else
                {
                    psUSBDevItem = psUSBDevItem->m_pNext;
                    continue;
                }
            }
            else
            {
                if (psUSBDevItem->m_sStorageInfo.nDiskId == nUSBDiskID)
                {
                    eError = EN_OK;
                    break;
                }
                else
                {
                    psUSBDevItem = psUSBDevItem->m_pNext;
                }
            }
        }while(psUSBDevItem);

        if (NULL == psUSBDevItem)
        {
            return EN_ERROR_FAILED;
        }
    }
    return eError;

}


/***********************************************************
Function: mnt_usb_device_probe
Description: To probe the usb device in the init stage.
Input: NONE
Output: NONE
Return: NONE
Others:
History:
************************************************************/
int mnt_usb_device_probe(void)
{
    St_xl_USB_Info  sUSBInfo;
    St_xl_storage_info sStorage;
    char szDiskName[16] = {0};
    char szDev[16] = {0};
    char c = 'a';
    int i;
    int ret = 1; 
    XL_DEBUG(EN_PRINT_DEBUG,"Entry \n");
    for(i=0;i<16;i++)
    {
        sprintf(szDiskName,"%s%c",USB_DEV_NAME,c);
        sprintf(szDev,"%s%c",DEV_CUT_NAME,c);
        if(access(szDiskName, F_OK) == 0)
        {/*1.前一次ACTION 是 ADD,后一次变成了Remove，则进入发送UNPLUG的消息*/
            sUSBInfo.eAction = EN_USB_ADD;
            sUSBInfo.eType = EN_USB_DEVICE_STORAGE;
            sUSBInfo.nDiskId = mnt_usb_create_disk_id();
            sStorage.nDiskId = sUSBInfo.nDiskId;
            strcpy(sStorage.szDiskName,szDev);

            if (EN_OK != mnt_usb_insert_dev_node(&sStorage))
            {
                XL_DEBUG(EN_PRINT_ERROR,"Can't insert the usb info node into the device list\n");
            }
//            mnt_msg_send_usbinfo_all(&sUSBInfo);
            mnt_usb_msg_send((char *)&(sUSBInfo),sizeof(sUSBInfo));
	    ret = 0;
        }
        c += 1;
        memset(szDiskName, 0 ,sizeof(szDiskName));
        memset(szDev, 0 ,sizeof(szDev));
    }
    
    for(i=0;i<16;i++)
    {
        sprintf(szDiskName,"%s%d","/dev/mmcblk",i);
        sprintf(szDev,"%s%d","mmcblk",i);
        if(access(szDiskName, F_OK) == 0)
        {/*1.前一次ACTION 是 ADD,后一次变成了Remove，则进入发送UNPLUG的消息*/
            sUSBInfo.eAction = EN_USB_ADD;
            sUSBInfo.eType = EN_USB_DEVICE_STORAGE;
            sUSBInfo.nDiskId = mnt_usb_create_disk_id();
            sStorage.nDiskId = sUSBInfo.nDiskId;
            strcpy(sStorage.szDiskName,szDev);

            if (EN_OK != mnt_usb_insert_dev_node(&sStorage))
            {
                XL_DEBUG(EN_PRINT_ERROR,"Can't insert the usb info node into the device list\n");
            }
//            mnt_msg_send_usbinfo_all(&sUSBInfo);
            mnt_usb_msg_send((char *)&(sUSBInfo),sizeof(sUSBInfo));
	    ret = 0;
        }
        memset(szDiskName, 0 ,sizeof(szDiskName));
        memset(szDev, 0 ,sizeof(szDev));
    }

    XL_DEBUG(EN_PRINT_DEBUG,"Exit \n");
    return ret;
}

/***********************************************************
Function: mnt_usb_remove_dev_node
Description: To remove the unplug usb device
Input: NONE
Output: NONE
Return: NONE
Others:
History:
************************************************************/
static EN_ERROR_NUM  mnt_usb_remove_dev_node(void)
{
    St_xl_USB_Dev * psUSBDevItem;
    St_xl_USB_Info sUSBInfo;
    St_xl_storage_info sStorage;
    char dev_name[16] = {0};
    char szDevName [16] = {0};
    int ret = 1;

    psUSBDevItem = g_psUSBDevHead;
    if (NULL == psUSBDevItem)
    {
        XL_DEBUG(EN_PRINT_ERROR,"the USB Dev list is NULL \n");
        return 0;
    }

    while(psUSBDevItem)
    {
        XL_DEBUG(EN_PRINT_DEBUG,"\n");
        strcpy(szDevName,psUSBDevItem->m_sStorageInfo.szDiskName);
        sprintf(dev_name,"/dev/%s",szDevName);
        
        if (access(dev_name,F_OK) != 0)
        {
            sUSBInfo.eAction = EN_USB_RM;
            sUSBInfo.eType = EN_USB_DEVICE_STORAGE;
            sUSBInfo.nDiskId = psUSBDevItem->m_sStorageInfo.nDiskId;
            
            sStorage.nDiskId = sUSBInfo.nDiskId;
            strcpy(sStorage.szDiskName,szDevName);

            mnt_usb_msg_send((char *)&(sUSBInfo),sizeof(sUSBInfo));
            mnt_usb_delete_dev_node(&sStorage);
            ret = 0;
        }
    
        psUSBDevItem = psUSBDevItem->m_pNext;
        memset(dev_name,0,sizeof(dev_name));
        memset(szDevName,0,sizeof(szDevName));
    }
    
    return ret;
}

