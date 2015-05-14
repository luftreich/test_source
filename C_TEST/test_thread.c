#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <linux/unistd.h>

#define N 5

pid_t gettid()
{
    //直接使用224代替__NR_gettid也可以
    return syscall(__NR_gettid);
}

void *thread_func(void *arg)
{
    printf("thread started, pid = %d\n", gettid());
    while (1) {
		printf(" I am thread    ");
        sleep(1);
    }
}

int main(void)     
{     
    int i;
    pthread_t tid[N];

    for (i = 0; i < N; i++) {
        pthread_create(&tid[i], NULL, thread_func, NULL);
    }

    sleep(1);
    for (i = 0; i < N; i++) {
        printf("tid = %lu\n", tid[i]);
    }

//    while (1) {
        sleep(1);
  //  }
    return 0;
}
