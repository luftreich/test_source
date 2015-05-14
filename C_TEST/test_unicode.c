#include <stdio.h>
#include <langinfo.h>
int main(void)
{
	{
	printf("code mode is %s \n",nl_langinfo(CODESET));
	}
}
