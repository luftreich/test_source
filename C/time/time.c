#include <stdio.h>    // for printf()  
#include <time.h>
#include <unistd.h>
#include <errno.h>

int main(void)
{
    time_t now;
    struct tm * tm_now;
    char datetime[200] = {0};

    time(&now);
    tm_now = localtime(&now);
    strftime(datetime, 200, "%Y-%m-%d %H:%M:%S", tm_now);

    printf("now datetime %s",datetime);
    

    return 0;
}
