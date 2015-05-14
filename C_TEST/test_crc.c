#include <stdio.h>
#include <inttypes.h>
 
#define Poly 0xEDB88320L	//CRC32标准
static uint32_t crc_tab32[256];	//CRC查询表
 
static void init_crc32_tab(void);	//生成CRC查询表
uint32_t get_crc32(uint32_t crcinit, uint8_t * bs, uint32_t bssize);	//获得CRC
uint32_t GetFileCRC(FILE *fd);	//获得文件CRC
 
static void init_crc32_tab( void ) 
{
	int i, j;
	uint32_t crc;
 
	for (i=0; i<256; i++)
	{
		crc = (unsigned long)i;
		for (j=0; j<8; j++) 
		{
			if ( crc & 0x00000001L )
				crc = ( crc >> 1 ) ^ Poly;
			else      
				crc = crc >> 1;
		}
		crc_tab32[i] = crc;
	}
}
 
uint32_t get_crc32(uint32_t crcinit, uint8_t * bs, uint32_t bssize)
{
	uint32_t crc = crcinit^0xffffffff;
 
	init_crc32_tab();
	while(bssize--)
		crc=(crc >> 8)^crc_tab32[(crc & 0xff) ^ *bs++];
 
	return crc ^ 0xffffffff;
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
		crc = get_crc32(crc, crcbuf, rdlen);
		//printf("CRC %x\n",crc);
	}
	return crc;
}
 
int main(int argc,char **argv)
{
	FILE *fd;
	unsigned int value=0;
	
	int crc = 0;
	char * tmpchar="abcdefg";
	crc = get_crc32(crc,tmpchar,7);
	printf("crc 0x%x \n",crc);
	if(argc<2)
	{
		printf("Usage: %s file",basename(argv[0]));
		return 0;//	exit(1);
	}
	if((fd=fopen(argv[1],"r"))==NULL)	
	{
		perror("Error:");
		return 0;//exit(1);
	}
	value = GetFileCRC(fd);
	printf("CRC: %X\n",value);
 
	fclose(fd);
	return 0;
}
