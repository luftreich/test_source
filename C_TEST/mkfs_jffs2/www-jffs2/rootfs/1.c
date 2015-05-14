#include <stdio.h>
int main(void)
{
	char c_ch = '\0';
	char *ps_value="个";

	//	c_ch = *ps_value;
	//	++ps_value;

		printf("%x %x %x\n",ps_value[0],ps_value[1],ps_value[2]);
		if((c_ch & 128) && (*ps_value& 128 ) )
		{
		printf("是\n");

		}


	return 0;	
}
