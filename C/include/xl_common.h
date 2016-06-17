#ifndef _XL_COMMON_H_
#define _XL_COMMON_H_
#ifdef __cplusplus
extern "C"{
#endif

#include    <stdio.h>
#include    <stdlib.h>
#include    <stdbool.h>
#include    <sys/types.h>
#include    <sys/ipc.h>
#include    <sys/msg.h>
#include    <sys/vfs.h>
#include    <sys/mount.h>
#include    <sys/socket.h>
#include    <sys/ioctl.h>
#include    <sys/un.h>
#include    <sys/stat.h>
#include    <sys/socket.h>
#include    <sys/time.h>
#include    <linux/netlink.h>
#include    <arpa/inet.h>
#include    <netinet/in.h>
#include    <dirent.h>
#include    <sys/inotify.h>
#include    <limits.h>
#include    <fcntl.h>
#include    <unistd.h>
#include    <string.h>
#include    <errno.h>
#include    <signal.h>
#include    <semaphore.h>
#include    <pthread.h>
#include    <math.h>
#include    <sys/mman.h>
#include    "xl_typedef.h"
#define XL_DEBUG(level,fmt,arg...)   //printf(fmt,##arg)
#ifdef XL_ENCODE
static int xl_encode_u32(char  *pcBuf, int ulData, int ulLen)
{
    char *pcOut = pcBuf;

  	pcOut[0] = (ulData>>24)&0xff;
	  pcOut[1] = (ulData>>16)&0xff;
	  pcOut[2] = (ulData>>8)&0xff;
	  pcOut[3] = ulData&0xff;

	  return 0;
}

static int xl_decode_u32(char  *pcBuf, int *pulData)
{
    char *pcIn = pcBuf;
	  int  ulData = 0;

	  ulData = pcIn[0];
	  ulData = (ulData<<8) + pcIn[1];
	  ulData = (ulData<<8) + pcIn[2];
	  ulData = (ulData<<8) + pcIn[3];
	  *pulData = ulData;

  	return 0;
}


#ifdef __cplusplus
}
#endif
#endif
#endif
