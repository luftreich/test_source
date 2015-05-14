#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>   
#include <getopt.h>   

typedef unsigned int uint32;
typedef unsigned char uint8;
typedef unsigned int * uintptr;
/*
*
* mkflashimage -IMG -VER 55826 -PRO BCM47081S_NEUTRAL -BID 0123456789 $OUTPUT/linux.trx  $OUTPUT/test.img  
*
*/
#define IMG_MAGIC (0x00474D49)
#define APP_MAGIC (0x00505041)
#define MAXLEN (1024)
struct img_header{
    uint32 nMagicNo;            /* IMG 0x00474D49*/
    							/*APP 0x00505041*/
    uint8 version[12];        /* version */
    uint8 board_id[16];       /* board id */
    uint8 product[16];        /* product name */
    uint8 reserved[16];       /* reserved unuse */
};
static const char *optString = "t:v:p:b:o:i:?h";

int create_img_file(struct img_header ImgHeader,char * OutFileName,char * InFileName)
{   
    FILE * outfile, *infile;
    outfile = fopen(OutFileName, "wb" );
    infile = fopen(InFileName, "rb");
    unsigned char buf[MAXLEN];
    if( outfile == NULL || infile == NULL )
    {
        printf("%s, %s,not exit",OutFileName,InFileName);
        return -1;
    }   

    int cnt=0;
    int rc;

    fwrite((char *)&ImgHeader, sizeof(char),sizeof(struct img_header), outfile);
    fflush(outfile);

    while( (rc = fread(buf,sizeof(unsigned char), MAXLEN,infile)) != 0 )
    {
        fwrite( buf, sizeof( unsigned char ), rc, outfile );
        fflush(outfile);
	}
	fclose(infile);
	fclose(outfile);
	return 0;
}

int display_usage(void)
{
	printf("usage: \n    -t <type:0-IMG,1-APP> \n -v <version:maxlen 12 Bytes> \n \
		-p <product name:maxlen 16 Bytes> \n -b <board_id:maxlen 16 Bytes>\n ? -h <help info> \n");
}

int main(int argc, char *argv[])
{
    int opt = 0;
    int ret =0;
    int nType = 0;
    char * pstr = NULL;
    struct img_header imghead;
    char output[128] = {0};
    char input[128] = {0};
    /* Initialize globalArgs before we get to work. */
    memset(&imghead,0,sizeof(imghead));

    opt = getopt( argc, argv, optString );
    while( opt != -1 ) {
        switch( opt ) {
            case 't':
            	if(strlen(optarg) > 1)
            	{
            		printf("unknow type return \n");
            		return -1;
            	}
            	nType = atoi(optarg);
            	printf("nType : %d \n",nType);
                if(0 == nType)
                {
                	imghead.nMagicNo = 0x00474D49;
                }
            	else if(1 == nType)
            	{
            		imghead.nMagicNo = 0x00505041;
            	}
            	else
            	{
            		printf("invalid type parem ,return \n");
            		return -1;
            	}
                break;
                
            case 'v':
                pstr = optarg;
                if (strlen(pstr) > (sizeof(imghead.version)-1))
                {
					printf(" the version string is too long ,return \n");
					return -1;                	
                }
                strcpy((char *)&(imghead.version),pstr);
                break;
                
            case 'p':
                pstr = optarg;
                if (strlen(pstr) > (sizeof(imghead.product)-1))
                {
                    printf(" the product string is too long ,return \n");
                    return -1;                	
                }
                strcpy((char *)&imghead.product,pstr);
                break;
                
            case 'b':
                pstr = optarg;
                if (strlen(pstr) > (sizeof(imghead.board_id)-1))
                {
                    printf(" the board_id string is too long ,return \n");
                    return -1;                	
                }
                strcpy((char *)&imghead.board_id,pstr);
                break;
            case 'o':
            	if(strlen(optarg) > 127)
            	{
                    printf("out put file name is too long \n");
                    return;            		
            	}
            	strcpy((char *)output,optarg);
            	break;
            case 'i':
            	if(strlen(optarg) > 127)
                {
                    printf("input file name is too long \n");
                    return;            		
                }
                strcpy((char *)input,optarg);
            	break;	
            case 'h':   /* fall-through is intentional */
            case '?':
                display_usage();
                break;
                
            default:
                /* You won't actually get here. */
                break;
        }
        
        opt = getopt( argc, argv, optString );
    }
    
    if (!strcmp(output,input))
    {
    	printf(" output file is as same as the input file ,please input different name \n");
    	return -1;
    }

    printf("\n Magic %s ,version %s product %s ,board_id %s,input %s,output %s \n",(char *)&(imghead.nMagicNo),imghead.version,imghead.product,imghead.board_id,input,output);
    if (nType == 0)
    {
    	printf("Start make IMG type file \n");
	}
	else if (nType == 1)
	{
		printf("Start make APP type file \n");
    }
    
    ret = create_img_file(imghead,output,input);

    if (ret == 0)
    {
    	printf(" make img %s successfully \n",output);
    }
    else
    {
    	printf(" make img failed \n");
    }
    return ret;
}

