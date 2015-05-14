#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <linux/unistd.h>

#define N 5
#include <semaphore.h>
sem_t m_sem;
pid_t gettid()
{
    //直接使用224代替__NR_gettid也可以
    return syscall(__NR_gettid);
}

void *thread_func(void *arg)
{
    printf("thread started, pid = %d\n", gettid());
    while (1) {
		sem_wait(&m_sem);
	    printf(" I am thread   %d\n ",gettid());
        sleep(1);
    }
}

void create_daemon(void);
int main(void)     
{     
    int i;
    pthread_t tid[N];
    sem_init(&m_sem,0,1);
    //daemon(0,1);
//	create_daemon();
    for (i = 0; i < N; i++) {
        pthread_create(&tid[i], NULL, thread_func, NULL);
    }
    sem_post(&m_sem);
    sem_post(&m_sem);
    sem_post(&m_sem);
    sem_post(&m_sem);
    sem_post(&m_sem);
    sleep(20);
    for (i = 0; i < N; i++) {
    pthread_cancel(tid[i]);
    pthread_join(tid[i],NULL);
    printf("Pthread join tid[%d] successfully\n",i);
    } 
    while (1) {
        sleep(1);
	printf("xxx\n");
    }
    return 0;
}

void create_daemon(void)
{
	    /* Our process ID and Session ID */
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) 
    {
        exit(EXIT_FAILURE);
    }

    /* If we got a good PID, then
    we can exit the parent process. */
    if (pid > 0) 
    {
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) 
    {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) 
    {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }
}
