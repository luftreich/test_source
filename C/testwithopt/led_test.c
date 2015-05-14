#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "xl_common.h"

#ifndef bool
typedef unsigned int bool;
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

typedef enum tagEN_DEVICE_TYPE{
        EN_LED_ALL,
        EN_LED_RED,
        EN_LED_GREEN,
        EN_LED_BLUE,
        EN_LED_DARK
}EN_LED_COLOR;

#define RED_LED (2)
#define GREEN_LED (3)
#define BLUE_LED (4)

void gpio_export(unsigned int gpio)
{
    char cmd_buf[255] = {0};
    /*echo 59 > /sys/class/gpio/export*/
    memset(cmd_buf, 0, sizeof(cmd_buf));
    sprintf(cmd_buf, "echo %d > /sys/class/gpio/export ", gpio);
    system(cmd_buf);
    return;
}

void gpio_direction(unsigned int gpio, bool direction)
{
    char cmd_buf[255] = {0};
    /*echo out > /sys/class/gpio/gpio59/direction*/ 
    memset(cmd_buf, 0, sizeof(cmd_buf));
    if(direction)
        sprintf(cmd_buf, "echo in > /sys/class/gpio/gpio%d/direction", gpio);
    else
        sprintf(cmd_buf, "echo out > /sys/class/gpio/gpio%d/direction ", gpio);
    system(cmd_buf);
    return;
}

void gpio_value(unsigned int gpio, bool high)
{
    char cmd_buf[255] = {0};
    /*echo 0/1 > /sys/class/gpio/gpio59/value*/ 
    memset(cmd_buf, 0, sizeof(cmd_buf));
    sprintf(cmd_buf, "echo %d > /sys/class/gpio/gpio%d/value ", high, gpio);
    system(cmd_buf);
    return;
}

void gpio_set(unsigned int gpio, bool direction, bool high)
{
    gpio_export(gpio);
    gpio_direction(gpio, direction);
    gpio_value(gpio, high);
    return;
}

/***********************************************************
Function: mnt_led_set_status
Description:
Input: p_sMSGData:received from MSG module
Output: NONE
Return: EN_OK:  success
        others: fail
Others:
History:
************************************************************/
int mnt_led_set_status(EN_LED_COLOR eColor)
{
    int ret = 0;
    switch(eColor)
    {
            case EN_LED_RED:
            {
                gpio_value(RED_LED,true);
                gpio_value(GREEN_LED,false);
                gpio_value(BLUE_LED,false);
            }
            break;
            case EN_LED_BLUE:
            {
                gpio_value(RED_LED,false);
                gpio_value(GREEN_LED,false);
                gpio_value(BLUE_LED,true);
            }
            break;
            case EN_LED_ALL:
            {
                gpio_value(RED_LED,true);
                gpio_value(GREEN_LED,true);
                gpio_value(BLUE_LED,true);
            }
            break;
            case EN_LED_DARK:
            {
                gpio_value(RED_LED,false);
                gpio_value(GREEN_LED,false);
                gpio_value(BLUE_LED,false);
            }
            break;
            case EN_LED_GREEN:
            {
                gpio_value(RED_LED,false);
                gpio_value(GREEN_LED,true);
                gpio_value(BLUE_LED,false);
            }
            break;
            default:
            ret = 1;
            break;
    }

    return ret;
}
static void print_usage(FILE *stream)
{
    fprintf(stream,"Exit : please input 'q','quit',or 'exit' \n");
    fprintf(stream,"Input led color to bright the led\n");
    fprintf(stream,"[color: \"red\" \"green\" \"blue\" ]\n");
    fprintf(stream,"Input the command to control led \n");
    fprintf(stream,"[command: \"all\" \"dark\"]\n");
}

int led_module_test(void)
{
     char input[256] = {0};
     print_usage(stdout);
     gpio_set(RED_LED,false,false);
     gpio_set(BLUE_LED,false,false);
     gpio_set(GREEN_LED,false,false);
     while (1)
     {
         printf("led >>> ");

         memset(input,0,sizeof(input));
         fgets(input,256,stdin);
         if (!strncmp("q",input,1)||!strncmp("quit",input,4)||!strncmp("exit",input,4))
         {
             fprintf(stdout,"quit led test \n");
             break;
         }

         if (!strncmp("red",input,3))
         {
               mnt_led_set_status(EN_LED_RED);
         }
         else if (!strncmp("green",input,5))
         {
               mnt_led_set_status(EN_LED_GREEN);
         }
         else if (!strncmp("blue",input,4))
         {
               mnt_led_set_status(EN_LED_BLUE);
         }
         else if (!strncmp("dark",input,4))
         {
               mnt_led_set_status(EN_LED_DARK);
         }
         else if (!strncmp("all",input,3))
         {
               mnt_led_set_status(EN_LED_ALL);
         }
         else 
         {
               fprintf(stdout,"error command for led\n");
               print_usage(stdout);
         }
     } 
     return 0;
}

pthread_t led_tid = 0;
static int led_thread_disable = 0;
void * led_test_thread(void)
{
        while(1)
        {
                if(1 == led_thread_disable)
                {
                        break;
                }
                mnt_led_set_status(EN_LED_RED);
                usleep(1000*500*2);
                mnt_led_set_status(EN_LED_GREEN);
                usleep(1000*500*2);
                mnt_led_set_status(EN_LED_BLUE);
                usleep(1000*500*2);
                mnt_led_set_status(EN_LED_ALL);
                usleep(1000*500*2);
                mnt_led_set_status(EN_LED_DARK);
                usleep(1000*500*2);
        }
        return NULL;
}
void led_start_run(void)
{
    if(0 == led_tid)
    {
       gpio_set(RED_LED,false,false);
       gpio_set(BLUE_LED,false,false);
       gpio_set(GREEN_LED,false,false);

       pthread_create(&led_tid,NULL,(void *)led_test_thread,NULL);
    }
        
    return;
}

void led_stop_run(void)
{
        led_thread_disable=1;
        return;
}
