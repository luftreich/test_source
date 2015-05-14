#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include "xl_common.h"
#include "test_upgrade.h"
#include "image.h"

typedef unsigned int uint32;
typedef unsigned char uint8;
typedef unsigned int * uintptr;
extern unsigned long crc32(unsigned long crc,const unsigned char * buf,unsigned int len);
#define IMG_MAGIC (0x00474D49)
#define APP_MAGIC (0x00505041)
#define XL_ROM_MAGIC (0x004c4c41)
struct img_header{
    uint32 nMagicNo;            /* IMG 0x00474D49*/
    							/*APP 0x00505041*/
    uint8 version[12];        /* version */
    uint8 board_id[16];       /* board id */
    uint8 product[16];        /* product name */
    uint8 reserved[16];       /* reserved unuse */
};

static void print_header (image_header_t *hdr);

int main(int argc,char **argv)
{
	int nRet;
    if(argc<2)
	{
		printf("Usage: %s [filename] \n",argv[0]);
		return 0;//	exit(1);
	}

    #if 0
    nRet = check_img_file(argv[1],&nType);
    if(nRet != 0){
        fprintf(stderr,"%s:File is not a img file \n",argv[1]);
        nRet = -1;
        return nRet;
    }
    #endif

    #if 0
    nRet = caculate_crc_file(argv[1]);
	if (0 == nRet)
	{
        printf("CRC checknum is good\n");
	}
	else
	{
		printf("caculate crc error,%d\n",nRet);
		nRet = -2;
		return nRet;
	}
    
    printf("caculate crc over\n");
    #endif

    nRet = erase_block("/dev/mtdblock1",0,0xFFFFFFFF);
    if (0 != nRet)
    {
            printf("Error erase blocks\n");
            return nRet;
    }

    nRet = burn_file_to_mtd(argv[1],0);
	if (0 == nRet)
	{
		printf("Upgrade  file  %s success\n",argv[1]);
	}
	else
	{
		printf("caculate crc error\n");
		nRet = -3;
		return nRet;
	}

    /*
    int tmpcrc=0;
    caculate_crc_mtd("/dev/mtdblock5",&tmpcrc);
    printf("/dev/mtdblock5 crc is 0x%08x\n",tmpcrc);
	*/
    return nRet;
}
	
int safe_fread(void *DstAddr,int cnt,int size,FILE *pfile)
{
	int num;
	num = fread(DstAddr,cnt,size,pfile);
	return num;
}
int check_img_file(char * upload_file,int *pnType)
{
#if 1
    FILE *pfile = NULL;
	int ret = 0;
	int count = 0;
	int nType;
    struct img_header img;
    
    if (NULL == upload_file)
    {
    	ret = -1;
    	goto err;
    }

    memset(&img,0,sizeof(img));
	if ((pfile = fopen(upload_file, "rb")) == NULL){
		ret = errno;
		goto err;
	}
	
    count = safe_fread(&img,1,sizeof(struct img_header),pfile);
    if(count != sizeof(struct img_header)){
        fprintf(stderr,"%s:File is not a img file \n",upload_file);
        ret = -1;
        goto err;
    }

    if (img.nMagicNo == IMG_MAGIC)
    {
    	nType = 0;//KERNEL+ROOTFS
    } 
    else if(img.nMagicNo == APP_MAGIC)
    {
    	nType = 1; //XLAPP
    }
    else if(img.nMagicNo == XL_ROM_MAGIC)
    {
        nType = 2; //XL_ROM
    }
    else
    {
    	printf("This file is not a img format file \n");
    	ret = -1;
    	goto err;
    }

    * pnType = nType;
	
	if (NULL != pfile)
	{
		fclose(pfile);
	}
    
    printf ("The rom type index is %d \n",nType);
    return ret;
    /*compare the img version with the current img*/
    //ret = compare_img_info(nType,img.version);
err:
   	printf("err\n");
	if (NULL != pfile)
	{
		fclose(pfile);
	}
	return -1;
#else
	FILE *pfile = NULL;
	int count;
    int ret;
    image_header_t uImage_head;

	/*check image file*/
	/*now only support for image.img file check*/
	if ((pfile = fopen(upload_file, "rb")) == NULL){
		ret = -1;
		return ret;
	}
	
	/*
    fseek(pfile,sizeof(struct img_header),SEEK_SET);
	*/
    /*Read the image header struct */
    count = safe_fread(&uImage_head, 1, sizeof(struct image_header), pfile);
	if (count < sizeof(struct image_header)) {		
		fprintf(stderr, "%s: File is too small (%d bytes)\n", upload_file, count);
		fclose(pfile);
        return -2;
	}
    
	/* Examine Image header */
    *pnType = 0;
    return 0;
#endif
}

int caculate_crc_file(char * datafile)
{

	int fd;
	struct stat sbuf;
	unsigned char *ptr;
    int nRet = 0;
    int offset  = 0;
    image_header_t * hdr;
    image_header_t header;

    memset(&header,0,sizeof(image_header_t));
    hdr = &header;
    
    char * cmdname = "caculate_crc_file";
	
    if ((fd = open(datafile, O_RDONLY)) < 0) {
		printf("open file error\n");
        nRet = -1;
        goto err;
	}

	if (fstat(fd, &sbuf) < 0) {
		printf ("%s: Can't stat %s: %s\n",
			cmdname, datafile, strerror(errno));

		nRet = -2;
        goto err;
	}
    
    offset = sizeof(struct img_header);
	ptr = (unsigned char *)mmap(0, sbuf.st_size,
				    PROT_READ, MAP_SHARED, fd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		printf ("%s: Can't read %s: %s\n",
			cmdname, datafile, strerror(errno));
	    nRet = -3;
        goto err;
    }

    memcpy(hdr,ptr+offset,sizeof(image_header_t));

    if (ntohl(hdr->ih_magic) != IH_MAGIC) 
    {
        printf ("%s: Bad Magic Number: \"%s\" is no valid image\n",
				cmdname, datafile);
        nRet = -4;
        goto err;
	}

	char * data = (char *)hdr;
	int len  = sizeof(image_header_t);

	int checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = htonl(0);	/* clear for re-calculation */

	if (crc32 (0,(const unsigned char *) data, len) != checksum) 
    {
        printf ("*** Warning: \"%s\" has bad header checksum!\n",
				datafile);
        nRet = -5;
        goto err;
	}

	data = (char *)(ptr + offset + sizeof(image_header_t));
	len  = sbuf.st_size - offset - sizeof(image_header_t) ;

	if (crc32 (0,(const unsigned char *) data, len) != ntohl(hdr->ih_dcrc)) 
    {
        printf ("*** Warning: \"%s\" has corrupted data!\n",
				datafile);
        nRet = -6;
        goto err;
    }

	/* for multi-file images we need the data part, too */
	print_header ((image_header_t *)(ptr+offset));

err:
	(void) munmap((void *)ptr, sbuf.st_size);
	(void) close (fd);

    return nRet;
}

static void print_header (image_header_t *hdr)
{
	time_t timestamp;
	uint32_t size;

	timestamp = (time_t)ntohl(hdr->ih_time);
	size = ntohl(hdr->ih_size);

	printf ("Image Name:   %.*s\n", IH_NMLEN, hdr->ih_name);
	printf ("Created:      %s", ctime(&timestamp));
	printf ("Data Size:    %d Bytes = %.2f kB = %.2f MB\n",
		size, (double)size / 1.024e3, (double)size / 1.048576e6 );
	printf ("Load Address: 0x%08X\n", ntohl(hdr->ih_load));
	printf ("Entry Point:  0x%08X\n", ntohl(hdr->ih_ep));
	printf ("Kernel Size:  0x%08X\n", ntohl(hdr->ih_ksz));

	if (hdr->ih_type == IH_TYPE_MULTI || hdr->ih_type == IH_TYPE_SCRIPT) {
		int i, ptrs;
		uint32_t pos;
		unsigned long *len_ptr = (unsigned long *) (
					(unsigned long)hdr + sizeof(image_header_t)
				);

		/* determine number of images first (to calculate image offsets) */
		for (i=0; len_ptr[i]; ++i)	/* null pointer terminates list */
			;
		ptrs = i;		/* null pointer terminates list */

		pos = sizeof(image_header_t) + ptrs * sizeof(long);
		printf ("Contents:\n");
		for (i=0; len_ptr[i]; ++i) {
			size = ntohl(len_ptr[i]);

			printf ("   Image %d: %8d Bytes = %4d kB = %d MB\n",
				i, size, size>>10, size>>20);
			if (hdr->ih_type == IH_TYPE_SCRIPT && i > 0) {
				/*
				 * the user may need to know offsets
				 * if planning to do something with
				 * multiple files
				 */
				printf ("    Offset = %08X\n", pos);
			}
			/* copy_file() will pad the first files to even word align */
			size += 3;
			size &= ~3;
			pos += size;
		}
	}
}

int caculate_crc_mtd(char * upload_file,int *pnCRC)
{
	return 0;
}

int burn_file_to_mtd(char * file_name,int nType)
{
	int nRet = 0;

    nRet = burn_data(file_name,"/dev/mtdblock1",nType);

	return nRet;
}

#define MAXLEN 1024
int burn_data(char * src_name,char * mtd_name,int nType)
{
    int enable_print = 1;
    FILE * outfile,* infile;
    unsigned char buf[MAXLEN];
    int cnt=0;
    int rc;

    if( mtd_name == NULL || mtd_name == NULL)
    {
        printf("This is invlue param\n");
        return -1;
    }

    outfile = fopen(mtd_name, "wb" );
    infile = fopen(src_name,"rb");
    if( outfile == NULL || infile == NULL)
    {
        return -2;
    }   


    printf("Start write nvram into flash ... %s \n",mtd_name);
    memset(buf,0,sizeof(buf));
    while( (rc = fread(buf,sizeof(unsigned char), MAXLEN,infile)) != 0 )
    {
        fwrite( buf, sizeof( unsigned char ), rc, outfile );
        fflush(outfile);
      	memset(buf,0,sizeof(buf));
        if (1 == enable_print)
        {
            if(cnt!=0 && (cnt%128 == 0))
                printf("\n");
            if(cnt%2 == 0)
            {
                printf("*");
                fflush(stdout);
            }
            cnt ++;
        }
	}
	fclose(infile);
	fclose(outfile);
    printf("\r\n Upgrade over\n");
	return 0;
}
