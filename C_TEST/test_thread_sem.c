#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

int myglobal;
sem_t sem;

void * thread_function(void *arg)
{
	int i,j;
	while(1)
	{
		sem_wait(&sem);
		printf("first run in thread \n");
		sleep(1);
		//sem_post(&sem);
	}
	return NULL;
}


int main(void)
{
	pthread_t mythread;
	int i;

	sem_init(&sem, 0, 1);//信号量初始化
	if(pthread_create(&mythread, NULL, thread_function, NULL))
	{
		printf("create thread error!\n");
		abort();
	}
	printf("%d \n",__LINE__);
/*	sleep(1);*/

	for(i = 0; i < 5; i++)
	{
		//sem_wait(&sem);//=0
		myglobal = myglobal + 1;
		printf("o\n");
		sleep(1);
	//	sem_post(&sem);//=1
	}
	sem_post(&sem);
	sem_post(&sem);
	sem_post(&sem);
	sem_post(&sem);
	sem_post(&sem);


	while(1)
		sleep(100);
	exit(0);
}
