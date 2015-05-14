#include <stdio.h>
typedef struct tagst_test
{
	int a;
	int b;
	int c;
	int d;
}st_test;
#define N 4
int main(void)
{
	int len;
	st_test * p_st;
	st_test * p_st1;
	p_st = malloc(sizeof(st_test)*N);
	printf("p_st 0x%x\n",&p_st[0]);
	printf("p_st1 0x%x\n",&p_st[1]);
	printf("p_st2 0x%x\n",&p_st[2]);
}
