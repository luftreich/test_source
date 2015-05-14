#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
using namespace std;
int test_fork(void)
{
        pid_t pid;
        pid =fork();
        if (pid<0)
                exit(0);
        else if (pid == 0)
        {
                //如果是子进程 睡眠20秒
                cout<<"children : "<<getpid()<<endl;
				cout<<"parent : "<<getppid()<<endl;
				return (0);
                sleep(20);
        }
        else
         { cout<<"hello! i'm parent process!"<<endl;
                //如果是父进程在这里等待
                pid_t pr = waitpid(pid,NULL,0);
                cout<<"return child pid "<<pr<<endl;
        }

        return 0;
}
int main(void)
{
	int ret;
	ret = test_fork();
	
	cout<<"curent pid : "<<getpid()<<endl;
	return ret;
}