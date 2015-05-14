
int check_img_file(char * upload_file,int *pnType);

int caculate_crc_file(char * datafile);
    
int erase_block(const char *device_name,long start,int count);

int burn_file_to_mtd(char * file_name,int nType);

int burn_data(char * src_name,char * mtd_name,int nType);
