#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void repipe_stdout(void); 
FILE *stream = NULL;
#define OPKG_OUT "/tmp/opkg_out.log"
/*
   thunder-miner-app - 1.0.3 - 1.0.4
 */
int opkg_list_upgradable(char * command)
{
    FILE * opkg_fd = NULL;
    char * p;
    char buffer [1024] = {0};
    char name [256] = {0};
    char version [32] = {0};
    char ser_ver[32] = {0};
    int ret = -1;

    opkg_fd = fopen(OPKG_OUT,"r");
    if (NULL == opkg_fd) 
    {
        return ret;
    }  
    while(NULL != fgets(buffer,sizeof(buffer),opkg_fd))
    {
      ret = 0;
      int n = 0;
      p = strtok(buffer, " ");
      while(p)  
      {   
              fprintf(stderr,"%s\n", p);  
              if (0 == n)
              {
                  sprintf(name,p);
              }
              if (1 == n)
              {
                  sprintf(version,p+1);
                  version[strlen(version)-1]=0;
                  break;
              }
              if (2 == n)
              {
                  sprintf(ser_ver,p+1);
                  ser_ver[strlen(ser_ver)-1]=0;
                  break;
              }
              p = strtok(NULL, "-");     
              n++; 
      }
       
      fprintf(stderr,"opkg-cl list-upgradable: name [%s] base-version [%s] server-version [%s] \n",name,version,ser_ver);      
      memset(name,0,sizeof(name));
      memset(version,0,sizeof(version));
      memset(ser_ver,0,sizeof(ser_ver));
    }
    fclose(opkg_fd);
    return ret;
}
/*
thunder-miner-app - 1.0.3   
 */
int opkg_list_installed(char * command)
{
    FILE * opkg_fd = NULL;
    char * p;
    char buffer [1024] = {0};
    char name [256] = {0};
    char version [256] = {0};
    int ret = -1;

    opkg_fd = fopen(OPKG_OUT,"r");
    if (NULL == opkg_fd) 
    {
        return ret;
    }  
    while(NULL != fgets(buffer,sizeof(buffer),opkg_fd))
    {
      ret = 0;
      int n = 0;
      p = strtok(buffer, " ");
      while(p)  
      {   
              fprintf(stderr,"%s\n", p);  
              if (0 == n)
              {
                  sprintf(name,p);
              }
              if (1 == n)
              {
                  sprintf(version,p+1);
                  version[strlen(version)-1]=0;
                  break;
              }
              p = strtok(NULL, "-");     
              n++; 
      }
      
      fprintf(stderr,"opkg-cl list-installed: name [%s] version [%s]\n",name,version);      
    }
    fclose(opkg_fd);
    return ret;
}

void opkg_upgrade()
{

}

void opkg_install(char * command)
{
}


void opkg_remove(char * command)
{

}
/*
test-opkg--app - 1.0.1 - Femon Video Disk Recorder plugin.
test-opkg-app - 1.0.1 - Femon Video Disk Recorder plugin.
thunder-miner-app - 1.0.2 - Femon Video Disk Recorder plugin.
*/
int opkg_list(void)
{
    FILE * opkg_fd = NULL;
    char * ptr = NULL;
    char * p;
    char buffer [1024] = {0};
    char name [256] = {0};
    char version [256] = {0};
    char context[1024] = {0};
    int ret = -1;
    
    opkg_fd = fopen(OPKG_OUT,"r");
    if (NULL == opkg_fd) 
    {
        return ret;
    }  
    while(NULL != fgets(buffer,sizeof(buffer),opkg_fd))
    {
      ret = 0;
      int n = 0;
      p = strtok(buffer, " ");
      while(p)  
      {   
              fprintf(stderr,"%s\n", p);  
              if (0 == n)
              {
                  sprintf(name,p);
              }
              if (1 == n)
              {
                  sprintf(version,p+1);
                  version[strlen(version)-1]=0;
              }
              if(2 == n)
              {
                  sprintf(context,p+1);
                  context[strlen(context)-2]=0;
                  break;
              }
              p = strtok(NULL, "-");     
              n++; 
      }
      
      fprintf(stderr,"name [%s] version [%s] context [%s]\n",name,version,context);      
    }
    fclose(opkg_fd);
    return ret;
}


int main(int argc, char *argv[])
{
    
    int ret = 0;
    char command[1024] = {0};
    while(1)
    {
        printf("start test\n");
        getchar();
        repipe_stdout();  
        gets(command);
        
        ret = system(command);
        if(strstr(command, "list-upgradable"))
        {
            opkg_list_upgradable(command);
        }
        else if(strstr(command,"list-installed"))
        {
            opkg_list_installed(command);
        }
        else if(strstr(command,"install"))
        {
            opkg_install(command);
        }
        else if(strstr(command,"remove"))
        {
            opkg_remove(command);
        }
        else if(strstr(command,"list"))
        {
            opkg_list();
        }

        fclose(stream);
        
        if (0 == ret)
        {
            fprintf(stderr,"%s successfully\n",command);
        }
        else
        {
            fprintf(stderr,"return %d \n",ret);
        }

        memset(command,0,sizeof(command));
    }
    return 0;
}

void repipe_stdout(void)  
{  
    stream = freopen(OPKG_OUT, "w", stdout ); // 重定向  
}



