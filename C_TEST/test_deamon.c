#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
void module_manager_thread(void );
int g_DBG=0;
int main(int argc, char *argv[]) 
{
        
        /* Our process ID and Session ID */
        pid_t pid, sid;
        
        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) {
                exit(EXIT_SUCCESS);
        }

        /* Change the file mode mask */
        umask(0);
        int i;       
	for(i=0;i<10;i++)
	{
		printf("-- -- -- -- -- -\n");
		usleep(100);
	}
        /* Open any logs here */        
                
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
                /* Log the failure */
                exit(EXIT_FAILURE);
        }
        

        
        /* Change the current working directory */
        if ((chdir("/")) < 0) {
                /* Log the failure */
                exit(EXIT_FAILURE);
        }
        
		if(argv[0]==0)
		{
				
		}
        /* Close out the standard file descriptors */
        if (g_DBG != 1)
		{
		//	close(STDIN_FILENO);
		//	close(STDOUT_FILENO);
		//	close(STDERR_FILENO);
		}
        pthread_t ntid;
     
     /* 启动线程A: 接收模块加载和卸载的消息队列*/
     int err = pthread_create(&ntid,NULL,(void *)module_manager_thread,NULL);
     if(err != 0)
     {
         printf("can't create thread: %s\n",strerror(err));
         return 1;
    }
        /* Daemon-specific initialization goes here */
        
        /* The Big Loop */
        while (1) {
           /* Do some task here ... */
           printf("#");
           sleep(1); /* wait 30 seconds */
        }
   exit(EXIT_SUCCESS);
}
void module_manager_thread(void )
{
	while(1)
	{
		printf("This is child pid %d \n",getpid());
		sleep(1);
	}
}
