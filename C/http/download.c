#include "xl_common.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define HTTP_RECV_BUF_SIZE	(1024 * 10)

#define HTTP_REQUEST "GET %s HTTP/1.1\r\nHost:%s\r\n\
User-Agent: http\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n\
Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3\r\n\
Accept-Encoding: gzip,deflate\r\n\
Connection: keep-alive\r\n\r\n"

#define HTTP_POST_REQUEST "POST %s HTTP/1.1\r\nHost:%s\r\n\
User-Agent: http\r\n\
Accept: */*\r\n\
Content-Type:application/json\r\n\
Connection: keep-alive\r\n\
Content-Length: %d\r\n\r\n"

#define HTTP_DOWNLOAD_RECV_TIMEOUT	(30)
#define HTTP_DOWNLOAD_FILE_PATH_LEN (512)

#define XL_DEBUG(level,fmt,arg...)   printf(fmt,##arg)
static uint32_t http_gethostbyname(const char* hostname);
static int http_tcp_connect(uint32_t host_ip, short port);
static int http_tcp_send(int fd, const char* data, uint32_t to_send);
static int http_tcp_recv(int fd, char* buf, uint32_t buf_len, void *);
static int http_parse_url(char* url, char* domain, int domain_len, uint32_t* p_host_ip, short* p_host_port, int* p_offset);
static int http_parse_http_retcode(const char* recvbuf);
static int http_parse_http_content_length(const char* recvbuf);

FILE* http_create_download_file(const char* url, char* file_path);

int   timeval_subtract(struct   timeval*   result,   struct   timeval*   x,   struct   timeval*   y);
int time_diff(struct timeval start,struct timeval stop)   
{   
    struct   timeval   diff;   
    timeval_subtract(&diff,&start,&stop);   
    printf("总计用时:%d 微秒\n",(int)(diff.tv_sec*1000000+diff.tv_usec));   
    return diff.tv_usec;
}                     
    
  /**   
      *   计算两个时间的间隔，得到时间差   
      *   @param   struct   timeval*   resule   返回计算出来的时间   
      *   @param   struct   timeval*   x             需要计算的前一个时间   
      *   @param   struct   timeval*   y             需要计算的后一个时间   
      *   return   -1   failure   ,0   success   
  **/   
  int   timeval_subtract(struct   timeval*   result,   struct   timeval*   x,   struct   timeval*   y)   
  {   
    
        if ( x->tv_sec > y->tv_sec )   
            return   -1;   
    
        if ( (x->tv_sec == y->tv_sec) && (x->tv_usec>y->tv_usec))   
            return   -1;
    
        result->tv_sec = (y->tv_sec) - (x->tv_sec);   
        result->tv_usec   =   (y->tv_usec) - (x->tv_usec);   
    

        if (result->tv_usec<0)   
        { 
            result->tv_sec--;   
            result->tv_usec+=1000000;   
        }   
    
        printf("start: sec %d ,us %d \n",(int)x->tv_sec,(int)x->tv_usec);
        printf("stop: sec %d ,us %d \n",(int)y->tv_sec,(int)y->tv_usec);

        return 0;   
  }   
    


uint32_t http_gethostbyname(const char* hostname)
{
	uint32_t host_ip = 0;
	struct addrinfo *answer, hint, *curr;

	/* 一个域名对应32个IP地址足够 */
	uint32_t ip_list[32] = {0};

	bzero(&hint, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;

	/* 获取服务器域名对应的IP地址 */
	int ret = getaddrinfo(hostname, NULL, &hint, &answer);
	if (ret != 0) 
	{
		XL_DEBUG(EN_PRINT_ERROR,"getaddrinfo: %s\n",gai_strerror(ret));
		return 0;
	}

	char ipstr[16] = {0};
	int i;
	/* 解析出所有的IP地址 */
	for (curr = answer, i = 0; curr != NULL; curr = curr->ai_next, i++) 
	{
		ip_list[i] = ((struct sockaddr_in *)(curr->ai_addr))->sin_addr.s_addr;
		inet_ntop(AF_INET,&(((struct sockaddr_in *)(curr->ai_addr))->sin_addr),ipstr, 16);
		XL_DEBUG(EN_PRINT_INFO, "resolve host:%s ip:%s\n", hostname, ipstr);
	}

	if (i != 0)
	{
		/* 负载均衡,随机选择解析出的多个IP地址 */
		srand(time(NULL));
		host_ip = ip_list[rand() % i];
		XL_DEBUG(EN_PRINT_INFO, "get broker server host ip:%u\n", host_ip);
	}

	freeaddrinfo(answer);
	return host_ip;
}

/***********************************************************
Function:     	http_tcp_connect
Description:	连接接口
Input:      	
Output:     	
Return:     	成功:返回socket fd 失败: -1
Others:     	
History: 
************************************************************/
int http_tcp_connect(uint32_t host_ip, short port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		XL_DEBUG(EN_PRINT_ERROR, "Create socket faild:%s\n", strerror(errno));
		return -1;
	}

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = host_ip;
	sin.sin_port = htons(port);

    struct timeval tv = {30,0};
    setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(char * )&tv,sizeof(struct timeval));

	if (connect(fd, (struct sockaddr*)&sin, sizeof(sin)))
	{
		XL_DEBUG(EN_PRINT_ERROR, "Connection to util.peiluyou.com failed:%s\n", strerror(errno));
		close(fd);
		return -1;
	}
    else
    {
        XL_DEBUG(EN_PRINT_INFO,"Connect to server successfully\n");
    }

	/* set recv timeout */
	int succ = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
	if (succ != 0)
	{
		XL_DEBUG(EN_PRINT_ERROR, "set local rpc connection timeout failed:%s\n", strerror(errno));
		close(fd);
		return -1;
	}

	return fd;
}

/***********************************************************
Function:     	http_tcp_send
Description:	发送指定大小的数据
Input:      	
				fd: 已经建立连接的socket fd
				data: 待发送数据缓冲区
				to_send: 指定的发送大小
Output:     	
Return:     	成功:0 失败: -1
Others:     	阻塞接口
History: 
************************************************************/
int http_tcp_send(int fd, const char* data, uint32_t to_send)
{
	if (NULL == data)
	{
		XL_DEBUG(EN_PRINT_ERROR, "bad send param\n");
		return -1;
	}

	ssize_t ret, has_send, remaining;
	has_send = 0;
	remaining = to_send;
	while (remaining)
	{
		ret = send(fd, data + has_send, remaining, 0);
		if (ret <= 0)
		{
			XL_DEBUG(EN_PRINT_ERROR, "send failed:%s\n", strerror(errno));
			return -1;
		}
		has_send += ret;
		remaining -= ret;
	}

	return 0;
}

/***********************************************************
Function:     	http_tcp_recv
Description:	接收数据
Input:      	
				fd: 已经建立连接的socket fd
				buf: 接收缓冲区
				buf_len: 接收缓冲区大小
Output:     	
Return:     	成功: 接收的数据大小 失败: -1
Others:     	阻塞接口, 超过缓冲区大小的数据将会被丢弃
History: 
************************************************************/
int http_tcp_recv(int fd, char* buf, uint32_t buf_len, void * ptr)
{
	if (NULL == buf)
	{
		XL_DEBUG(EN_PRINT_ERROR, "bad recv param\n");
		return -1;
	}

	ssize_t ret , has_recv = 0;
	ssize_t remaining = buf_len;
	while (remaining)
	{
		ret = recv(fd, buf + has_recv, remaining, 0);
		if (0 == ret)
		{
			break;
		}
		else if (ret < 0)
		{
			XL_DEBUG(EN_PRINT_ERROR, "recv failed:%s\n", strerror(errno));
			return -1;
		}
		has_recv += ret;
		remaining -= ret;
        
        char * pos = NULL;
        pos = strstr(buf,"\r\n\r\n");
        if(NULL == pos)
        {
            continue;
        }
        else
        {
            break;
        }
	}

	return has_recv;
}

int http_parse_url(char* url, char* domain, int domain_len, uint32_t* p_host_ip, short* p_host_port, int* p_offset)
{
	/* http://xxxx:xx/xxxx */
	if (memcmp(url, "http://", 7))
	{
		XL_DEBUG(EN_PRINT_ERROR, "invalid url:%s\n", url);
		return -1;
	}

	/* skip http:// */
	char* pos1 = url + 7;
	char* pos2 = strchr(pos1, '/');
	if (NULL == pos2)
	{
		XL_DEBUG(EN_PRINT_ERROR, "invalid url:%s\n", url);
		return -1;
	}

	*p_offset = pos2 - url;

	char buf[64] = {0};
	memcpy(buf, pos1, pos2 - pos1);
    XL_DEBUG(EN_PRINT_INFO,"Get the domain name : [%s]\n",buf);
	pos2 = strchr(buf, ':');
	if (NULL == pos2)
	{
		*p_host_port = 80; 
	}
	else
	{
		*p_host_port = atoi(pos2 + 1);
		*pos2 = 0;
	}

	snprintf(domain, domain_len, "%s", buf);

	*p_host_ip = http_gethostbyname(buf);
	if (0 == *p_host_ip)
	{
		XL_DEBUG(EN_PRINT_ERROR, "Invalid host name:[%s]\n", buf);
		return -2;	
	}		
    XL_DEBUG(EN_PRINT_INFO,"p_host_ip is [0x%x] \n", *p_host_ip);

	return 0;
}

int http_parse_http_retcode(const char* recvbuf)
{
	char* pos = strstr(recvbuf, "\r\n");
	if (NULL == pos)
	{
		XL_DEBUG(EN_PRINT_ERROR, "invalid http header.\n");
		return -1;
	}

	char line[128] = {0};
	memcpy(line, recvbuf, pos - recvbuf);
	if(strstr(line, "200"))
	{
		return 200;
	}
	else if (strstr(line, "404"))
	{
		return 404;
	}
	else if (strstr(line, "302") || strstr(line, "301"))
	{
		return 302;
	}
    else if(strstr(line, "201"))
	{
		return 201;
	}

	XL_DEBUG(EN_PRINT_ERROR, "unknown http ret code:%s\n", line);
	return -1;
}

int http_parse_http_content_length(const char* recvbuf)
{
	char* pos = strstr(recvbuf, "Content-Length:");
	if (NULL == pos)
	{
		XL_DEBUG(EN_PRINT_ERROR, "invalid http header.\n");
		return -1;
	}

	char* pos2 = strstr(pos, "\r\n");
	if (NULL == pos2)
	{
		XL_DEBUG(EN_PRINT_ERROR, "invalid http header.\n");
		return -1;
	}

	/* skip "Content-Length: " */
	pos += 16;
	char len_buf[20] = {0};
	memcpy(len_buf, pos, pos2 - pos);
	return atoi(len_buf);
}

FILE* http_create_download_file(const char* url, char* file_path)
{
	/* parse upd file name */
	char* pos = strrchr(url, '/');
	if (NULL == pos)
	{
		XL_DEBUG(EN_PRINT_ERROR, "invalid url!\n");
		return NULL;
	}

	snprintf(file_path, HTTP_DOWNLOAD_FILE_PATH_LEN, "./%s", pos + 1);
	XL_DEBUG(EN_PRINT_INFO, "download file path:[%s]\n", file_path);

	FILE* fp = fopen(file_path, "w+");
	if (NULL == fp)
	{
		XL_DEBUG(EN_PRINT_ERROR, "open %s failed:%s\n", file_path, strerror(errno));
		return NULL;
	}

	return fp;
}

	

uint32_t http_download_file(const char* url,const char * extern_ip,const char * extern_port)
{
	int ret;
	uint32_t host_ip;
	short host_port;
	int  offset;
	
	/* parse url, get host ip, host port, query string offset */
	char domain[64] = {0};
	ret = http_parse_url(url, domain, sizeof(domain), &host_ip, &host_port, &offset);
	if (-1 == ret)
	{
		XL_DEBUG(EN_PRINT_ERROR, "parse url failed!\n");
		return 1;
	}
	else if (-2 == ret)
	{
		XL_DEBUG(EN_PRINT_ERROR, "resolve host name failed!\n");
		return 2;
	}
	char* query_string = url + offset;
	XL_DEBUG(EN_PRINT_INFO, "domain:%s ip:%u port:%u query_string:%s\n", domain, host_ip, host_port, query_string);

	/* build http request string */
	int hrlen = 2048;
	char* http_request = (char*) malloc(hrlen);
    char upnp_string [256] = {0}; 
    
    sprintf(upnp_string,"{\"host\":\"%s\",\"port\":%d}",extern_ip,atoi(extern_port));
	
    memset(http_request, 0, hrlen);
	snprintf(http_request, hrlen, HTTP_POST_REQUEST, query_string, domain,strlen(upnp_string));
    
    strcat(http_request,upnp_string);
	
    XL_DEBUG(EN_PRINT_INFO, "\nhttp_request:\n%s\n", http_request);
	/* connect the http server */
	int fd = http_tcp_connect(host_ip, host_port);
	if (-1 == fd)
	{
		XL_DEBUG(EN_PRINT_ERROR, "connect failed!\n");
		close(fd);
        free(http_request);
		return 3;
	}
    else
    {
        XL_DEBUG(EN_PRINT_INFO,"TCP connect to server successfully\n");
}
    
    /* send http request */
	ret = http_tcp_send(fd, http_request, strlen(http_request));
	if (ret != 0)
	{
		close(fd);
        free(http_request);
		return 4;
	}
    else
    {
        XL_DEBUG(EN_PRINT_INFO,"send tcp http_request send successfully\n");
    }

	/* recv http response header */
	char catch[1024]={'\0'};
    char * recv_buf = catch;
	if (NULL == recv_buf)
	{
		XL_DEBUG(EN_PRINT_FATAL, "malloc %d bytes faild!\n", HTTP_RECV_BUF_SIZE);
		close(fd);
        free(http_request);
		return 5;
	}

	int first_recv_len = http_tcp_recv(fd, recv_buf, HTTP_RECV_BUF_SIZE, NULL);
	if (-1 == first_recv_len)
	{
		close(fd);
		free(recv_buf);
		if (EAGAIN == errno || EWOULDBLOCK == errno)
		{
			XL_DEBUG(EN_PRINT_ERROR, ".|.|.|.|.|.|.|.|.|.|.|.|.|.|.|.| download timeout\n");
            free(http_request);
			return 7;
		}
        free(http_request);
		return 6; 
	}

	XL_DEBUG(EN_PRINT_INFO, "recv %d bytes:\n%s\n", (int)strlen(recv_buf), recv_buf);

	/* parse http response code */
	ret = http_parse_http_retcode(recv_buf);
	XL_DEBUG(EN_PRINT_INFO, "\nhttp ret code:%d\n", ret);

	uint32_t err_code = 0;
	if (201 == ret)
	{
		/* parse http response content lenght */
		int file_size  = http_parse_http_content_length(recv_buf);
		if (-1 == file_size)
		{
			close(fd);
			free(recv_buf);
            free(http_request);
			return 9;
		}

		fprintf(stderr,"[Content-Length:%d]\n", file_size);	

		/* write the first recv file content */
		char* header_end = strstr(recv_buf, "\r\n\r\n");
		if (NULL == header_end)
		{
			XL_DEBUG(EN_PRINT_ERROR, "invalid http response\n");
			close(fd);
			free(recv_buf);
            free(http_request);
			return 10;
		}
		
		fprintf(stdout,"%s\n",header_end + 4);
	}
	else if (404 == ret)
	{
		XL_DEBUG(EN_PRINT_ERROR, "404 File Not Found\n");
		err_code = -404;
	}

	close(fd);
    free(http_request);
	return err_code;
}

int main(int args,char * argv[])
{
//    struct   timeval   start,stop;   
//    gettimeofday(&start,0);
    if (args < 4)
    {
        printf ("%s: url ,extern_ip,exptern_port\n",argv[0]);
        return -1;
    }
    
    int i = 3;
    while (i)
    { 
        http_download_file(argv[1],argv[2],argv[3]); 
        i--;
    }
   //    gettimeofday(&stop,0);  
//    time_diff(start,stop);

    return 0;
}

