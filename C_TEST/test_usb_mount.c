#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#define MOUNT_FILE "/proc/mounts"
#define DEV_NAME "/dev/sd"
#define MOUNT_LOG_FILE "/tmp/_mount_log"
#define STDOUT (1)
void parsing_mount_log(char *buf);
void mnt_usb_get_mount_info(char *szDevName);
int main(void)
{
	char devname[32];
	char buf[256] = {0};
	char c = 'a';
	FILE * fd;
	int i;

	for(i=0;i<8;i++)
	{
		sprintf(devname,"%s%c",DEV_NAME,c);
		if( access(devname, F_OK)!=0 )
		{
			printf("%s is not exist \n",devname);
		}
		else
		{
			printf("%s is exist\n",devname);
			mnt_usb_get_mount_info(devname);
		
	
			fd = fopen(MOUNT_LOG_FILE,"r");
			if (NULL == fd)
			{
				printf("unable  open file %s ,errno %s\n",MOUNT_LOG_FILE,strerror(errno));
				return -1;
			}
	
			while (fgets(buf,sizeof(buf),fd)!=NULL)
			{
					printf("I read one line\n");
					parsing_mount_log(buf);
					memset(buf,0,sizeof(buf));
			}
			fclose(fd);
		}
		c +=1;
		memset(devname, 0 ,sizeof(devname));
	}
	return 0;
}
void mnt_usb_get_mount_info(char *szDevName)
{
	int nTmpFd,oldstdout;
	char command[1024]={0};
	char szMountinfo[2048]={0};
	
	nTmpFd = open(MOUNT_LOG_FILE,O_CREAT | O_RDWR);
	if (-1 == nTmpFd)
	{
		printf("open file %s failed \n",MOUNT_LOG_FILE);
		printf(" errno %s \n",strerror(errno));
	}
	oldstdout = dup(STDOUT);
	dup2 (nTmpFd,STDOUT);
	close(nTmpFd);
	sprintf(command,"cat %s | grep %s ",MOUNT_FILE,szDevName);
	system(command);
	
	dup2(oldstdout,STDOUT);
	close(oldstdout);
}
/*
   /dev/sda1 /tmp/media/usb1/sda1 vfat rw,relatime,fmask=0022,dmask=0022,codepage=cp437,iocharset=utf8,shortname=mixed,errors=remount-ro 0 0
 */

void parsing_mount_log(char *buf)
{
	char * info;
	char devname[32] = {0};
	char mountdir[256] = {0};
	char filesys[16] = {0};
	info = strchr(buf,' ');
	strncpy(devname,buf,(info-buf));
	printf("strlen devname %d ,devname %s\n",strlen(devname),devname);
	
	buf = info+1;
	info = strchr(buf,' ');
	strncpy(mountdir,buf,(info-buf));
	printf("mountdir: %s \n",mountdir);
	
	buf = info+1;
	info = strchr(buf,' ');
	strncpy(filesys,buf,(info-buf));
	printf("file system: %s \n",filesys);
	
}
