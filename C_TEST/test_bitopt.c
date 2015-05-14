#include <stdio.h>
#include <stdlib.h>

main()
{
	int i=get_i();
	printf("=== main return i == %d\n",i);
	printf("sizeof () == %d\n",sizeof(long));
	return 0;
}
int get_i(void)
{
	int i=0;
	int n=(1<<10)|(1<<4);
		
	printf("n == 0x%x\n",n);
	for(i=0;i<32;i++)
	{
		if (n&(1<<i))
		{
			printf("get bit i ==%d\n",i);
			printf("get bit i ==0x%x\n",1<<i);
		}
	}
	return 0;
}
