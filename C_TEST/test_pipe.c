#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <unistd.h>

int main()
{
        pid_t pid;

        int pipe_fdr[2];
        int pipe_fdw[2];

        if (pipe(pipe_fdr) == -1)
        {
                perror("pipe failed!/n");
                exit(0);
        }

        if (pipe(pipe_fdw) == -1)
        {
                perror("fork failed!/n");
                exit(0);
        }

        pid = fork();

        if (pid == 0)
        {/*子进程操作*/
                /*关闭标准输出描述符(自动),并复制pipe_fdr[0]到标准输出描述符,返回值为最小及尚未使用的文件描述符*/
                if (dup2(pipe_fdr[0], 0) == -1)
                {
                        perror("dup failed!/n");
                        exit(0);
                }
                //*关掉多于的管道*/
                close(pipe_fdr[0]);
                close(pipe_fdr[1]);

                /*关闭标准输入(自动,并把pipe_fdw[1]复制到标准输入描述符*/
                if (dup2(pipe_fdw[1], 1) == -1)
                {
                        perror("dup failed!/n");
                        exit(0);
                }

                /*关闭多于的管道*/
                close(pipe_fdw[0]);
                close(pipe_fdw[1]);

                if (execl("./cal", "cal", NULL) < 0)
                {
                        printf("Error!!/n");
                }
                else
                {
                        printf("cal success/n");
                }

        }
        else
        {
                /*关闭多于的文件描述符*/
                close(pipe_fdr[0]);
                close(pipe_fdw[1]);

                while (1)
                {
                        FILE* in = fdopen(pipe_fdr[1], "w");
                        FILE* out = fdopen(pipe_fdw[0], "r");

                        char line[4096];
                        fgets(line, 4096, stdin);
                        fprintf(in, line);
                        /*把缓冲区的内容强行写入in文件*/
                        fflush(in);

                        /*读取cal的计算结果*/
                        fgets(line, 4096, out);
                        printf(line);
                }
                /*等待子进程结束*/
                wait(NULL);
        }

        return 0;
}