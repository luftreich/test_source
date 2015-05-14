#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
int main( void )
{
    pid_t childpid;
    pid_t test_pid;
    int status;
//    signal(SIGCHLD,SIG_IGN);
    childpid = fork();
    if ( childpid < 0 )
    {
        perror( "fork()" );
        exit( EXIT_FAILURE );
    }
    else if ( childpid == 0 )
    {
        puts( "In child process" );
        sleep( 3 );//让子进程睡眠3秒，看看父进程的行为
        printf("\tchild pid = %d\n", getpid());
        printf("\tchild ppid = %d\n", getppid());
        exit(EXIT_SUCCESS);
    }
    else
    {
        test_pid = waitpid( childpid, &status, 0 );
        if (test_pid != childpid)
        {
            perror(" waitpid ");
        }
        puts( "in parent" );
        printf( "test_pid is %d,childpid is %d \n",test_pid,childpid );
        printf( "\tparent pid = %d\n", getpid() );
        printf( "\tparent ppid = %d\n", getppid() );
        printf( "\tchild process exited with status %d \n", status );
    }
    exit(EXIT_SUCCESS);
}

