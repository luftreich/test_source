#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
typedef unsigned int uint32;
typedef unsigned char uint8;
typedef unsigned int * uintptr;

#define IMG_MAGIC (0x00474D49)
#define APP_MAGIC (0x00505041)
struct img_header{
    uint32 nMagicNo;            /* IMG 0x00474D49*/
    							/*APP 0x00505041*/
    uint8 version[12];        /* version */
    uint8 board_id[16];       /* board id */
    uint8 product[16];        /* product name */
    uint8 reserved[16];       /* reserved unuse */
};

#define CRC32_INIT_VALUE 0xffffffff
#define	OFFSETOF(type, member)	((uint32)(uintptr)&((type *)0)->member)
#define TRX_MAX_OFFSET (3)
#define TRX_MAGIC_STR "HDR0"
#define TRX_MAGIC (0x30524448)
#define MAX_FIRMWARE_SIZE (0x100000*32)
#define MIN(a,b)  (((a)<(b))?(a):(b))
#define CRC_INNER_LOOP(n, c, x) \
	(c) = ((c) >> 8) ^ crc##n##_table[((c) ^ (x)) & 0xff]


char *crc32_table = "I threw a wish in the well\n\
Don’t ask me I’ll never tell\n\
I looked to you as it fell\n\
and now you’re in my way\n\
I’d trade my soul for a wish\n\
Pennies and dimes – for a kiss\n\
I wasn’t looking for this\n\
But now you’re in my way\n\
Your stare was holding\n\
Ripped jeans – skin was showing\n\
Hot night – Wind was blowing\n\
Where d’ you think you’re going baby?\n\
Hey I just met you\n\
and this is crazy\n\
But here’s my number\n\
So, Call Me Maybe\n\
It’s hard to look right\n\
at you baby!\n\
But here’s my number\n\
So, Call Me Maybe\n\
Hey I just met you\n\
and this is crazy\n\
But here’s my number\n\
So, Call Me Maybe\n\
and all the other boys\n\
try to chase me\n\
But here’s my number\n\
So, Call Me Maybe\n\
";
int main(int argc,char **argv)
{
	int fd;
	unsigned int value=0;
	int nRet;
	int crc = 0;
	int nType;
	int nDataLen = 0;
    int i =0;

    if(argc<2)
	{
		printf("Usage: %s file \n",argv[0]);
		return 0;//	exit(1);
	}
    
    while(i <100)
    {
    char * file_name = argv[1];
    printf("write data to file %s_%d \n",file_name,i);
    
    char file_path[256] = {0};
    sprintf(file_path,"/tmp/thunder/%s_%d",file_name,i);
    fd = open(file_path,O_WRONLY|O_CREAT);
    if (NULL == fd)
    {
        perror("open file error \n");
        return -1;
    }


    nRet = write(fd,crc32_table,strlen(crc32_table));
    
    if (nRet != strlen(crc32_table))
    {
        printf("Write file error nRet %d ,strlen %d \n",nRet,strlen(crc32_table));
    }

    close(fd);
    i++;
    }
	return nRet;
}

 
