
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#define XL_DEBUG(level, fmt, arg...)  do { xl_debug_print(level, __FILE__, __LINE__, fmt, ##arg); } while (0)


static FILE *log_fp = NULL;


char *g_level_name[] = {
	"off",					// EN_DEBUG_OFF
	"info",				// EN_DEBUG_INFO
	"debug",                        //EN_DEBUG_DBG
	"warn",				// EN_DEBUG_WARN
	"error",					// EN_DEBUG_ERROR
	"all",					// EN_DEBUG_MAX
	"maxdebug",				// EN_DEBUG_MAX
	0
};

void xl_debug_print(int level, char *file_name, int line_no, char *fmt, ...)
{
    va_list ap;
    
    /*1 参数检查*/
    
    /*2 将打印信息重定向到文件描述符*/
    if (!log_fp)
        log_fp = stdout;

    /*3  打印时间标签 */
    {
        time_t t;
        struct tm *tm;
        t = time(NULL);
        tm = localtime(&t);
        fprintf(log_fp, "[%04d/%02d/%02d %02d:%02d:%02d] ",
            tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    }


  //  if ((EN_DEBUG_OFF  != level)&&level <= g_msg_print_level)
    {
        fprintf(log_fp, "%s:%d: %s: ", file_name, line_no, g_level_name[level]);
    }  

    /*4 格式化打印信息*/
    va_start(ap, fmt);
    if (vfprintf(log_fp, fmt, ap) == -1)
    {
        va_end(ap);
        return;
    }
    va_end(ap);
    
    fflush(log_fp);
    return ;
}

void main(int argc , char *argv[])
{
	int i ;
	for(i=0;i<argc;i++)
	{
		XL_DEBUG(1,"get argc num : %d ,argv[%d]: %s \n",argc,i,argv[i]);
	}
	return 0;
}

