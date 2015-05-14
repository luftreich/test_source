/******************************************************************************
Copyright (C), 2003-2013, Xunlei Network Tech. Ltd.
FileName: test_flash_ctrl.c
Author:
version:
Description: -
Date 2014/01/18
Description:
History:
        1. Date:  2014-01-18
           Author: hanshaoyang
           Modification:
******************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <mtd/mtd-user.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
int region_erase(int Fd, int start, int count, int unlock, int regcount)
{
    int i, j;
    region_info_t * reginfo;

    reginfo = calloc(regcount, sizeof(region_info_t));

    for(i = 0; i < regcount; i++)
    {
        reginfo[i].regionindex = i;
        if(ioctl(Fd,MEMGETREGIONINFO,&(reginfo[i])) != 0)
        return 8;
        else
        printf("Region %d is at %d of %d sector and with sector "
        "size %x\n", i, reginfo[i].offset, reginfo[i].numblocks,
        reginfo[i].erasesize);
    }

    // We have all the information about the chip we need.

    for(i = 0; i < regcount; i++)
    { //Loop through the regions
        region_info_t * r = &(reginfo[i]);

        if((start >= reginfo[i].offset) &&
        (start < (r->offset + r->numblocks*r->erasesize)))
        break;
    }

    if(i >= regcount)
    {
        printf("Starting offset %x not within chip.\n", start);
        return 8;
    }

    //We are now positioned within region i of the chip, so start erasing
    //count sectors from there.

    for(j = 0; (j < count)&&(i < regcount); j++)
    {
        erase_info_t erase;
        region_info_t * r = &(reginfo[i]);

        erase.start = start;
        erase.length = r->erasesize;

        if(unlock != 0)
        { //Unlock the sector first.
            if(ioctl(Fd, MEMUNLOCK, &erase) != 0)
            {
            perror("\nMTD Unlock failure");
            close(Fd);
            return 8;
            }
        }

        printf("\rPerforming Flash Erase of length %u at offset 0x%x",
        erase.length, erase.start);
        fflush(stdout);
        if(ioctl(Fd, MEMERASE, &erase) != 0)
        {
            perror("\nMTD Erase failure");
            close(Fd);
            return 8;
        }

        start += erase.length;
        if(start >= (r->offset + r->numblocks*r->erasesize))
        { //We finished region i so move to region i+1
            printf("\nMoving to region %d\n", i+1);
            i++;
        }
    }

    printf(" done\n");

    return 0;
}

int non_region_erase(int Fd, int start, int count, int unlock)
{
    mtd_info_t meminfo;

    if (ioctl(Fd,MEMGETINFO,&meminfo) == 0)
    {
        erase_info_t erase;

        erase.start = start;

        erase.length = meminfo.erasesize;

        int TmpCount = meminfo.size/meminfo.erasesize;
        printf("mtd size 0x%x  erasesize 0x%x blk count 0x%x\n",meminfo.size,meminfo.erasesize,TmpCount);
        if (start == 0 && count == 0xFFFFFFFF)
        {
            count = TmpCount;
            printf("Erase all blocks \n");
        }
        
        
        for (; count > 0; count--)
        {
            printf("\rPerforming Flash Erase of length %u at offset 0x%x",
            erase.length, erase.start);
            fflush(stdout);

            if(unlock != 0)
            {
                //Unlock the sector first.
                printf("\rPerforming Flash unlock at offset 0x%x",erase.start);
                if(ioctl(Fd, MEMUNLOCK, &erase) != 0)
                {
                perror("\nMTD Unlock failure");
                close(Fd);
                return 8;
                }
            }

            if (ioctl(Fd,MEMERASE,&erase) != 0)
            {
                perror("\nMTD Erase failure");
                close(Fd);
                return 8;
            }

            erase.start += meminfo.erasesize;
        }
        printf(" done\n");
    }
    return 0;
}

int erase_block(const char *device_name,long start,int count)
{
    int regcount;
    int Fd;
    int unlock = 0;
    int res = 0;

    printf("erase_block : device:%s   start 0x%0x, count:%d \n",device_name,(int)start,count);

    // Open and size the device
    if ((Fd = open(device_name,O_RDWR)) < 0)
    {
    fprintf(stderr,"File open error\n");
    return 8;
    }

    printf("Erase Total %d Units\n", count);

    if (ioctl(Fd,MEMGETREGIONCOUNT,&regcount) == 0)
    {
        printf(" regcount is %d \n", regcount);
        if(regcount == 0)
        {
        res = non_region_erase(Fd, start, count, unlock);
        }
        else
        {
        res = region_erase(Fd, start, count, unlock, regcount);
        }
    }
    close(Fd);

    return res;
}

#define MAXLEN (1024)
#define CFG_MTD "/dev/mtd8"
#define ROM_SIZE (0x10000)

FILE *flashfd = NULL;

int xl_rom_init(void)
{
    long size =0;

    flashfd = fopen(CFG_MTD, "wb");
    if (NULL == flashfd)
    {
         perror(" can't open device \n");
         return -1;
    }
    fseek(flashfd,SEEK_END,0);

    size = ftell(flashfd);

    fseek(flashfd,SEEK_SET,0);

    printf("size of %s is  0x%x \n",CFG_MTD,(int)size);

    return 0;
}

/***********************************************************
Function: xl_write_rom
Description:  write data to
Input:
Output:
Return:
Others:
History:
************************************************************/
int xl_write_rom(int nOffset,const char *SrcAddr,int len)
{
    char * buf;
    buf = (char *)SrcAddr;
    fseek(flashfd,SEEK_SET,nOffset);
    fwrite( buf, sizeof(unsigned char),len,flashfd);
    return 0;
}

/***********************************************************
Function: xl_read_rom
Description:  read data to
Input:
Output:
Return:
Others:
History:
************************************************************/
int xl_read_rom(int nOffset,int len,const char *DesAddr)
{
    char buf[MAXLEN];
    int nRet = 0;
    int size =0;
    int rc = 0;

    memset ((void *)&buf,0,sizeof(buf));
    fseek(flashfd,SEEK_SET,nOffset);
    while(1)
    {
        rc = fread(buf,sizeof(unsigned char), MAXLEN,flashfd);
        if (0 >= rc)
        {
            nRet = 1;
            perror("read data from file error\n");
            break;
        }
        else
        {
            size += rc;
            if (size >= len)
            {
                nRet = 0;
                memcpy((char *)DesAddr,&buf,(rc-(size-len)));
                break;
            }
            else
            {
                memcpy((char *)DesAddr,&buf,rc);
                memset ((char *)&buf,0,sizeof(buf));
            }
        }
    }

    return nRet;
}


int main_test(int argc, char * argv[])
{
    int nRet =0 ;
    nRet = xl_rom_init();
    if (nRet != 0)
    {
        printf("error\n");
    }
    return 0;
}

int _main(int argc, char * argv[])
{
    int start;
    int count;

    if (1 >= argc)
    {
    fprintf(stderr,"You must specify a device\n");
    printf("Help : %s <device name >  <start>  <count> <unlock:1,lock :0> \n", argv[0]);
    return 16;
    }

    if (argc > 2)
    start = strtol(argv[2], NULL, 0);
    else
    start = 0;

    if (argc > 3)
    count = strtol(argv[3], NULL, 0);
    else
    count = 1;

    printf("L: %d \n",__LINE__);
    erase_block(argv[1],start,count);
    return 0;
}


