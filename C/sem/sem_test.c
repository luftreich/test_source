#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include <pthread.h> 
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *arry;
};
 
static int sem_id = 0;
 
static int set_semvalue();
static void del_semvalue();
static int semaphore_p();
static int semaphore_v();

static void * test_thread1(void * args)
{
	int i =0;
	while (1)
	{
		/* 离开临界区，休眠随机时间后继续循环 */
        if(!semaphore_p())
        {
			printf("exit line [%d]\n",__LINE__);
            exit(EXIT_FAILURE);
        }
		i++;
		printf("test thread %d \n",i);
		sleep(1);
	}
}
int main(int argc, char *argv[])
{
 
    /* 创建信号量 */
    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
 
        /* 程序第一次被调用，初始化信号量 */
        if(!set_semvalue())
        {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }

	pthread_t p_tid = 0;
	pthread_create(&p_tid,NULL,test_thread1,NULL);


	while (1)
	{
		printf("please input a number : [1-10]\n");
		char c = getchar();
		int i = c-'0';
		if (i<1)
		{
			continue;
		}

		printf("the number is %d\n",i);
		int n = 0;
		for (n = 0 ;n < i; n++)
		{
			/* 进入临界区 */
			if(!semaphore_v())
			{
				printf("exit line [%d]\n",__LINE__);
				exit(EXIT_FAILURE);
			}
		}
	}

    if(argc > 1)
    {
        /* 如果程序是第一次被调用，则在退出前删除信号量 */
        sleep(3);
        del_semvalue();
    }
    exit(EXIT_SUCCESS);
}
 
static int set_semvalue()
{
    /* 用于初始化信号量，在使用信号量前必须这样做 */
    union semun sem_union;
 
    sem_union.val = 0;
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1)
    {
        return 0;
    }
    return 1;
}
 
static void del_semvalue()
{
    /* 删除信号量 */
    union semun sem_union;
 
    if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
    {
         fprintf(stderr, "Failed to delete semaphore\n");
    }
}

static int semaphore_p()
{
    /* 对信号量做减1操作，即等待P（sv）*/
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;//P()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_p failed\n");
        return 0;
    }
    return 1;
}
 
static int semaphore_v()
{
    /* 这是一个释放操作，它使信号量变为可用，即发送信号V（sv）*/
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;//V()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_v failed\n");
        return 0;
    }
    return 1;
}
