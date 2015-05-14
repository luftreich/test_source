
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#define MSG_TYPE(from_id)  ((~(from_id))&(~(1<<31)))
#define NET_MODULE_NAME         "NETWORK_MODULE"
#define NET_MODULE_NAME_TEST    "NETWORK_MODULE_TEST"
#define MNT_MODULE_NAME         "MNT_MODULE"
#define AGENT_MODULE_NAME       "AGENT_MODULE"
static char * s_ModuleName[] =
{
    NET_MODULE_NAME,
    MNT_MODULE_NAME,
    AGENT_MODULE_NAME,
};

int strtohex(char *str)
{
    int i,sum = 0;
    for(i = 0;str[i] != '\0';i++)
    {
        if(str[i]>='0' && str[i]<='9')
            sum += sum*16 + str[i]-'0';
        else if(str[i]<='f' && str[i]>='a')
            sum += sum*16 + str[i]-'a'+10;
        else if(str[i]<='F' && str[i]>='A')
        sum += sum*16 + str[i]-'A'+10;
    }

    return sum;
}

long long Convert_to_U64_High(int nHigh)
{
    int i = 0;
    long long long_value = 0;
    long_value = nHigh;
    for (i = 0; i<8;i++)
    {
        long_value *= 16;
    }
    return long_value;
}
void main(int argc , char *argv[])
{
	int i =0;
    char *strSN=NULL;
	
    strSN = argv[1];

    long long nProductNo = 0;
    long long nProductVersion = 0;
    int nFactoryNo = 0;
    int nYear = 0;
    int nMounth = 0;
    int nBatch = 0;
    int nSerialCnt = 0;
    
    long long lTest = 0x10000000;
    printf("lTest is %llx\n",lTest*16);

    nProductNo = (strSN[0]-'A')*26+(strSN[1]-'A')+1;
    printf("nProductNo is %d \n",nProductNo);    

    switch (strSN[2])
    {
        case 'T':
        nProductVersion = 0;
        break;
        case 'P':
        nProductVersion = 1;
        break;
        case 'M':
        nProductVersion = 2;
        break;
        default:
        printf("Error SN string \n");
        return ;
    }
    printf("nProductVersion is %d \n",nProductVersion);

    nFactoryNo = (strSN[3]-'A')+1;
    printf("nFactoryNo is %d \n",nFactoryNo);
    
    char strYear[3] = {0};
    memcpy(strYear,&strSN[4],2);
    nYear = strtohex(strYear);
    printf("strYear is %s \n",strYear);
    printf("nYear is %d \n",nYear);
    
    char strMonth[2] = {0};
    strMonth[0] = strSN[6];
    nMounth = strtohex(strMonth);
    printf("nMounth is %d \n",nMounth);
    
    char strBatch[2] = {0};
    strBatch[0] = strSN[7];
    nBatch = atoi(strBatch);
    printf("nBatch is %d \n",nBatch);
    
    char strSerialCnt[6] = {0};
    memcpy(strSerialCnt,&strSN[8],5);
    nSerialCnt = atoi(strSerialCnt);
    printf("nSerialCnt is %d \n",nSerialCnt);

    int LowIntNo = (nSerialCnt|(nBatch<<17)|(nMounth<<21)|(nYear<<25));
    int HighIntNo = (nYear>>7)|(nFactoryNo<<1)|(nProductVersion<<6)|(nProductNo<<8);
    printf("Caculate the Device id is 0x%x%x\n",HighIntNo,LowIntNo);
    long long Device_id = Convert_to_U64_High(HighIntNo)+LowIntNo;

    printf("Device id is %llx \n",Device_id);
    return 0;
}

