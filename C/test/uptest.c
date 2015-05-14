#include <stdio.h>
#include <string.h>
#include "xl_common.h"
int main(int argc, char *argv[])
{
    char data[1024] = {0};
    char * ptr=NULL; 
    char file[256] = {0};
    if (argc != 2) {
        fprintf(stderr, "wrong number of args\n");
    }

    if (strcmp("mac",argv[1]) == 0)
    {
        system("echo auto3 > /sys/class/aml_keys/aml_keys/version");
        system("echo mac >  /sys/class/aml_keys/aml_keys/key_name");
        sprintf(file,"/sys/class/aml_keys/aml_keys/key_read");
    }
    else if (strcmp("sn",argv[1]) == 0)
    {
        system("echo auto3 > /sys/class/aml_keys/aml_keys/version");
        system("echo usid >  /sys/class/aml_keys/aml_keys/key_name");
        sprintf(file,"/sys/class/aml_keys/aml_keys/key_read");
    }
    else 
    {
        printf("Error: %s mac,or %s sn \n",argv[0],argv[0]);
        return -1;
    }



    FILE *fp = fopen(file, "rb");
    //FILE *out = fopen("/tmp/mac","w"); 
    do 
    {
        ptr = fgets(data,sizeof(data),fp);
        if (NULL == ptr)
        {
           break;
        }

    } while(!feof(fp));

    printf("read : [%s]\n",data);
    
    char mac[1024]={0};
    char tmpdata[5] = {0};
    int i=0;
    ptr = data;
    if (strlen(ptr) <= 0)
    {
        printf("err,return\n");
        return 0 ;
    }
    
    while (strlen(ptr)>0)
    {
        int val =0;
        snprintf(tmpdata,sizeof(tmpdata),"0x%s",ptr);

        ptr += 2;
        sscanf(tmpdata,"%x",&val);
        sprintf(&mac[i],"%c",val );
        i++;
        memset(tmpdata,0,sizeof(tmpdata));
    }
    printf("data: %s \n",mac);
    return 0;
}


