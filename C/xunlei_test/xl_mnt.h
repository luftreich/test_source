#ifndef _XL_MNT_H_
#define _XL_MNT_H_
#ifdef __cplusplus
extern "C"{
#endif
#include "xl_typedef.h"

#define ALL_USB_DISK (0xffffffff)
#define DISK_MOUNTED    (0)
#define DISK_UNMOUNTED    (1)

typedef enum tagEN_ACTION
{
	EN_USB_RM = 0,          /*USB device plug out*/
	EN_USB_ADD,     /*USB device plug in*/
	EN_USB_CHG,         /*USB device change state*/
	EN_USB_END
}EN_ACTION;

typedef enum tagEN_MNT_USB_STATE
{
    EN_MNT_USB_PLUG_IN=1,
    EN_MNT_USB_QUERY,
    EN_MNT_USB_REST,
    EN_MNT_USB_PLUG_OUT,
    EN_MNT_USB_TIMEOUT,
    EN_MNT_USB_UNMOUNTED
}EN_MNT_USB_STATE;

typedef enum tagEN_DEVICE_TYPE
{
	EN_USB_DEVICE_STORAGE = 0,  /*USB storage device*/
	EN_USB_DEVICE_UNKNOW,       /*USB ,other type */
	EN_USB_DEVICE_END
}EN_DEVICE_TYPE;

typedef enum tagEN_LED_COLOR
{
    EN_LED_WHITE,   /*white led*/
    EN_LED_BLUE,    /*blue led*/
    EN_LED_ORANGE,   /*orange led*/
    EN_LED_RED,   /*red led*/
    EN_LED_DARK
}EN_LED_COLOR;

typedef struct tagSt_xl_storage_info
{
	U32 nDiskId;                  /*USB Disk ID ,Create by mnt,0~127*/
	char szDiskName[32];          /*Device name of the USB Disk : can't delete it ,because we need the name to parse the kernel log*/
}St_xl_storage_info;

typedef struct tagSt_xl_USB_Info
{
	EN_ACTION eAction;            /* Action:0 plugin ,1:plugout*/
	EN_DEVICE_TYPE  eType;        /*Type:0 Storage device,1:other device*/
 	U32 nDiskId;                  /*USB Disk ID*/
}St_xl_USB_Info;

typedef struct tagSt_xl_partition_info
{
    U32  nUSBDiskId;              /* USB Disk ID*/
    BOOL bMounted;                /*0:mounted,1:unmounted*/
    char  szMountDir[256];         /*Mounted path*/
    char  szVolume[64];            /*Volume name*/
    U64  u64Capacity;             /*Total Capacity in Byte units*/
    U64  u64Used;                 /*Used Space in Byte units*/
    U32  nPatitionId;             /* Partition ID*/
    char  szPartName[32];          /*0:Partition name such as sda1*/
    U32  nMapIndex;               /*The map index based on 'C'*/
}St_xl_partition_info;

typedef struct tagSt_xl_USBDisk_info
{
    U32 nUSBDiskId;
    U32 nPartitionCnt;
    St_xl_partition_info * psPartitionHead;
}St_xl_USBDisk_info;

#define XL_ERRNO_BASEID_MNT 	(1000)
typedef enum tagEN_MNT_ERRNO
{
    EN_MNT_ERRNO_SUCC = 0,
    EN_MNT_ERRNO_UNMOUNT_FAILED = XL_ERRNO_BASEID_MNT+1,                    /*usb  umount failed */
    EN_MNT_ERRNO_USB_TIMEOUT,                   /*usb active respond failed*/
    EN_MNT_ERRNO_USB_NOT_PLUGIN,                    /*no usb disk plugin*/
    EN_MMT_ERRNO_END,  
}EN_MNT_ERRNO;

#ifdef __cplusplus
}
#endif
#endif

