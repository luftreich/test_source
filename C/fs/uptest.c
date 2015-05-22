#include <stdio.h>
#include <string.h>
#include "xl_common.h"
#define TEST_FILE "/tmp/dcdn_base/.test_disk_readonly"
int main(int argc, char *argv[])
{
	char	command[256] = {0};
	sprintf(command,"rm -rf %s",TEST_FILE);
	system(command);
	FILE *fp = fopen(TEST_FILE, "w");
    if ( fp == NULL)
	{
		printf("unable open file ,readonly system\n");
		return -1;
	}
	
	int numb = sizeof("test");
	int nRet = fwrite("test",1,sizeof("test"),fp);
		
	if (nRet != numb)
	{
		printf("can't write ,readonly system\n");
		fclose(fp);
		return -1;
	}
	else
	{
		printf("writable system\n");
	}

	fclose(fp);
	return 0;
}


