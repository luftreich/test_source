#define _LARGEFILE64_SOURCE
#include "xl_common.h"
#include <sys/stat.h>
#include <unistd.h>
int  main(int argc,char * argv[])
{
    struct stat64 buf;
    stat64(argv[1], &buf);
    printf("[file %s ]  size = %lld \n",argv[1], (long long )buf.st_size);
    printf("[file %s ]  size = %lld \n",argv[1], (long long )buf.st_size);
    return 0;
}

