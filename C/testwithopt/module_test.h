#ifndef _MODULE_TEST_H_
#define _MODULE_TEST_H_
#ifdef __cplusplus
extern "C"{
#endif

int led_module_test(void);
int usb_module_test(void);
int sdcard_module_test(void);
int usb_init(void);
int net_detect(char *);
int device_get_ip(char * , char *);
void led_stop_run(void);
void led_start_run(void);
void usb_stop_run(void);
void usb_start_run(void);
void sdcard_start_run(void);
void sdcard_stop_run(void);
void ethwork_stop_run(void);
void ethwork_start_run(void);
int flash_module_test(void);
#ifdef __cplusplus
}
#endif
#endif
