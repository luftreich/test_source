#include <sys/vfs.h>
#include <sys/stat.h>
#include <stdio.h>
#define MOUNT_PATH "/tmp/media/usb1/sda1"
int main(void)
{
	int nRet;
	struct statfs disk_statfs;
	long long freespace;
	long long totalspace;
	nRet = statfs(MOUNT_PATH,&disk_statfs);
	if (nRet >=0)
	{
        	freespace = (((long long)disk_statfs.f_bsize * (long long)disk_statfs.f_bfree)/(long long)1024);
		totalspace = (((long long)disk_statfs.f_bsize * (long long)disk_statfs.f_blocks) /(long long)1024);
	}
	printf("fbsize %d ,fbfree %d, fblocks %d \n",disk_statfs.f_bsize,disk_statfs.f_bfree,disk_statfs.f_blocks);
	printf("freespace %ld ,totalspace %ld\n",freespace,totalspace);
}
