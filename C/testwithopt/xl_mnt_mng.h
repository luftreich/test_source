#ifndef _XL_MNT_MSG_H_
#define _XL_MNT_MSG_H_
#ifdef __cplusplus
extern "C"{
#endif

#include "xl_mnt_command.h"
#include "xl_mnt_usb.h"
#include "xl_mnt_led.h"

typedef struct tagSt_mnt_netlink_dev
{
    char dev_name[16];
    int state;
}St_mnt_netlink_dev;


EN_ERROR_NUM mnt_usb_msg_send(char *pData,int nLen);
int mnt_usb_msg_init(void);
EN_ERROR_NUM mnt_usb_check_disk_id(U32 nUSBDiskID);
EN_ERROR_NUM mnt_usb_get_disk_name(U32 nUSBDiskID,char * szDiskName);

EN_ERROR_NUM mnt_sdcard_read_partition(U32 nUSBDiskID,U32 *pnCnt,St_xl_partition_info **pDestData);
EN_ERROR_NUM mnt_usb_read_partition(U32 nUSBDiskID,U32 *pnCnt,St_xl_partition_info **pDestData);
EN_ERROR_NUM mnt_usb_read_partition_space(char * szDir,S64 *pu64Total,S64 *pu64Used);
int mnt_sys_init();
void mnt_led_reserve_status(void);
int mnt_led_set_status(EN_LED_COLOR eColor);
int mnt_ubus_main();
int mnt_ubus_usb_plug_notify();
int mnt_ubus_netlink_plug_notify(void *data);
int mnt_ubus_mine_func_notify(char * data);
/***********************************************************
Function:mnt_msg_send_usbinfo_all
Description:
Input:
Output:
Return:
Others:
History:
************************************************************/
EN_ERROR_NUM mnt_msg_send_usbinfo_all(St_xl_USB_Info * psUSBInfo);
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
EN_ERROR_NUM mnt_usb_msg_rcv(char *pData,int *pnLen);

/***********************************************************
Function:mnt_msg_send_usb_state
Description:Send usb state to all other modules
Input:
Output:
Return:
Others:
History:
************************************************************/
EN_ERROR_NUM mnt_msg_send_usb_state(int nDiskId,EN_MNT_USB_STATE eState);

#ifdef __cplusplus
}
#endif
#endif



