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

#define XL_DEBUG(level,fmt,arg...)  //printf(fmt,##arg)

#ifdef __cplusplus
}
#endif
#endif


