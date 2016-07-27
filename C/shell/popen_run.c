/*******************************************************************************************
** Name:popen.c
**      This program is used to show the usage of popen() .
*******************************************************************************************/
#include <stdio.h>
#include <string.h>
int main( void )
{
   FILE   *stream;
   char   buf[1024];

    memset( buf, '\0', sizeof(buf) );//初始化buf,以免后面写如乱码到文件中
    stream = popen( "ls -l", "r" ); //将“ls －l”命令的输出 通过管道读取（“r”参数）到FILE* stream
    fread( buf, sizeof(char), sizeof(buf), stream); //将刚刚FILE* stream的数据流读取到buf中
    pclose( stream );
    printf("%s\n",buf);
    return 0;
}
