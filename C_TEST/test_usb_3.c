#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <ctype.h> 
#include <sys/un.h> 
#include <sys/ioctl.h> 
#include <sys/socket.h> 
#include <linux/types.h> 
#include <linux/netlink.h> 
#include <errno.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <dirent.h>

typedef int bool;
#define TRUE (1)
#define FALSE (0)

#define UEVENT_BUFFER_SIZE 2048 
typedef enum tagEN_ERROR_NUM
{
    EN_OK = 0,
    EN_ERR_PARA,
    EN_NULL_P,
    EN_ERROR_NO_MEMORY,
    EN_ERROR_UNKNOW,
    EN_MAX
}EN_ERROR_NUM;

typedef enum tagEN_ACTION
{
	EN_USB_ADD = 0,
	EN_USB_RM,
	EN_USB_CHG,
	EN_USB_END
}EN_ACTION;

typedef enum tagEN_DEVICE_TYPE
{
	EN_USB_DEVICE_STORAGE = 0,
	EN_USB_DEVICE_UNKNOW,
	EN_USB_DEVICE_END
}EN_DEVICE_TYPE;
typedef struct tagSt_xl_storage_info{
	int nDiskId:16;
	int nPatitionCnt:16;
	char szDiskName[32];
}St_xl_storage_info;
typedef struct tagSt_xl_USB_Info {
	EN_ACTION action;
	EN_DEVICE_TYPE  type;
	int m_nVid;
	int m_nPid;	
	int m_nDid;
	St_xl_storage_info  m_Storage;
}St_xl_USB_Info;

typedef struct tagSt_xl_partition_info{
	int nDiskId:16;
	int nPatitionId:16;
	char szPartitionName[32];
	bool bMounted;
	char mount_dir[256];
}St_xl_partition_info;

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

static char g_DevID[128] = {0};

static void parse_event(const char *msg, struct luther_gliethttp *luther_gliethttp);
static int mnt_usb_get_dev (struct luther_gliethttp *luther_gliethttp,int *major,int * minor);
static EN_ACTION mnt_usb_get_action (struct luther_gliethttp *luther_gliethttp);
static int mnt_usb_get_partition_cnt(char * dev_name);
static EN_USB_DETECT_STATE mnt_usb_info_analysis(struct luther_gliethttp *luther_gliethttp,St_xl_USB_Info * sUSBInfo,EN_ACTION eAction_now);
static void mnt_usb_get_device_id(char * szProduct,int * pnVid,int * pnPid,int *pnDid);
static void mnt_usb_print_usbinfo(St_xl_USB_Info * sUSBInfo);
static EN_ERROR_NUM  mnt_usb_create_storage(struct luther_gliethttp *luther_gliethttp,St_xl_storage_info * pStorageInfo);
static void  mnt_usb_destroy_storage(St_xl_storage_info  *p_sStorageInfo);


int usb_detect(void ) ;
static int init_hotplug_sock() 
{ 
    const int buffersize = 1024; 
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
int main(void)
{
	pthread_t tid;	
	pthread_create(&tid, NULL,(void *) usb_detect, NULL);
	sleep(20);
	pthread_cancel(tid);
	pthread_join(tid,NULL);
	printf(" Cancel the pthread successfully\n");
	return 0;
}
int usb_detect(void ) 
{ 
    int hotplug_sock = init_hotplug_sock(); 
    struct luther_gliethttp luther_gliethttp;
    int nFlag =0;    
    int i,n;
    
    static EN_USB_DETECT_STATE eSTATE = EN_USB_GET_ACTION;
    static EN_ACTION eAction_now = EN_USB_RM,eAction_pre = EN_USB_RM;
    static int dev_major_pre=0,dev_major_now=0,dev_minor_pre=0,dev_minor_now=0;	
    St_xl_USB_Info sUSBInfo;
    St_xl_USB_Info * p_sUsbInfo;
    memset(&sUSBInfo,0,sizeof(sUSBInfo));
    while(1) 
    { 
        /* Netlink message buffer */ 
        char buf[UEVENT_BUFFER_SIZE * 2] = {0}; 
        n = recv(hotplug_sock, &buf, sizeof(buf), 0); 
#if 0
        for(i=0;i<n;i++)
        {
        if (buf[i] != 0)
        {
        printf("%c",buf[i]);
        }
        else
        {
        printf("#");
        }
        }
        printf("\n\r-----------------------------------------------------\n\r");
#else
        /*1.前一次ACTION 是 ADD,后一次变成了Remove，则进入发送UNPLUG的消息*/
        /*2.如果 devmajor_pre == devmajor_now ,但是devminor_pre != devminor_now,证明新一轮的检测开始了，进入EN_USB_GET_ACTION*/

        parse_event(buf,&luther_gliethttp);

        mnt_usb_get_dev(&luther_gliethttp,&dev_major_now,&dev_minor_now); 
        eAction_now = mnt_usb_get_action(&luther_gliethttp);
        if ( EN_USB_CHG == eAction_now)
        {
			eAction_now = EN_USB_ADD;
        }
        /*如果ACTION变了或者minor变了，说明新的一次插入、或拔出开始了*/
        if(eAction_now != eAction_pre )
        {
            eSTATE = EN_USB_GET_ACTION;
            dev_minor_pre = dev_minor_now;
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
                eSTATE = mnt_usb_info_analysis(&luther_gliethttp,&sUSBInfo,eAction_now);
            }
            break;
            case EN_USB_GET_DEV_NAME:
            {
                if (EN_USB_ADD == eAction_now)
                {
                    EN_ERROR_NUM eError = mnt_usb_create_storage(&luther_gliethttp,&(sUSBInfo.m_Storage));
                    if (EN_OK == eError)
                    {
                        //mnt_usb_insert_dev_node(&sUSBInfo);
                        printf("------------------------------------>发送USB盘插入的消息吧\n");
                        printf(" ---------------- --------Storage info--------- -------------- \n");
                        printf("Disk Id %d  ,partition id %d disk name %s \n",sUSBInfo.m_Storage.nDiskId,sUSBInfo.m_Storage.nPatitionCnt,sUSBInfo.m_Storage.szDiskName);
                        eSTATE = EN_USB_STANDBY;
                    }
                    
                }
                if (EN_USB_RM == eAction_now)
                {
                      //p_sUsbInfo = mnt_usb_find_dev_node(sUSBInfo.m_nVid,sUSBInfo.m_nPid,sUSBInfo.m_nDid);  
                      //if (NULL !=p_sUsbInfo)
                      {
                            memcpy(&(sUSBInfo.m_Storage),&(p_sUsbInfo->m_Storage),sizeof(St_xl_storage_info));
                            printf("------------------------------------>发送USB盘删除成功的消息吧\n");
                            //mnt_usb_delete_dev_node(p_sUsbInfo);
                            eSTATE = EN_USB_STANDBY ;
                      }
                }                
            }
            case EN_USB_STANDBY:
            {	
                /*抛上来的任何其他消息都不处理了*/				
            }
            break;
            default:
            break;
        }
#endif        
    } 
    return 0; 
}
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
        }
        else if (!strncmp(msg,"PRODUCT=",8))
        {
            msg += 8;
            luther_gliethttp->product = msg;
        }
        while(*msg++);
    }    
}

static int mnt_usb_get_dev (struct luther_gliethttp *luther_gliethttp,int *major,int * minor)
{
	/*subsystem get the key word "block" */
	if (strcmp ("block",luther_gliethttp->subsystem))
	{
		return -1;
	}
	if (((-1) == luther_gliethttp->major)||((-1)==luther_gliethttp->minor))
	{
		return -1;
	}
	else
	{
		*major = luther_gliethttp->major;
		*minor = luther_gliethttp->minor;
	}
	return 0;
}

static EN_ACTION mnt_usb_get_action (struct luther_gliethttp *luther_gliethttp)
{
	if(!strcmp("add",luther_gliethttp->action))
	{
		return EN_USB_ADD;
	}
	
	if(!strcmp("remove",luther_gliethttp->action))
	{
		return EN_USB_RM;
	}
	
	if(!strcmp("change",luther_gliethttp->action))
	{
		return EN_USB_CHG;
	}
}

static int mnt_usb_get_partition_cnt(char * dev_name)
{
    int nFd = -1;
    char acBuf[256] = {0};
    int nCount = 0;
    DIR *pDirFd = NULL;
    struct dirent *pDirInfo;
    char aFileName[64] = {0};
    char szUsbDirName[64] = {0};
    char szUsbPartDirName[64] = {0};

    sprintf(szUsbDirName,"/sys/block/%s",dev_name);
    if((pDirFd =opendir(szUsbDirName)) == NULL )
    {
        printf(" no usb-storage device attatched !\r\n");
        return -1;
    }
    closedir(pDirFd);
    
    nCount = 0;
    while (1)
    {
        sprintf(szUsbPartDirName,"%s/%s%d",szUsbDirName,dev_name,(nCount+1));
        if ((pDirFd = opendir(szUsbPartDirName)) != NULL)
        {
            nCount++;
            closedir(pDirFd);
            continue;
        }
        else
        {
            break;
        }
    }
    return nCount;
}

static EN_USB_DETECT_STATE mnt_usb_info_analysis(struct luther_gliethttp *luther_gliethttp,St_xl_USB_Info * sUSBInfo,EN_ACTION eAction_now)
{
    static char subsystem_now[16]={0};
    static char subsystem_pre[16]={0};
    static  char szProduct[16] = {0};
    EN_USB_DETECT_STATE eState = EN_USB_IDENTIFY_DEVICE;
    static bool bIsDiskDev = TRUE;
    bool bIsEnableSnd = FALSE;
    
    strcpy(subsystem_now,luther_gliethttp->subsystem);
    strcpy (szProduct,luther_gliethttp->product);
    mnt_usb_get_device_id(szProduct,&(sUSBInfo->m_nVid),&(sUSBInfo->m_nPid),&(sUSBInfo->m_nDid));   

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
    		bIsDiskDev = TRUE;
    		bIsEnableSnd = TRUE;
    	}
    	else if(!strcmp ("usb_device",subsystem_now)&&!strcmp ("usb",subsystem_pre))
    	{
    		eState = EN_USB_STANDBY;
    		bIsDiskDev = FALSE;
    		bIsEnableSnd = TRUE;
    	}
    	strcpy(subsystem_pre,subsystem_now);
    }

    if ( EN_USB_RM == eAction_now )
    {            	
    	/*判断是否是storage设备*/
    	if(!strcmp ("scsi_host",subsystem_pre)&&!strcmp ("usb",subsystem_now))
    	{
    		eState = EN_USB_GET_DEV_NAME;
    		bIsDiskDev = TRUE;
    		bIsEnableSnd = TRUE;
    	}
    	if(!strcmp ("usb_device",subsystem_pre)&&!strcmp ("usb",subsystem_now))
    	{
    		eState = EN_USB_STANDBY;
    		bIsDiskDev = FALSE;
    		bIsEnableSnd = TRUE;
    	}		
    	strcpy(subsystem_pre,subsystem_now);
    }
    
    if (TRUE == bIsEnableSnd)
    {
    	sUSBInfo->action = eAction_now;
    	if (FALSE == bIsDiskDev)
    	{
    		sUSBInfo->type = EN_USB_DEVICE_UNKNOW;
    		printf("发送消息一个未知的设备给其他模块\n");
    	}
    	else
    	{
            sUSBInfo->type = EN_USB_DEVICE_STORAGE;              
    	}
    	printf("								XL__DBG: 组装USB info  结束\n");
    	mnt_usb_print_usbinfo(sUSBInfo);
    }	
    return eState;
}
static int mnt_usb_create_disk_id(void)
{
    int i=0;
    for(i = 0;i<128;i++)
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
    return i;
}
static void  mnt_usb_destroy_disk_id(int nDevId)
{
    int n;
    n = nDevId;
    if (g_DevID[n] == TRUE)
    {
        g_DevID[n] = FALSE;
    }
}

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
    printf("        ------------------------>action %d type %d  VID 0x%x PID  0x%x   DID  0x%x  p_storage 0x%x \n",sUSBInfo->action,sUSBInfo->type,sUSBInfo->m_nVid,sUSBInfo->m_nPid,sUSBInfo->m_nDid,sUSBInfo->m_Storage);
    printf(" fun %s   end ... ... ... \n",__FUNCTION__);
}

static EN_ERROR_NUM  mnt_usb_create_storage(struct luther_gliethttp *luther_gliethttp,St_xl_storage_info * pStorageInfo)
{
    St_xl_storage_info * p_sStorageInfo = NULL;
    
    char subsystem_now [16] ={0};
      /*获取 设备节点名称*/
      strcpy(subsystem_now,luther_gliethttp->subsystem);
      if(!strcmp("block",subsystem_now))
      {
          p_sStorageInfo = pStorageInfo;
          if (NULL == p_sStorageInfo)
          {
                return EN_ERROR_UNKNOW;
          }
          memset(p_sStorageInfo,0,sizeof(St_xl_storage_info));
          p_sStorageInfo->nDiskId = mnt_usb_create_disk_id();
          strcpy(p_sStorageInfo->szDiskName,luther_gliethttp->devname);
          p_sStorageInfo->nPatitionCnt = mnt_usb_get_partition_cnt(p_sStorageInfo->szDiskName);
      }        
      else
      {
            return EN_ERROR_UNKNOW;
      }
      return EN_OK;
}
static void  mnt_usb_destroy_storage(St_xl_storage_info  *p_sStorageInfo)
{
    mnt_usb_destroy_disk_id(p_sStorageInfo->nDiskId);
    free (p_sStorageInfo);
    return;
}


