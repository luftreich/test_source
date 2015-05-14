#include <stdio.h>
#include <time.h>
#include <sys/select.h>
int main(void)
{
	struct timeval t_start,t_end;
	time_t t;
        struct tm *tm;
	while(1)
	{
	t = time(NULL);
	tm = localtime(&t);
	gettimeofday(&t_start,NULL);
	printf("[%04d/%02d/%02d %02d:%02d:%02d:%03d] \n",tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec,(t_start.tv_usec/1000));
	}
}
