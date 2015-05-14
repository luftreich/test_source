#include<stdio.h>

//return 0 is big_endian return 1 is small endian

int checkSystem()
{

	union check
	{
		int i;
		char ch;
	}c;

	c.i = 1;

	return (c.ch == 1);
}

int main(void)
{
	int j = 0;
	j = checkSystem();
	printf("ret=%d\n",j);
	printf(" sizeof (ll) %d \n",sizeof(long long));
	if (1 == j) 
		printf("return it is small endian\n");
	if (0 == j)
		printf("return it is big_endian\n");
	return 0;
}

