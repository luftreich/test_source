/******************************************************************************
Copyright (C), 2003-2013, Xunlei Network Tech. Ltd.
FileName: xl_flash.c
Author:
version:
Description: -
Date 2014/02/14
Description:
History:
        1. Date:  2014-02-14
           Author: hanshaoyang
           Modification:
******************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <mtd/mtd-user.h>
#include <errno.h>
#include <getopt.h>

#include "xl_common.h"
#define MAXLEN (1024)
#define CFG_PRIV_FILE "/tmp/thunder/private_data"
#define CFG_MTD "/dev/mtdblock3"
#define ERASE_BLOCK_SIZE (0x10000)
#define MIN(a,b) ((a)>(b)?:(a),(b))

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

    printf("erase_block : device:%s   start 0x%0x, count:%d \n",device_name,(unsigned int)start,count);

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

#if 0
int main_(int argc, char * argv[])
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
}
#endif

int xl_private_area_init(void)
{
    long size =0;
    FILE *flashfd = NULL;

    flashfd = fopen(CFG_MTD, "wb");
    if (NULL == flashfd)
    {
         perror(" can't open device \n");
         return -1;
    }
    fseek(flashfd,0,SEEK_END);

    size = ftell(flashfd);

    fseek(flashfd,0,SEEK_SET);

    XL_DEBUG(EN_PRINT_DEBUG,"size of %s is  0x%x \n",CFG_MTD,size);
    
    fclose(flashfd);
    return 0;
}

/***********************************************************
Function: xl_write_private_area
Description:  write data to
Input:
Output:
Return:
Others:
History:
************************************************************/
int xl_write_private_area(int StartAddr,int nLen,const char *SrcAddr)
{
    FILE * flashfd = NULL;
    int nWrittenSize = 0;
    
    if (NULL == SrcAddr)
    {
        return -1;
    }
        
    flashfd = fopen(CFG_PRIV_FILE, "wb");
    if (NULL == flashfd)
    {
            XL_DEBUG(EN_PRINT_ERROR," can't open file %s \n",CFG_PRIV_FILE);
            return -1;
    }
    
    fseek(flashfd,StartAddr,SEEK_SET);
    nWrittenSize = fwrite(SrcAddr, sizeof(unsigned char),nLen,flashfd);
    XL_DEBUG(EN_PRINT_INFO,"Write to file %s ,%d bytes \n",CFG_PRIV_FILE,nWrittenSize);
    fclose(flashfd);
    return 0;
}

/***********************************************************
Function: xl_read_private_area
Description:  read data to
Input: nOffset : start read address based
       len: the length of the data be read
Output:
Return:
Others:
History:
************************************************************/
int xl_read_private_area(int StartAddr,int len,char *DesAddr)
{
    char buf[MAXLEN];
    char * TmpDest;
    int nRet = 0;
    int size =0;
    int rc = 0;
    int nLen = 0;
    int nReadLen = 0;
    int nOffset = 0;
    FILE *flashfd = NULL;

    nOffset = StartAddr;
    flashfd = fopen(CFG_PRIV_FILE, "rb");
    if (NULL == flashfd)
    {
         XL_DEBUG(EN_PRINT_ERROR," can't open file %s  \n",CFG_PRIV_FILE);
         return -1;
    }


    TmpDest = (char *)DesAddr;
    memset(buf,0,MAXLEN);
    fseek(flashfd,nOffset,SEEK_SET);
    nLen = len;
    while(1)
    {
        if ( nLen > MAXLEN)
        {
            nReadLen = MAXLEN;
        }
        else
        {
            nReadLen = nLen;
        }

        rc = fread(buf,sizeof(unsigned char), nReadLen,flashfd);
        if (0 >= rc)
        {
            nRet = 1;
            perror("read data from file error\n");
            break;
        }
        else
        {
            nLen -= nReadLen;
            size += rc;
            if (size >= nReadLen)
            {
                nRet = 0;
                memcpy((char *)DesAddr,buf,(rc-(size-nReadLen)));
                break;
            }
            else
            {
                memcpy(DesAddr,buf,rc);
                memset (buf,0,sizeof(buf));
            }
        }
    }
    
    if (NULL != flashfd)
    {
        fclose(flashfd);
    }
    return nRet;
}

/***********************************************************
Function: xl_write_private_area_flash
Description:  write data to
Input:
Output:
Return:
Others:
History:
************************************************************/
int xl_write_private_area_flash(int StartAddr,int nLen,const char *SrcAddr)
{
    char * buf;
    FILE * flashfd = NULL;
    char tmpbuf[ERASE_BLOCK_SIZE] = {0};
    int nUnitCnt = 0;
    int nWrittenSize = 0;
    int nBase_start = 0;
    int nTmpLen = 0;
    int nFreeSize = 0;
    int i = 0; 
    int nOffset;
    
    if (NULL == SrcAddr)
    {
        return -1;
    }

    nOffset = StartAddr;
    nBase_start = (nOffset / ERASE_BLOCK_SIZE); 
    nUnitCnt = ((nLen / ERASE_BLOCK_SIZE)+1);
    nFreeSize = (BLOCK_SIZE - (nOffset % ERASE_BLOCK_SIZE));

    buf = (char *)SrcAddr;
    nTmpLen = nLen;
    printf("base Start 0x%x,nUnitCnt  %d\n",nBase_start,nUnitCnt);
    for (i=0; i<nUnitCnt; i++)
    {
        memset(tmpbuf,0,ERASE_BLOCK_SIZE);
        
        xl_read_private_area((nBase_start+i)*ERASE_BLOCK_SIZE,ERASE_BLOCK_SIZE,tmpbuf);
        
        nWrittenSize = MIN(nTmpLen,nFreeSize);
        memcpy((tmpbuf+nOffset),buf,nWrittenSize);
        nTmpLen -= nWrittenSize;
        erase_block(CFG_MTD,nBase_start+i,1);
        
        flashfd = fopen(CFG_MTD, "wb");
        if (NULL == flashfd)
        {
            perror(" can't open device \n");
            return -1;
        }
    
        fseek(flashfd,(nBase_start+i)*BLOCK_SIZE,SEEK_SET);
        fwrite(tmpbuf, sizeof(unsigned char),BLOCK_SIZE,flashfd);
        fclose(flashfd);

        nTmpLen -= nWrittenSize;
        nFreeSize = ERASE_BLOCK_SIZE;
    }
    return 0;
}

/***********************************************************
Function: xl_read_private_area_flash
Description:  read data to
Input: nOffset : start read address based
       len: the length of the data be read
Output:
Return:
Others:
History:
************************************************************/
int xl_read_private_area_flash(int StartAddr,int len,char *DesAddr)
{
    char buf[MAXLEN];
    char * TmpDest;
    int nRet = 0;
    int size =0;
    int rc = 0;
    int nLen = 0;
    int nReadLen = 0;
    int nOffset = 0;
    FILE *flashfd = NULL;

    nOffset = StartAddr;
    flashfd = fopen(CFG_MTD, "rb");
    if (NULL == flashfd)
    {
         perror(" can't open device \n");
         return -1;
    }


    TmpDest = (char *)DesAddr;
    memset(buf,0,MAXLEN);
    fseek(flashfd,nOffset,SEEK_SET);
    nLen = len;
    while(1)
    {
        if ( nLen > MAXLEN)
        {
            nReadLen = MAXLEN;
        }
        else
        {
            nReadLen = nLen;
        }

        rc = fread(buf,sizeof(unsigned char), nReadLen,flashfd);
        if (0 >= rc)
        {
            nRet = 1;
            perror("read data from file error\n");
            break;
        }
        else
        {
            nLen -= nReadLen;
            size += rc;
            if (size >= nReadLen)
            {
                nRet = 0;
                memcpy((char *)DesAddr,buf,(rc-(size-nReadLen)));
                break;
            }
            else
            {
                memcpy(DesAddr,buf,rc);
                memset (buf,0,sizeof(buf));
            }
        }
    }
    
    if (NULL != flashfd)
    {
        fclose(flashfd);
    }
    return nRet;
}


#if 0
int main(int argc, char * argv[])
{
    int nRet =0 ;
    nRet = xl_rom_init();
    if (nRet != 0)
    {
        printf("error\n");
    }
    char buf[0x10000] = {0};

    xl_write_rom(0,"AAAAAAAA",9);
    xl_read_rom(0,9,buf);
    printf("buf read is %s \n",buf);
}
#endif


