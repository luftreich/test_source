#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include "xl_common.h"
int IsLittleEnd(void)
{  
    union
    {  
        unsigned int  a;  
        unsigned char b;  
    }c;  
    c.a = 1;  
    if (1 == c.b)
        printf("Little\n");
    else
        printf("Big\n");  
    return 0;
}
char * board_info[][2] = {
    {"V1.1","RS1403"}, 
    {"V1.1","RS1403"}, 
    {"V1.1","RS1403"}, 
    {"V1.1","RS1403"}, 
    {"V1.2","RS1407"}
};

int main()
{
    IsLittleEnd();
    return 0;
}
int main_none ()
{
    int i,j;

    for(i=0;i<2;i++)
    {
        for(j=0;j<2;j++)
        {
            printf("[%d][%d]: %s ",i,j,board_info[i][j]);
        }
        printf("\n");
    }
    char * tmpdata[1][2];
    int cnt = 0;
    cnt = sizeof(board_info) /sizeof(tmpdata);
    printf("there are %d item \n",cnt);
    return 0;
}
