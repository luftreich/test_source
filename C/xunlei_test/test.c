#include <unistd.h>
#include <getopt.h>
#include "xl_common.h"
#include "module_test.h"
#include <string.h>
#include "module_test.h"


typedef int int32_t;

typedef int TFun(void);

typedef struct tagmodule_test{
    char * module_name;
    TFun * test_fun;
}module_test;


char * program_name = NULL;

int all_module_test(void);
extern int led_module_test(void);
module_test g_module[] = {
    {"led",led_module_test},
    {"usb",usb_module_test},
    {"sdcard",sdcard_module_test},
    {"flash",flash_module_test},
    {"allstart",all_module_test},
};
pthread_t all_tid = 0;
void print_usage(FILE *stream)
{
    fprintf(stream, "Exit : please input 'q','quit' or 'exit' \n");
    int i = sizeof(g_module) / sizeof(module_test);
    fprintf(stream,"Input module name to test as follows : \n");
    for (i =0;i<(sizeof(g_module) / sizeof(module_test));i++)
    {
         fprintf(stream,"module : %s \n",g_module[i].module_name);
    }
    return;
}

const struct option long_options[] = {
{"led",required_argument,NULL,'l'},
{"usb",required_argument,NULL,'u'},
{"sdcard",required_argument,NULL,'s'}
};

void * memrchr(const void *s,int c,size_t n);
void sigroutine(int duno)
{
    switch(duno)
    {
        case 11:
        printf("SIGSEGV get a signal \n");
        break;
        default:
        break;
    }
}
static int all_module_flag = 0;
void * run_all_module(void)
{
    time_t timep_s,timep_n;
    time (&timep_s);
    char starttime[256] = {0};
    sprintf(starttime,"%s",asctime(gmtime(&timep_s)));
    //signal(SIGSEGV,sigroutine);
    while(1)
    {
        if(all_module_flag ==1)
        {break;}


        led_start_run();
        //usb_start_run();
        //sdcard_start_run();
        ethwork_start_run();
        fflush(stdout);

        time(&timep_n);
        sleep(10);
        printf("start time : %s, now time %s \n",starttime,asctime((gmtime(&timep_n))));
    }

    {
            printf("quit from all\n");
            led_stop_run();
            usb_stop_run();
            sdcard_stop_run();
            ethwork_stop_run();
    }
    return NULL;
}

void stop_all_module(void)
{
    all_module_flag=1;
}
int all_module_test(void)
{
    char input[256] = {0};
    if (0 == all_tid)
    pthread_create(&all_tid,NULL,(void*)run_all_module,NULL);
    while(1)
    {
        memset(input,0,sizeof(input));
        fgets(input,2048,stdin);
        if (!strncmp("q",input,1)||!strncmp("Q",input,1)||!strncmp("quit",input,4))
        {
             stop_all_module();
             break;
        }
    }
    return 0 ;
}
int32_t main(int32_t argc, char **argv) {

    char input [2048] = {0};
    int cnt = 0;
    int ret = 0;
    char ip[16] = {0};
    usb_init();

    ret = device_get_ip(ip,"eth0");
    if (ret != 0)
    {
        if (1 == net_detect("eth0"))
        {
            system("/etc/init.d/S40network restart");
            device_get_ip(ip,"eth0");
        }
        else
        {
             printf("Please plug in the network cable\n");
             return -1;
        }
    }

    printf("The ip is %s \n",ip);
    if (argc >= 2)
    {
        if (0 == strcmp("allstart",argv[1]))
        {
          all_module_test();
          while(1)
          {
              sleep(60);
          }
        }
    }
    printf(">>> \t\n");
    while(1)
    {
        memset(input,0,sizeof(input));
        print_usage(stdout);
        printf(">>> \t");
        fgets(input,2048,stdin);
        if (!strncmp("q",input,1)||!strncmp("quit",input,4)||!strncmp("exit",input,4))
        {
            fprintf(stdout,"quit the XunLei Miner test sw\n");
            exit(1);
        }

        for(cnt = 0;cnt < (sizeof(g_module)/sizeof(module_test));cnt++)
        {
            if (!strncmp(g_module[cnt].module_name,input,strlen(g_module[cnt].module_name)-1))
            {
                g_module[cnt].test_fun();
                break;
            }
        }

        continue;

    }
    return 0;
}
