#ifndef _XL_MNT_USB_H_
#define _XL_MNT_USB_H_
#ifdef __cplusplus
extern "C"{
#endif
#include "xl_mnt.h"

void * mnt_usb_detect_thread(void);

EN_ERROR_NUM mnt_usb_init(void);

EN_ERROR_NUM mnt_usb_term(void);

EN_ERROR_NUM mnt_reset_init(void);

EN_ERROR_NUM mnt_reset_term(void);

void mnt_usb_init_device_sem(void);

int mnt_usb_device_probe(void);

int mnt_sysload_init();

int mnt_sysload_term();

int xl_mnt_encode_sysload(char *pData, int *pLen );
#ifdef __cplusplus
}
#endif
#endif


