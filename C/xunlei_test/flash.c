#include "xl_common.h"
#define MEDIA_READ_FILE "/media/.flashread"
#define MEDIA_WRITE_FILE "/media/.flashwrite"

static pthread_t tid_w=0;
static pthread_t tid_r=0;
static int flash_w_break = 0;
static int flash_r_break = 0;


static int CreateMyFile(char * szFileName,int nFileLength)
{
   FILE* fp = fopen(szFileName, "wb+"); // 创建文件
   if(fp==NULL)
   {
           printf("creat file error\n");
       return -1;
   }
   else
   {
	   fseek(fp, nFileLength-1, SEEK_SET);    // 将文件的指针 移至 指定大小的位置
	   fputc(32, fp);                        // 在要指定大小文件的末尾随便放一个数据
	   fclose(fp);
           return 0;
   }
}

static void * write_flash_all_thread(void)
{
    char command[1024] = {0};
    char file_name[1024] = {0};
    int i=0;
        
    sprintf(file_name,"/tmp/.flashwrite");
    
    if (access(file_name,F_OK) != 0)
    {
          CreateMyFile(file_name,1024*1024*10);
    }
    
    flash_w_break = 0;
    while(1)
    {
        if(flash_w_break)
        {
                break;
        }
        
        memset(command,0,sizeof(command));
        sprintf(command,"cp  /tmp/.flashwrite  %s",MEDIA_WRITE_FILE);
        fprintf(stdout,"flash write to %s, %d\r",MEDIA_WRITE_FILE,i);
        if(i==100)
            i=0;
        i++;
        system(command);
        system("sync");
        memset(command,0,sizeof(command));
        sprintf(command,"rm -rf %s",MEDIA_WRITE_FILE);
        system(command);
        system("sync");
        fflush(stdout);
   }
    return NULL;
}
static void * read_flash_all_thread(void)
{
    char command[1024] = {0};
    int i=0;
        
    if (access(MEDIA_READ_FILE,F_OK) != 0)
    {
          CreateMyFile(MEDIA_READ_FILE,1024*1024*10);
    }
    
    memset(command,0,sizeof(command));
    sprintf(command,"cp  -a %s /tmp/.flashread",MEDIA_READ_FILE);
     
    flash_r_break=0;
    while(1)
    {

        fprintf(stdout,"read flash %d\r",i);

        if (i==100)
           i=0;
        i++;
        system(command);
        if(flash_r_break)
        {
            break;
        }
        fflush(stdout);
   }
    return NULL;
}

static void do_write_flash_all(void)
{
        if (tid_w == 0)
        {
             printf("\n");
             pthread_create(&tid_w,NULL,(void *)write_flash_all_thread,NULL);
        }
}

static void do_read_flash_all(void)
{
        if (tid_r == 0)
        {
             printf("\n");
             pthread_create(&tid_r,NULL,(void *)read_flash_all_thread,NULL);
        }
}

static void print_usage(FILE *stream)
{
    fprintf(stream,"Exit : please input 'q','quit',or 'exit' \n");
    fprintf(stream,"Input \"allread\" to read flash always\n");
    fprintf(stream,"Input \"allwrite\" to write flash always\n");
}

static int mnt_flash_term_data(void)
{
    flash_r_break=1;
    flash_w_break =1;
    
    if(0!=tid_w)
    {
            pthread_cancel(tid_w);
            pthread_join(tid_w,NULL);
            tid_w=0;
    }

    if(0!=tid_r)
    {
            pthread_cancel(tid_r);
            pthread_join(tid_r,NULL);
            tid_r=0;
    }
    return 0;
}

int flash_module_test(void)
{
     char input[256] = {0};
     print_usage(stdout);
     while (1)
     {
         printf("flash >>> ");

         memset(input,0,sizeof(input));
         fgets(input,256,stdin);
         if (!strncmp("q",input,1)||!strncmp("quit",input,4)||!strncmp("exit",input,4))
         {
             mnt_flash_term_data();
             fprintf(stdout,"quit flash test \n");
             break;
         }

         if (!strncmp("allread",input,7))
         {
               do_read_flash_all();
         }
         else if (!strncmp("allwrite",input,8))
         {
               do_write_flash_all();
         }
         else 
         {
               fprintf(stdout,"error command for led\n");
               print_usage(stdout);
         }

     } 
     return 0;
}

