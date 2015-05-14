#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
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

struct trx_header {
uint32 magic; /* "HDR0" (0x30524448)*/
uint32 len; /* Length of file including header */
uint32 crc32; /* 32-bit CRC from flag_version to end of file */
uint32 flag_version; /* 0:15 flags, 16:31 version */
uint32 offsets[3];/*offset 0 是第一个bin的起始地址*/
				  /*offset 1 是第二期bin的起始地址*/
}; 

uint32 hndcrc32(uint8 *pdata, uint32 nbytes, uint32 crc);
static const uint32 crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

int main(int argc,char **argv)
{
	FILE *fd;
	unsigned int value=0;
	int nRet;
	int crc = 0;
	int nType;
	
    if(argc<2)
	{
		printf("Usage: %s file \n",argv[0]);
		return 0;//	exit(1);
	}

	nRet = check_img_file(argv[1],&nType);
    if(nRet != 0){
        fprintf(stderr,"%s:File is not a img file \n",argv[1]);
        nRet = -1;
        return nRet;
    }

	nRet = caculate_crc_file(argv[1],&crc);
	if (0 == nRet)
	{
		printf("caculate crc success\n");
	}
	else
	{
		printf("caculate crc error\n");
		nRet = -2;
		return nRet;
	}

	nRet = burn_file_to_mtd(argv[1],nType);
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

	return nRet;
}

uint32_t GetFileCRC(FILE *fd)
{
	uint32_t size = 16 * 1024;
	uint8_t crcbuf[size];
	uint32_t rdlen;
	uint32_t crc = 0;	//CRC初始值为0
 
	while((rdlen = fread(crcbuf, sizeof(uint8_t), size, fd)) > 0)
	{
		//printf("crc %x crcbuf %s rdlen %d \n",crc,crcbuf,rdlen);
		crc = hndcrc32(crcbuf, rdlen,crc);
		//printf("CRC %x\n",crc);
	}
	return crc;
}
	
uint32 hndcrc32(uint8 *pdata, uint32 nbytes, uint32 crc)
{
	uint8 *pend;

	pend = pdata + nbytes;
	while (pdata < pend)
		CRC_INNER_LOOP(32, crc, *pdata++);

	return crc;
}

int safe_fread(void *DstAddr,int cnt,int size,FILE *pfile)
{
	int num;
	num = fread(DstAddr,cnt,size,pfile);
	return num;
}

int check_img_file(char * upload_file,int *pnType)
{
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
}
int caculate_crc_file(char * upload_file,int *pnCRC)
{
	FILE *pfile = NULL;
	int first,len,read_len,count;
	int ret,crc_len=0;
	int nType = 0;
	int crc;
	char tmp_buf[512];
	struct trx_header trx;
	struct img_header img;

	printf("nType num %d \n",nType);
	/*check image file*/
	/*now only support for image.img file check*/
	if ((pfile = fopen(upload_file, "rb")) == NULL){
		ret = errno;
		goto err;
	}
	
	fseek(pfile,sizeof(struct img_header),SEEK_SET);
	count = safe_fread(&trx, 1, sizeof(struct trx_header), pfile);
	if (count < sizeof(struct trx_header)) {		
		fprintf(stderr, "%s: File is too small (%d bytes)\n", upload_file, count);
		ret = -2;
		goto err;
	}

	/* Examine TRX header */
	if (trx.magic != TRX_MAGIC ||
	    trx.len > MAX_FIRMWARE_SIZE ||
	    trx.len < sizeof(struct trx_header)) {
		fprintf(stderr, "%s: Bad trx header\n", upload_file);
		ret = -3;
		goto err;
	}

	/* Calculate CRC over header */
	crc = hndcrc32((uint8 *) &trx.flag_version,
	               sizeof(struct trx_header) - OFFSETOF(struct trx_header, flag_version),
	               CRC32_INIT_VALUE);

	first = 1;
	for (len = trx.len - sizeof(struct trx_header); len; len -= count) {
		if (first) {
			read_len = MIN(len, sizeof(tmp_buf) - sizeof(struct trx_header));
			first = 0;
		} else
			read_len = MIN(len, sizeof(tmp_buf));

		count = safe_fread(tmp_buf, 1, read_len, pfile);
		if (count != read_len) {
			ret = -1;
			goto err;
		}
		
		crc_len += count;
		/*calculate crc value*/
		crc = hndcrc32((uint8 *) &tmp_buf, count, crc);
	}

	printf("crc 0x%x trx_crc 0x%x crc_len %d trx.len %d\n",crc,trx.crc32,crc_len,trx.len);
	/* Check CRC before writing if possible */
	if (crc_len == (trx.len-sizeof(struct trx_header))) {
		if (crc != trx.crc32) {
			printf("%s: Bad CRC\n", upload_file);
			ret = -1;
			goto err;
		}
		else
		{
			*pnCRC = trx.crc32;
		}
	}

	if (NULL != pfile)
	{
		fclose(pfile);
	}
	return 0;
err:
	printf("err\n");
	if (NULL != pfile)
	{
		fclose(pfile);
	}
	return -1;
}

int burn_file_to_mtd(char * file_name,int nType)
{
	int nRet = 0;

	FILE * upload_file = NULL;

	if(nType == 0) /*kernel*/
	{
		nRet = burn_data(file_name,"/dev/mtd2");
	}
    else if(nType == 1) /*xlapp*/
	{
		nRet = burn_data(file_name,"/dev/mtd4");
	}
	else
	{
		printf("Can't recongnize the file %s type \n",file_name);
		return -1;
	};

	return nRet;
}
#define MAXLEN 1024
int burn_data(char * src_name,char * mtd_name)
{
    int enable_print = 1;
    FILE * outfile,* infile;
    unsigned char buf[MAXLEN];
    int cnt=0;
    int nRet = 0;
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
        printf("%s",mtd_name,"not exit\n");
        return -2;
    }   

    nRet = fseek(infile,sizeof(struct img_header),SEEK_SET);
    if (0 != nRet)
    {
    	printf("unable to seek \n");
    	return -3;
    }
    printf("Start write data into flash ... %s \n",mtd_name);
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
