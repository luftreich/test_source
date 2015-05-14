#include <stdio.h>    // for printf()  
#include <signal.h> 
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

void sigFunc()
{
   static int iCnt = 0;
   printf("The %d Times: Hello world\n", iCnt++);
}

int main(void)
{
   struct itimerval tv, otv;
   signal(SIGALRM, sigFunc);
   //how long to run the first time
   tv.it_value.tv_sec = 3;
   tv.it_value.tv_usec = 0;
   //after the first time, how long to run next time
   tv.it_interval.tv_sec = 600;
   tv.it_interval.tv_usec = 0;

   if (setitimer(ITIMER_REAL, &tv, &otv) != 0)
	printf("setitimer err %d\n", errno);

   while(1)
   {
	    sleep(1);
	    printf("otv: %d, %d, %d, %d\n", (int)otv.it_value.tv_sec, (int)otv.it_value.tv_usec, (int)otv.it_interval.tv_sec, (int)otv.it_interval.tv_sec);
   }
   return 0;
}
