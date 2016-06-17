/**********************************************************
Copyright (C), 2003-2013, Xunlei Network Tech. Ltd.
File Name:xl_usb_mng.c
Author:     hanshaoyang
Version:    1.0
Date:        2013-11-26
Description:  ����USB����ز���
History:
    1. Date:2013-11-26
       Author:hanshaoyang
       Modification:����ʵ�ֹ���
***********************************************************/
#include "xl_common.h"
#include "xl_mnt_usb.h"
#include "xl_mnt_command.h"
#include "xl_mnt_mng.h"

typedef struct  tagSt_xl_USB_Part
{
    St_xl_partition_info m_sStorageInfo;
    void * m_pNext;
}St_xl_USB_Part;

typedef struct tagSt_usb_query_thread
{
    St_xl_USB_Info * pUsbInfo;
    pthread_t tid;
    int nRunState;
    EN_MNT_USB_STATE detect_stat;
}St_usb_query_thread;

/* --- local macro --- */
#define MOUNT_FILE "/proc/mounts"
#define MOUNT_LOG_FILE "/tmp/_mount_log"
#define MNT_USB_MOUNT_FILE "/proc/mounts"
#define STDOUT (1)
#define EXT2_SUPER_MAGIC    (0xEF53)
#define EXT3_SUPER_MAGIC    (0xEF53)
#define JFFS2_SUPER_MAGIC   (0x72b6)
#define MSDOS_SUPER_MAGIC   (0x4d44)
#define NTFS_SB_MAGIC       (0x5346544e)
#define UBIFS_SUPER_MAGIC   (0x24051905)
#define YAFFS_MAGIC         (0x5941FF53)
#define NTFS_MAGIC          (0x65735546)
#define BUF_LEN (10*sizeof(struct inotify_event))
/* --- local variable --- */
static pthread_t s_tid[3] = {0};
static BOOL bInit = FALSE;
static St_xl_USB_Part *s_psPartHead = NULL;
static St_usb_query_thread s_UsbThread[128];
/* --- local functions --- */
EN_ERROR_NUM mnt_usb_unmount_disk(U32 nUSBDiskID);
static EN_ERROR_NUM mnt_usb_get_mount_path(char* szDevName,char *szPartName,char *szMountDir,int *nMapIndex);
static EN_ERROR_NUM parsing_mount_log(char *buf,char *szPartName,char *szMountDir);
static EN_ERROR_NUM mnt_usb_create_mount_log(char *szDiskName,int nDiskId);
static EN_ERROR_NUM mnt_usb_get_label(char *szDir,const char *szPartName,char *szVolume);
static EN_ERROR_NUM fat32_get_label(const char *szPartName,char *szVolume);
static U32 mnt_usb_get_partition_count(U32 nUSBDiskID);
static U32 mnt_sdcard_get_partition_count(U32 nUSBDiskID);
static void * mnt_usb_probe_partition_thread(void);
static EN_ERROR_NUM mnt_usb_read_partition_from_list(St_xl_partition_info *psPartitionInfo,U32 nUSBDiskID,U32 nPartCnt);
static EN_ERROR_NUM mnt_sdcard_read_partition_from_list(St_xl_partition_info *psPartitionInfo,U32 nUSBDiskID,U32 nPartCnt);
static EN_ERROR_NUM mnt_usb_insert_partition_list(St_xl_partition_info *psPartition,int nCnt);
static unsigned long get_file_size(const char *path);
static EN_ERROR_NUM mnt_usb_detect_partition(U32 nDiskID);
static void * mnt_usb_query_mount_log_thread (void * pUSBInfo);
static int mnt_usb_get_partition_cnt(char * dev_name,int * nMaxPartIndex);
static EN_ERROR_NUM ext3_get_label(const char *szPartName,char *szVolume);
static EN_ERROR_NUM ntfs_get_label(const char *szPartName,char *szVolume);
EN_ERROR_NUM mnt_usb_send_partition(int nFrom,int nTo,int nSerial,int nDiskId);
EN_ERROR_NUM mnt_usb_send_partition_all(void);

/* --- extern functions --- */
extern int ntfs3g_getlabel(const char *szDevName,char *szVolume);
extern int g_usb_plugout;

/***********************************************************
Function:mnt_usb_init
Description:The initialize function of usb module
Input: NONE
Output: NONE
Return: EN_OK: init usb module successfully
        others: init usb module failed
Others:
History:
************************************************************/
EN_ERROR_NUM mnt_usb_init(void)
{
    EN_ERROR_NUM eError = EN_ERROR_UNKNOW;
    int nErr;

    if (FALSE == bInit)
    {

        
        memset (&s_UsbThread,0,sizeof(s_UsbThread));
        nErr = pthread_create(&s_tid[0],NULL,(void *)mnt_usb_probe_partition_thread,NULL);
        if(nErr != 0)
        {
             XL_DEBUG(EN_PRINT_ERROR,"pthread_create : msg_queue_route error :%s \n",strerror(errno));
             eError = EN_ERROR_UNKNOW;
        }
        else
        {
            XL_DEBUG(EN_PRINT_INFO,"create pthread succussfully ,tid  0x%x\n",(int)s_tid[0]);
            eError = EN_OK;
        } 

        nErr = pthread_create(&s_tid[1],NULL,(void *)mnt_usb_detect_thread,NULL);
        if(nErr != 0)
        {
             XL_DEBUG(EN_PRINT_ERROR,"pthread_create : msg_queue_route error :%s \n",strerror(errno));
             eError = EN_ERROR_UNKNOW;
        }
        else
        {
            XL_DEBUG(EN_PRINT_INFO,"create pthread succussfully ,tid  0x%x\n",(int)s_tid[1]);
            eError = EN_OK;
        }
        
        mnt_usb_device_probe();

        bInit = TRUE;
    }
    return eError;
}
/***********************************************************
Function:mnt_usb_term
Description:The term function relative to mnt_usb_init
Input: NONE
Output: NONE
Return: EN_OK: term usb module successfully
        others: term usb module failed
Others:
History:
************************************************************/
EN_ERROR_NUM mnt_usb_term(void)
{
    if (TRUE == bInit)

    {
        if (0 != s_tid[0])
        {
            pthread_cancel(s_tid[0]);
            pthread_join(s_tid[0],NULL);
        }

        if (0 != s_tid[1])
        {
            pthread_cancel(s_tid[1]);
            pthread_join(s_tid[1],NULL);
        }
        bInit = FALSE;
    }
    return EN_OK;
}

EN_ERROR_NUM mnt_sdcard_read_partition(U32 nUSBDiskID,U32 *pnCnt,St_xl_partition_info **pDestData)
{
    U32 nPartitionCnt = 0;
    St_xl_partition_info *pTmpPtr;
    EN_ERROR_NUM eError;


    eError =  mnt_usb_check_disk_id(nUSBDiskID);
    if (EN_OK != eError)
    {
        *pnCnt = 0;
        *pDestData = NULL;
        return EN_ERROR_UNKNOW;
    }

    XL_DEBUG(EN_PRINT_INFO, "mnt_usb_read_partition  USBDiskID %d Input start \n",nUSBDiskID);
    nPartitionCnt = mnt_sdcard_get_partition_count(nUSBDiskID);
    if (0 == nPartitionCnt)
    {
        XL_DEBUG(EN_PRINT_ERROR,"Get Partition count failed ,nPartition cnt %d\n",nPartitionCnt);
        *pnCnt = nPartitionCnt;
        *pDestData = NULL;
        return EN_ERROR_FAILED;
    }
    else
    {
        XL_DEBUG(EN_PRINT_DEBUG,"Get Partition count %d \n",nPartitionCnt);
    }

    /*4 create partitions data */
    if (0 < nPartitionCnt)
    {
        pTmpPtr = malloc (sizeof(St_xl_partition_info)*nPartitionCnt);
        XL_DEBUG(EN_PRINT_DEBUG,"Alloc memory get addr 0x%x \n",(int)pTmpPtr);
        if (NULL == pTmpPtr)
        {
            XL_DEBUG(EN_PRINT_ERROR,"malloc memory failed \n");
            *pDestData = pTmpPtr;
            return EN_ERROR_NO_MEMORY;
        }
        memset (pTmpPtr,0,sizeof(sizeof(St_xl_partition_info)*nPartitionCnt));
        eError = mnt_sdcard_read_partition_from_list(pTmpPtr,nUSBDiskID,nPartitionCnt);
        if (EN_OK == eError)
        {
            XL_DEBUG(EN_PRINT_DEBUG,"Read partition infomation successfully\n");
        }
        else
        {
            XL_DEBUG(EN_PRINT_ERROR,"Read partition infomation failed\n");
        }
    }
    *pDestData = pTmpPtr;
    *pnCnt = nPartitionCnt;
    return eError;
}

static EN_ERROR_NUM mnt_sdcard_read_partition_from_list(St_xl_partition_info *psPartitionInfo,U32 nUSBDiskID,U32 nPartCnt)
{
    EN_ERROR_NUM eError = EN_ERROR_FAILED;
    St_xl_USB_Part *pTmpNode = NULL;
    int cnt = 0;
    pTmpNode = s_psPartHead;
    XL_CHECK_NULL(pTmpNode);
    XL_CHECK_NULL(psPartitionInfo);

    while(pTmpNode)
    {
        if(!strncmp("/dev/mmc",pTmpNode->m_sStorageInfo.szPartName,8))
        {
            if ((pTmpNode->m_sStorageInfo.nUSBDiskId == nUSBDiskID||(ALL_USB_DISK ==  nUSBDiskID))&&(pTmpNode->m_sStorageInfo.nUSBDiskId > 0))
            {
                memcpy(psPartitionInfo,&(pTmpNode->m_sStorageInfo),sizeof(St_xl_partition_info));
                psPartitionInfo++;
                cnt++;
                if (cnt > nPartCnt)
                {
                    XL_DEBUG(EN_PRINT_ERROR,"unable copy the partition infomation ,less memory\n");
                    eError = EN_ERROR_FAILED;
                    break;
                }
                else
                {
                    eError = EN_OK;
                }
            }
        } 
        pTmpNode = pTmpNode->m_pNext;
    }
    return eError;
}


/***********************************************************
Function:
Description: Read Partition infomation from list
Input:nUSBDiskID USB Disk ID
Output: pnCnt:Partition cnt
        pDestData :address used store the partiton memory data
Return: EN_OK :Get tht usb partition successfully
        EN_ERROR_UNKNOW: No usbdisk plug in
        EN_ERROR_NO_MEMORY: NO memory to store usb partiton infomation
Others: Need to free the pDestData after used this function
History:
************************************************************/
EN_ERROR_NUM mnt_usb_read_partition(U32 nUSBDiskID,U32 *pnCnt,St_xl_partition_info **pDestData)
{
    U32 nPartitionCnt = 0;
    St_xl_partition_info *pTmpPtr;
    EN_ERROR_NUM eError;


    eError =  mnt_usb_check_disk_id(nUSBDiskID);
    if (EN_OK != eError)
    {
        *pnCnt = 0;
        *pDestData = NULL;
        return EN_ERROR_UNKNOW;
    }

    XL_DEBUG(EN_PRINT_INFO, "mnt_usb_read_partition  USBDiskID %d Input start \n",nUSBDiskID);
    nPartitionCnt = mnt_usb_get_partition_count(nUSBDiskID);
    if (0 == nPartitionCnt)
    {
        XL_DEBUG(EN_PRINT_ERROR,"Get Partition count failed ,nPartition cnt %d\n",nPartitionCnt);
        *pnCnt = nPartitionCnt;
        *pDestData = NULL;
        return EN_ERROR_FAILED;
    }
    else
    {
        XL_DEBUG(EN_PRINT_DEBUG,"Get Partition count %d \n",nPartitionCnt);
    }

    /*4 create partitions data */
    if (0 < nPartitionCnt)
    {
        pTmpPtr = malloc (sizeof(St_xl_partition_info)*nPartitionCnt);
        XL_DEBUG(EN_PRINT_DEBUG,"Alloc memory get addr 0x%x \n",(int)pTmpPtr);
        if (NULL == pTmpPtr)
        {
            XL_DEBUG(EN_PRINT_ERROR,"malloc memory failed \n");
            *pDestData = pTmpPtr;
            return EN_ERROR_NO_MEMORY;
        }
        memset (pTmpPtr,0,sizeof(sizeof(St_xl_partition_info)*nPartitionCnt));
        eError = mnt_usb_read_partition_from_list(pTmpPtr,nUSBDiskID,nPartitionCnt);
        if (EN_OK == eError)
        {
            XL_DEBUG(EN_PRINT_DEBUG,"Read partition infomation successfully\n");
        }
        else
        {
            XL_DEBUG(EN_PRINT_ERROR,"Read partition infomation failed\n");
        }
    }
    *pDestData = pTmpPtr;
    *pnCnt = nPartitionCnt;
    return eError;
}

static EN_ERROR_NUM mnt_usb_read_partition_from_list(St_xl_partition_info *psPartitionInfo,U32 nUSBDiskID,U32 nPartCnt)
{
    EN_ERROR_NUM eError = EN_ERROR_FAILED;
    St_xl_USB_Part *pTmpNode = NULL;
    int cnt = 0;
    pTmpNode = s_psPartHead;
    XL_CHECK_NULL(pTmpNode);
    XL_CHECK_NULL(psPartitionInfo);

    while(pTmpNode)
    {
        if(!strncmp("/dev/sd",pTmpNode->m_sStorageInfo.szPartName,7))
        {
            if ((pTmpNode->m_sStorageInfo.nUSBDiskId == nUSBDiskID||(ALL_USB_DISK ==  nUSBDiskID))&&(pTmpNode->m_sStorageInfo.nUSBDiskId > 0))
            {
                memcpy(psPartitionInfo,&(pTmpNode->m_sStorageInfo),sizeof(St_xl_partition_info));
                psPartitionInfo++;
                cnt++;
                if (cnt > nPartCnt)
                {
                    XL_DEBUG(EN_PRINT_ERROR,"unable copy the partition infomation ,less memory\n");
                    eError = EN_ERROR_FAILED;
                    break;
                }
                else
                {
                    eError = EN_OK;
                }
            }
        } 
        pTmpNode = pTmpNode->m_pNext;
    }
    return eError;
}


/***********************************************************
Function: mnt_usb_delete_partitions_from_list
Description: delete the whole partitions in one disk from list
Input: USB Disk ID
Output: NONE
Return: EN_OK : successfully
        others:failed
Others:
History:
************************************************************/
static EN_ERROR_NUM mnt_usb_delete_partitions_from_list(U32 nUSBDiskID)
{
    St_xl_USB_Part *pTmpNode,*pTmpPreNode;
    St_xl_USB_Part *pNode;

    EN_ERROR_NUM eError = EN_ERROR_FAILED;
    
    XL_DEBUG(EN_PRINT_INFO, "+++++++++++++++++++++++++++++++++++++mnt_usb delete Input start ++++++++++++++++++++++++++++++\n");
    
    pTmpNode = s_psPartHead;
    pTmpPreNode = NULL;

    while(NULL != pTmpNode)
    {
        if (pTmpNode->m_sStorageInfo.nUSBDiskId == nUSBDiskID)
        {
            pNode = pTmpNode;
            if (s_psPartHead == pTmpNode)
            {
                s_psPartHead = pTmpNode->m_pNext;
                pTmpPreNode = NULL;
            }
            else
            {
                pTmpPreNode->m_pNext = pTmpNode->m_pNext;
            }
            free(pNode);
            pTmpNode = pTmpNode->m_pNext;
            mnt_ubus_usb_plug_notify();
        }
        else
        {
            pTmpPreNode = pTmpNode;
            pTmpNode = pTmpNode->m_pNext;
        }

    }
    XL_DEBUG(EN_PRINT_INFO,"  s_psPartHead is 0x%x\n",(int)s_psPartHead);
    return eError;
}

/***********************************************************
Function: mnt_usb_get_partition_cnt
Description: Get Partition cnt from the system file
Input: dev_name :the USB Disk name
Output: pMaxPartIndex: the max index of the partition id
Return: -1: Get an error partition count
        >=0 :Get a valid partition count
Others:
History:
************************************************************/
static int mnt_usb_get_partition_cnt(char * dev_name,int * pMaxPartIndex)
{
    int i = 0;
    int nCount = 0;
    int max_index = 0;
    DIR *pDirFd = NULL;
    char szUsbDirName[64] = {0};
    char szUsbPartDirName[64] = {0};
    int  nIndex = 0;
    
    /*To check if the device name is a valid device*/
    sprintf(szUsbDirName,"/sys/block/%s",dev_name);
    if((pDirFd =opendir(szUsbDirName)) == NULL )
    {
        XL_DEBUG(EN_PRINT_ERROR," no usb-storage device attatched ! opendir %s failed \r\n",szUsbDirName);
        return -1;
    }
    closedir(pDirFd);

    /*To check how many partitions in this device */
    nIndex = 0;
    if (!strncmp("sd",dev_name,2))
    {
        while (i < 64)
        {
            memset(szUsbPartDirName,0,sizeof(szUsbPartDirName));
            sprintf(szUsbPartDirName,"/dev/%s%d",dev_name,nIndex);
            if (access((const char *)szUsbPartDirName,F_OK)==0)
            {
                max_index = nIndex;
                nCount ++;
                XL_DEBUG(EN_PRINT_INFO,"Found Partition is %s \n",szUsbPartDirName); 
            }
            i++;
            nIndex++;
        }
    }
 
    if (!strncmp("mmcblk",dev_name,6))
    {
        while (i < 64)
        {
            memset(szUsbPartDirName,0,sizeof(szUsbPartDirName));
            sprintf(szUsbPartDirName,"/dev/%sp%d",dev_name,nIndex);
            if (access((const char *)szUsbPartDirName,F_OK)==0)
            {
                max_index = nIndex;
                nCount ++;
                XL_DEBUG(EN_PRINT_INFO,"Found Partition is %s \n",szUsbPartDirName); 
            }
            i++;
            nIndex++;
        }
    }
      
    *pMaxPartIndex = max_index;
    
    return nCount;
}

/***********************************************************
Function: mnt_usb_detect_partition
Description: To detect the infomation of one partition
Input: nDiskID:The USB Disk ID
Output: none
Return: EN_OK :successfully
        others: failed
Others:
History:
************************************************************/
static EN_ERROR_NUM mnt_usb_detect_partition(U32 nDiskID)
{
    St_xl_partition_info *psTmpPartition;
    void *pMalloc;

    int map_index = 0;
    EN_ERROR_NUM eError;
    int nPartitionCnt = 0;
    int nPartitionIndex = 0;
    int nMaxPartitionIndex = 1;
    char szDiskName[32] = {0};
    
    /*根据DISK ID 获取出其对应的硬盘名字*/
    eError = mnt_usb_get_disk_name(nDiskID,szDiskName);
    if (EN_OK != eError)
    {
        XL_DEBUG(EN_PRINT_ERROR,"unable get disk name \n");
        return eError;
    }

    /*根据mount命令生成一个log文件*/
    eError = mnt_usb_create_mount_log(szDiskName,nDiskID);
    if (EN_OK != eError)
    {
        return eError;
    }
    /*根据设备名称检查出有多少个partition*/
    nPartitionCnt = mnt_usb_get_partition_cnt(szDiskName,&nMaxPartitionIndex);
    if (0 >= nPartitionCnt)
    {
        XL_DEBUG(EN_PRINT_ERROR,"Unable to get the partition number of usb disk %s \n",szDiskName);
        return EN_ERROR_FAILED;
    }

    /*分配好partition信息保存的结构体空间*/
    psTmpPartition = malloc(nPartitionCnt*sizeof(St_xl_partition_info));
    if (NULL == psTmpPartition)
    {
        XL_DEBUG(EN_PRINT_ERROR,"no memory\n");
        return EN_ERROR_FAILED;
    }

    pMalloc = psTmpPartition;
    XL_DEBUG(EN_PRINT_INFO,"Found partition count is [%d], Disk Id is [%d] \n",nPartitionCnt,nDiskID);
    memset(psTmpPartition,0,sizeof(nPartitionCnt*sizeof(St_xl_partition_info)));

    /*检查分区的详细信息,1:是否mount 2:mount路径 3:partition的空间使用信息 4:label卷标信息*/
    int n = 0;
    do
    {
        if(!strncmp("sd",szDiskName,2))
        {
            sprintf((void *)(psTmpPartition->szPartName),"/dev/%s%d",szDiskName,(nPartitionIndex));
        }
        else
        {
            sprintf((void *)(psTmpPartition->szPartName),"/dev/%sp%d",szDiskName,(nPartitionIndex));
        }

        XL_DEBUG(EN_PRINT_INFO,"The disk count is  %d,nMaxPartitionIndex is %d \n",nPartitionCnt,nMaxPartitionIndex);
        XL_DEBUG(EN_PRINT_INFO,"To detect the partition device %s \n",psTmpPartition->szPartName);
        if (access((const char *)(psTmpPartition->szPartName),F_OK) == 0)
        {
            psTmpPartition->nUSBDiskId = nDiskID;
            psTmpPartition->nPatitionId = nPartitionIndex;
            XL_DEBUG(EN_PRINT_INFO,"To get the device %s info mation \n",psTmpPartition->szPartName);
            memset(psTmpPartition->szMountDir,0,256);
            eError = mnt_usb_get_mount_path(szDiskName,(char *)(psTmpPartition->szPartName),(char *)(psTmpPartition->szMountDir),&map_index);
            if (EN_OK != eError)
            {
                XL_DEBUG(EN_PRINT_INFO,"USB partition %s  has not been mounted\n",psTmpPartition->szPartName);
                psTmpPartition->bMounted = DISK_UNMOUNTED;     /* unmounted */
            }
            else
            {
                XL_DEBUG(EN_PRINT_INFO,"USB is mounted now\n");
                psTmpPartition->bMounted = DISK_MOUNTED;    /* mounted */
                psTmpPartition->nMapIndex = map_index;    /* mounted */
                
                XL_DEBUG(EN_PRINT_INFO,"Get the partition map index is %d \n",map_index);
                eError = mnt_usb_read_partition_space((char *)(psTmpPartition->szMountDir),(S64 *)&(psTmpPartition->u64Capacity),(S64 *)&(psTmpPartition->u64Used));
                if (EN_OK != eError)
                {
                    XL_DEBUG(EN_PRINT_ERROR, "Can't get the space info\n");
                }
                else
                {
                    XL_DEBUG(EN_PRINT_DEBUG, "Read the partition space info successfully\n");
                }
                eError = mnt_usb_get_label((char *)(psTmpPartition->szMountDir),(char *)psTmpPartition->szPartName,(char *)(psTmpPartition->szVolume));
                if (EN_OK != eError)
                {
                    XL_DEBUG(EN_PRINT_DEBUG,"Can't get label\n");
                }
                else
                {
                    XL_DEBUG(EN_PRINT_DEBUG, "Get the partition label info successfully\n");
                }
            }
            psTmpPartition++;
            n++;
            if (n >= nPartitionCnt)
            {
                XL_DEBUG(EN_PRINT_DEBUG,"++ All partition detected ++ ,break \n");
                break;
            }

        }
        else
        {
            XL_DEBUG(EN_PRINT_INFO,"Can't find the partition device %s \n",psTmpPartition->szPartName);
        }
        nPartitionIndex++;
    }while(nPartitionIndex <= nMaxPartitionIndex);

    psTmpPartition = pMalloc;

    eError = mnt_usb_insert_partition_list(psTmpPartition,nPartitionCnt);
    if (EN_OK != eError)
    {
    }
    free(pMalloc);

#if 0
    if (EN_OK == eError)
    {
        mnt_usb_send_partition_all();
    }
#endif
    return eError;
}


/***********************************************************
Function: mnt_usb_insert_partition_list
Description: Insert partition nodes into the list
Input: psPartition: the start address of the node
        nCnt: the number of the nodes
Output:
Return:
Others:
History:
************************************************************/
static EN_ERROR_NUM mnt_usb_insert_partition_list(St_xl_partition_info *psPartition,int nCnt)
{
    St_xl_USB_Part *psTmpNode;
    St_xl_USB_Part *psPreTmpNode = NULL;
    St_xl_partition_info *psTmpPartNode;
    int i;

    XL_CHECK_NULL(psPartition);
    XL_CHECK_PARA((0 >= nCnt));

    psTmpPartNode = psPartition;
    psTmpNode = s_psPartHead;
    
    XL_DEBUG(EN_PRINT_INFO,"Insert %d partition node into list \n",nCnt);

    if (NULL == s_psPartHead)
    {
        XL_DEBUG(EN_PRINT_INFO,"The partition list is null\n");
        for (i=0;i<nCnt;i++)
        {
            psTmpNode = malloc(sizeof(St_xl_USB_Part));
            if (NULL != psTmpNode)
            {
                memset(psTmpNode,0,sizeof(St_xl_USB_Part));
            }
            else
            {
                XL_DEBUG(EN_PRINT_ERROR,"There is no memory here\n");
                return EN_ERROR_NO_MEMORY;
            }

            memcpy(&(psTmpNode->m_sStorageInfo),psTmpPartNode,sizeof(St_xl_partition_info));
        
            if (psPreTmpNode)
            {
                psPreTmpNode->m_pNext = psTmpNode;
            }

            if (0 == i)
            {
                s_psPartHead = psTmpNode;
            }
            
            XL_DEBUG(EN_PRINT_INFO,"psTmpPartNode USBid %d ,part_id %d \n",(psTmpPartNode->nUSBDiskId),psTmpPartNode->nPatitionId);
            psTmpPartNode++;
            psPreTmpNode = psTmpNode;
        }
        mnt_ubus_usb_plug_notify();
    }
    else
    {
        for(i=0;i<nCnt;i++)
        {
            psPreTmpNode = s_psPartHead;
            while(1)
            {
                if ((psPreTmpNode->m_sStorageInfo.nUSBDiskId == psTmpPartNode->nUSBDiskId)&&(psPreTmpNode->m_sStorageInfo.nPatitionId == psTmpPartNode->nPatitionId))
                {
                    memset(&(psPreTmpNode->m_sStorageInfo),0,sizeof(St_xl_partition_info));
                    memcpy(&(psPreTmpNode->m_sStorageInfo),psTmpPartNode,sizeof(St_xl_partition_info));
                    break;
                }

                if (NULL == psPreTmpNode->m_pNext)
                {
                    XL_DEBUG(EN_PRINT_INFO,"The partition node list is not null , we will malloc new node \n");
                    psPreTmpNode->m_pNext = malloc(sizeof(St_xl_USB_Part));
                    psPreTmpNode = psPreTmpNode->m_pNext;
                    memcpy(&(psPreTmpNode->m_sStorageInfo),psTmpPartNode,sizeof(St_xl_partition_info));
                    psPreTmpNode->m_pNext = NULL;
                    break;
                }
                else
                {
                    psPreTmpNode = psPreTmpNode->m_pNext;
                }
                mnt_ubus_usb_plug_notify();
            }
            psTmpPartNode++;
        }
    }
    return EN_OK;
}


/***********************************************************
Function: mnt_usb_get_mount_path
Description: Get the mounted path of a partition
Input: szPartName: the partition name
Output: szMountDir: To store the mounted path of a partition
Return: EN_OK:successfully
        others:failed
Others:
History:
************************************************************/
static EN_ERROR_NUM mnt_usb_get_mount_path(char * szDevName,char *szPartName,char *szMountDir,int * nMapIndex)
{
    EN_ERROR_NUM eError = EN_ERROR_FAILED;
    char buf[1024] = {0};
    FILE * fd;
    int i = 0;
    char mount_file[256] = {0};

    sprintf(mount_file,"%s_%s",MOUNT_LOG_FILE,szDevName);

    fd = fopen(mount_file,"r");
    if (NULL == fd)
    {
        XL_DEBUG(EN_PRINT_ERROR,"unable  open file %s ,errno %s\n",mount_file,strerror(errno));
        return EN_ERROR_FAILED;
    }
    while (fgets(buf,sizeof(buf),fd)!=NULL)
    {
        eError = parsing_mount_log(buf,szPartName,szMountDir);
        if (EN_OK == eError)
        {
            XL_DEBUG(EN_PRINT_INFO,"#### get partition %s ,mount dir is %s ,mapped id is %d \n",szPartName,szMountDir,i);
            *nMapIndex = i;
            break;
        }
        else
        {
            memset(buf,0,sizeof(buf));
            eError = EN_ERROR_FAILED;
            XL_DEBUG(EN_PRINT_INFO,"#### unable get partition %s ,mount dir is %s ,mapped id is %d \n",szPartName,szMountDir,i);
            i++;
            continue;
        }
    }
    fclose(fd);
    return eError;
}

/***********************************************************
Function: parsing_mount_log
Description: To filter the mounted path of a partition from the a character string
Input: buf: a buffer in which the mounted infomation is recorded
       szPartName: the partition name
Output: szMountDir: the mounted path of a partition
Return: EN_OK: successfully
        others:failed
Others:
History:
************************************************************/
static EN_ERROR_NUM parsing_mount_log(char *buf,char *szPartName,char *szMountDir)
{
    char *info = NULL;
    char * tmpbuf = NULL;
    char devname[32] = {0};

    XL_CHECK_NULL(buf);
    XL_CHECK_NULL(szPartName);
    XL_CHECK_NULL(szMountDir);
    
    tmpbuf = buf;
    XL_DEBUG(EN_PRINT_INFO,"mount_log: %s \n",buf);
    info = strchr(tmpbuf,' ');
    XL_DEBUG(EN_PRINT_INFO,"mount_log: info  %s \n",info);
    strncpy(devname,buf,(info-tmpbuf));
    XL_DEBUG(EN_PRINT_DEBUG,"strlen devname %d ,devname %s\n",strlen(devname),devname);
    if (!strcmp(devname,szPartName))
    {
        tmpbuf = info+1;
        info = strchr(tmpbuf,' ');
        strncpy(szMountDir,tmpbuf,(info-tmpbuf));
        XL_DEBUG(EN_PRINT_DEBUG,"mountdir: %s \n",szMountDir);
        return EN_OK;
    }
    else
    {
        return EN_ERROR_FAILED;
    }
}

/***********************************************************
Function: mnt_usb_create_mount_log
Description: To create a log file and record the mounted infomation into the log
Input: szDiskName: the USB Disk Name
Output:
Return: EN_OK: successfully
        others:failed
Others:
History:
************************************************************/
static EN_ERROR_NUM mnt_usb_create_mount_log(char *szDiskName,int nDiskId)
{
    char command[256]={0};
    char mount_file[256] = {0};
    EN_ERROR_NUM eError;
    static int nPresize = 0;
    static int nNowsize = 0;


    if( s_UsbThread[nDiskId].detect_stat == 1)
    {
        nPresize = 0;
        nNowsize = 0;
        s_UsbThread[nDiskId].detect_stat = 0;
    }

    sprintf(mount_file,"%s_%s",MOUNT_LOG_FILE,szDiskName);
    //sprintf(command,"rm -rf %s ",mount_file);
    system(command);
    memset(command,0,sizeof(command));
//    XL_DEBUG(EN_PRINT_DEBUG,"**************************************************Cat Proc-Mounts \n");
//    system("cat /proc/mounts");
    sprintf(command,"cat %s | grep %s > %s",MOUNT_FILE,szDiskName,mount_file);
    system(command);

    nNowsize = get_file_size(mount_file);
    if (0 >= nNowsize)
    {
        XL_DEBUG(EN_PRINT_WARN,"Can not creat a mount log file filter the %s device \n",szDiskName);
        return EN_ERROR_FAILED;
    }
    else
    {
        if (nPresize != nNowsize)
        {
            eError = EN_OK;
        }
        else
        {
            eError = EN_ERROR_FAILED;
        }
        nPresize = nNowsize;
    }
    return eError;
}

static unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0)
    {
        return filesize;
    }
    else
    {
        filesize = statbuff.st_size;
    }
    return filesize;
}

/***********************************************************
Function: mnt_usb_read_partition_space
Description: Access to the partition space
Input:  szDir: the mounted path of a partition
Output: pu64Total: total size of the partition in Byte
        pu64Used : Used size of the partition in Byte
Return: EN_OK: successfully
        others:failed
Others:
History:
************************************************************/
EN_ERROR_NUM mnt_usb_read_partition_space(char *szDir,S64 *pu64Total,S64 *pu64Used)
{
    S64 freespace = 0;
    S64 totalspace = 0;
    struct statfs disk_statfs;

    XL_CHECK_NULL(szDir);
    XL_CHECK_NULL(pu64Total);
    XL_CHECK_NULL(pu64Used);
    if( statfs(szDir, &disk_statfs) >= 0 )
    {
        freespace = (long long)disk_statfs.f_bfree * (long long)disk_statfs.f_bsize;
        totalspace = (long long)disk_statfs.f_blocks * (long long)disk_statfs.f_bsize;
        *pu64Total = totalspace;
        *pu64Used = totalspace - freespace;
         XL_DEBUG(EN_PRINT_INFO,"totalspace    %4lld bytes, usedspace %4lld bytes\n", totalspace, (totalspace - freespace));
    }
    else
    {
        return EN_ERROR_FAILED;
    }
    return EN_OK;
}

/***********************************************************
Function: mnt_usb_get_partition_count
Description: Get the partition count from the USB partition list
Input: nUSBDiskID: The usb disk id
Output: none
Return: The partition count of the usbdisk
Others:
History:
************************************************************/
static U32 mnt_usb_get_partition_count(U32 nUSBDiskID)
{
    U32 nPartCnt = 0;
    St_xl_USB_Part *pTmpNode;


    pTmpNode = s_psPartHead;
    if (NULL == pTmpNode)
    {
        return 0;
    }

    while(pTmpNode)
    {
        if(!strncmp("/dev/sd",pTmpNode->m_sStorageInfo.szPartName,7))
        {
            if (((ALL_USB_DISK ==  nUSBDiskID)||(pTmpNode->m_sStorageInfo.nUSBDiskId == nUSBDiskID))&&(0 < pTmpNode->m_sStorageInfo.nUSBDiskId))
            {
                nPartCnt++;
            }
        }
        pTmpNode = pTmpNode->m_pNext;
    }
    return nPartCnt;
}
static U32 mnt_sdcard_get_partition_count(U32 nUSBDiskID)
{
    U32 nPartCnt = 0;
    St_xl_USB_Part *pTmpNode;


    pTmpNode = s_psPartHead;
    if (NULL == pTmpNode)
    {
        return 0;
    }

    while(pTmpNode)
    {
        if(!strncmp("/dev/mmc",pTmpNode->m_sStorageInfo.szPartName,8))
        {
            if (((ALL_USB_DISK ==  nUSBDiskID)||(pTmpNode->m_sStorageInfo.nUSBDiskId == nUSBDiskID))&&(0 < pTmpNode->m_sStorageInfo.nUSBDiskId))
            {
                nPartCnt++;
            }
        }
        pTmpNode = pTmpNode->m_pNext;
    }
    return nPartCnt;
}

/***********************************************************
Function: mnt_usb_get_label
Description: To get the label of a partition
Input: szDir: the mounted path of a partition
       szPartName: the partition name
Output:szVolume: the label name
Return: EN_OK: successfully
        others:failed
Others:
History:
************************************************************/
static EN_ERROR_NUM mnt_usb_get_label(char *szDir,const char *szPartName,char *szVolume)
{
    struct statfs disk_statfs;
    int nRet = 0;
    EN_ERROR_NUM eError = EN_OK;

    XL_CHECK_NULL(szDir);
    XL_CHECK_NULL(szPartName);
    XL_CHECK_NULL(szVolume);

    if( statfs(szDir, &disk_statfs) >= 0 )
    {
        XL_DEBUG(EN_PRINT_INFO,"Disk fs type is %d \n",disk_statfs.f_type);

        switch (disk_statfs.f_type)
        {
            case EXT3_SUPER_MAGIC:
                eError = ext3_get_label(szPartName,szVolume);
                XL_DEBUG(EN_PRINT_INFO,"Get label %s \n",szVolume);
                break;
            case MSDOS_SUPER_MAGIC:
                eError = fat32_get_label(szPartName,szVolume);
                XL_DEBUG(EN_PRINT_INFO,"Get label %s \n",szVolume);
                break;
            case NTFS_SB_MAGIC:
            case NTFS_MAGIC:
                nRet = ntfs_get_label(szPartName,szVolume);
                //sprintf(szVolume,"ntfs_%s",szPartName);
                if (0 == nRet)
                {
                    eError = EN_OK;
                }
                else
                {
                    eError = EN_ERROR_FAILED;
                }
                XL_DEBUG(EN_PRINT_INFO,"ntfs Get label %s \n",szVolume);
                break;
            default:
                XL_DEBUG(EN_PRINT_INFO,"disk_statfs.f_type 0x%x \n",disk_statfs.f_type);
                sprintf(szVolume,"unknow label");
                XL_ASSERT(0);
                break;
        }

    }
    return eError;
}

#define EXT3_VOLUME_OFFSET (0x478)
/***********************************************************
Function:ext3_get_label
Description:To get the label of a ext3/ext2 FS
Input:szPartName: the partition name
Output:szVolume: the label name
Return: EN_OK: successfully
        others:failed
Others:
History:
************************************************************/
static EN_ERROR_NUM ext3_get_label(const char *szPartName,char *szVolume)
{
    FILE *fp = NULL;
    char lable[17]={0};

    fp = fopen(szPartName,"rb");
    if (NULL ==fp)
    {
        printf("an error file discrambler \n");
        return EN_ERROR_FAILED;
    }
    fseek(fp,EXT3_VOLUME_OFFSET,SEEK_SET);
    fread(lable,16,1,fp);
    lable[16]='\0';
    strcpy(szVolume,lable);
    fclose(fp);

    return EN_OK;
}

#define FAT32_VOLUME_WIN_OFFSET (0x800000)
#define FAT32_VOLUME_UNIX_OFFSET (0x47)
#define INVALID_VOLUME "NO NAME"
/***********************************************************
Function:fat32_get_label
Description:To get the label of a FAT32 FS
Input:szPartName: the partition name
Output:szVolume: the label name
Return: EN_OK: successfully
        others:failed
Others:
History:
************************************************************/
#if 0
static EN_ERROR_NUM fat32_get_label(const char *szPartName,char *szVolume)
{
    FILE *fp = NULL;
    char lable[12]={0};

    fp = fopen(szPartName,"rb");
    if (NULL ==fp)
    {
        printf("an error file discrambler \n");
        return EN_ERROR_FAILED;
    }
    fseek(fp,FAT32_VOLUME_WIN_OFFSET,SEEK_SET);
    fread(lable,11,1,fp);
    lable[11]='\0';
    strcpy(szVolume,lable);
    fclose(fp);

    return EN_OK;
}
#else
#define FAT32_LABEL_LOG "/tmp/.vfat"
static EN_ERROR_NUM fat32_get_label(const char *szPartName,char *szVolume)
{
    strcpy(szVolume,"ntfslabel");
    return 0;
    FILE *fp = NULL;
    char cmd[256]={0};
    char last_buf[2048] = {0};
    char read_buf[2048] = {0};
    int flag = 0;
    if (NULL == szPartName || NULL == szVolume)
    {
        XL_DEBUG(EN_PRINT_ERROR,"Unable get the vfat label\n");
        return EN_ERROR_FAILED;
    }

    sprintf(cmd,"/thunder/bin/dosfslabel %s > %s ",szPartName,FAT32_LABEL_LOG);
    system(cmd);        
    fp = fopen(FAT32_LABEL_LOG,"r");
    if (NULL ==fp)
    {
        XL_DEBUG(EN_PRINT_ERROR,"unable to open file %s \n",FAT32_LABEL_LOG);
        return EN_ERROR_FAILED;
    }

    while (fgets(read_buf,2048,fp))
    {
        memset(last_buf,0,sizeof(last_buf));
        strcpy(last_buf,read_buf);
        memset(read_buf,0,sizeof(read_buf));
        flag = 1;
    }

    fclose(fp);
    if (1 == flag)
    {
        last_buf[strlen(last_buf)-1] = 0;
        XL_DEBUG(EN_PRINT_INFO,"Get the lase line string is  [%s] \n",last_buf);
        strcpy(szVolume,last_buf);
        return EN_OK;
    }
    else
    {
        XL_DEBUG(EN_PRINT_ERROR,"unable read the last line ,the file may be empty\n");
        return EN_ERROR_FAILED;
    }
}

#endif

#define NTFS_LABEL_LOG "/tmp/.ntfs"
#if 1
static EN_ERROR_NUM ntfs_get_label(const char *szPartName,char *szVolume)
{
    strcpy(szVolume,"ntfslabel");
    return 0;
}
#else
static EN_ERROR_NUM ntfs_get_label(const char *szPartName,char *szVolume)
{
    FILE *fp = NULL;
    char cmd[256]={0};
    char last_buf[2048] = {0};
    char read_buf[2048] = {0};
    int flag = 0;

    if (NULL == szPartName || NULL == szVolume)
    {
        XL_DEBUG(EN_PRINT_ERROR,"Unable get the vfat label\n");
        return EN_ERROR_FAILED;
    }

    sprintf(cmd,"/thunder/bin/ntfslabel -f  %s > %s ",szPartName,NTFS_LABEL_LOG);
    system(cmd);        
    fp = fopen(NTFS_LABEL_LOG,"r");
    if (NULL ==fp)
    {
        XL_DEBUG(EN_PRINT_ERROR,"unable to open file %s \n",NTFS_LABEL_LOG);
        return EN_ERROR_FAILED;
    }

    while (fgets(read_buf,2048,fp))
    {
        memset(last_buf,0,sizeof(last_buf));
        strcpy(last_buf,read_buf);
        memset(read_buf,0,sizeof(read_buf));
        flag = 1;
    }

    fclose(fp);
    
    if (1 == flag)
    {
        last_buf[strlen(last_buf)-1] = 0;
        XL_DEBUG(EN_PRINT_INFO,"Get the last line string is  [%s] \n",last_buf);
        strcpy(szVolume,last_buf);
        return EN_OK;
    }
    else
    {
        XL_DEBUG(EN_PRINT_ERROR,"unable read the last line ,the file may be empty\n");
        return EN_ERROR_FAILED;
    }   
}
#endif
static void * mnt_usb_probe_partition_thread(void)
{
    St_xl_USB_Info sUSBInfo;
    EN_ERROR_NUM eError;
    char strRcvData[256];
    int nLen;
    while(1)
    {
        memset(strRcvData,0,sizeof(strRcvData));
        eError = mnt_usb_msg_rcv(strRcvData, &nLen);
        if (EN_OK != eError)
        {
            XL_DEBUG(EN_PRINT_ERROR,"receive msg error\n");
            continue;
        }
        if (nLen != sizeof(sUSBInfo))
        {
            XL_DEBUG(EN_PRINT_ERROR," received a unknow msg in usb module \n");
            continue;
        }

        memcpy(&sUSBInfo,strRcvData,nLen);
        XL_DEBUG(EN_PRINT_INFO,"Act %d type %d DiskId %d \n",sUSBInfo.eAction,sUSBInfo.eType,sUSBInfo.nDiskId);
        s_UsbThread[sUSBInfo.nDiskId].pUsbInfo = malloc (sizeof(sUSBInfo));
        
        memcpy(s_UsbThread[sUSBInfo.nDiskId].pUsbInfo,&sUSBInfo,sizeof(sUSBInfo));

        if (EN_USB_ADD == sUSBInfo.eAction)
        {
            XL_DEBUG(EN_PRINT_INFO,"EN_USB_ADD cmd received \n");
            s_UsbThread[sUSBInfo.nDiskId].nRunState = 1;
            s_UsbThread[sUSBInfo.nDiskId].detect_stat = EN_MNT_USB_PLUG_IN;
            int nErr = pthread_create(&s_UsbThread[sUSBInfo.nDiskId].tid,NULL,(void *)mnt_usb_query_mount_log_thread,(void *)s_UsbThread[sUSBInfo.nDiskId].pUsbInfo);
            if(nErr != 0)
            {
                XL_DEBUG(EN_PRINT_ERROR,"pthread_create : msg_queue_route error :%s \n",strerror(errno));
                eError = EN_ERROR_UNKNOW;
            }
            else
            {
                eError = EN_OK;
            }
        }

        if (EN_USB_RM == sUSBInfo.eAction)
        {
            XL_DEBUG(EN_PRINT_INFO,"++++++++++++++++++++++++++++++++++++USB_RCV_RM_SIG,USBDISK ID %d++++++++++++++++++++++++++++++++++++++++++\n",sUSBInfo.nDiskId);
            if (s_UsbThread[sUSBInfo.nDiskId].tid)
            {
                pthread_cancel(s_UsbThread[sUSBInfo.nDiskId].tid);
            }
            if (s_UsbThread[sUSBInfo.nDiskId].pUsbInfo)
            {
                free(s_UsbThread[sUSBInfo.nDiskId].pUsbInfo);
            }
            s_UsbThread[sUSBInfo.nDiskId].nRunState = 0;
            s_UsbThread[sUSBInfo.nDiskId].detect_stat = EN_MNT_USB_PLUG_OUT;
            mnt_usb_delete_partitions_from_list(sUSBInfo.nDiskId);
            XL_DEBUG(EN_PRINT_INFO,"EN_USB_RM == sUSBInfo.eAction \n");
        }

        if (EN_OK != eError)
        {
            XL_DEBUG(EN_PRINT_ERROR,"Can't probe partition infomation  \n");
        }
        else
        {
            XL_DEBUG(EN_PRINT_INFO,"Probe partition infomation thread is working ... \n");
        }
    }                          
    return NULL;
}


static void * mnt_usb_query_mount_log_thread (void * pUSBInfo)
{
    
    St_xl_USB_Info sUSBInfo; 
    int time = 1000*1000;
    int flag = 0;
    int one_counts = 0;
    int five_counts = 0;
    
    if (NULL != pUSBInfo)
    {
        memcpy(&sUSBInfo,pUSBInfo,sizeof(St_xl_USB_Info));
    }
    else
    {
        XL_DEBUG(EN_PRINT_ERROR,"Create mnt_usb_query_mount_log_thread  input param error\n");
        return NULL;
    }
    
    XL_DEBUG(EN_PRINT_INFO,"query thread entry :EN_USB_ADD cmd received \n");
    
    s_UsbThread[sUSBInfo.nDiskId].detect_stat = EN_MNT_USB_PLUG_IN;

    while (s_UsbThread[sUSBInfo.nDiskId].nRunState)
    {
        usleep(time);
        
        EN_ERROR_NUM eError = mnt_usb_detect_partition(sUSBInfo.nDiskId);
        if ((EN_OK == eError))
        {
            if (flag == 0)
            {
                s_UsbThread[sUSBInfo.nDiskId].detect_stat = EN_MNT_USB_REST;
                flag = 1;
            }

            time = 3*1000*1000;
        }
        else
        {
            if (0 == flag)
            {
                s_UsbThread[sUSBInfo.nDiskId].detect_stat = EN_MNT_USB_QUERY;
            }
        }

        /*Caculate the timeout */
        if(time == 1000*1000)
        {
            one_counts++;
        }

        if(time == 3*1000*1000)
        {
            five_counts++;
        }
        
        if (((one_counts+five_counts*3)>=90)&&(flag == 0))
        {
            s_UsbThread[sUSBInfo.nDiskId].detect_stat = EN_MNT_USB_TIMEOUT;
        }
    }  
    
    if (NULL != pUSBInfo)
    {
        free(pUSBInfo);
    }
    return NULL;
}


bool mnt_is_usb_insert(void)
{
    int nPartitionCnt = 0;
    nPartitionCnt = mnt_usb_get_partition_count(ALL_USB_DISK);
    if (0 == nPartitionCnt)
    {
        return false;
    }
    else
    {
        return true;
    }
}
