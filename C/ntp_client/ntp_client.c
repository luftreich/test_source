#include "xl_common.h"

#define SECONDS_1900_1970 0X83AA7E80

#define NTP_SRV_PORT 123
#define NTP_PACKET_LEN 48

#define TIMEOUT 10

#define NTP_SERVER_0 "123.162.191.94" //ntp.cc.sandai.net
#define NTP_SERVER_1 "91.189.94.4"    //ntp.ubuntu.net
#define NTP_SERVER_2 "202.210.2.101" //cn.pool.ntp.org 
#define NTP_SERVER_3 "17.83.253.7"  //time.asia.apple.com

static void construct_ntp_packet(char content[])
{
	memset(content,0,NTP_PACKET_LEN);
	content[0] = 0x1b;//
}

int get_ntp_time(int sockfd,struct sockaddr_in *server_addr,struct tm * net_tm)
{
	char content[256];
	time_t timet;
	long temp;
	int addr_len = 16;
	struct timeval block_time;
	fd_set sockfd_set;

	FD_ZERO(&sockfd_set);
	FD_SET(sockfd,&sockfd_set);

	block_time.tv_sec = TIMEOUT;
	block_time.tv_usec = 0;

	construct_ntp_packet(content);

	if (sendto(sockfd,content,NTP_PACKET_LEN,0,(struct sockaddr *)server_addr,addr_len) < 0)
	{
		perror("sendto error");
		return(-1);
	}
	int ret =  select(sockfd + 1,&sockfd_set,NULL,NULL,&block_time);
	if(ret >0)
	{
		memset(content,0,sizeof(content));
		if (recvfrom(sockfd,content,sizeof(content),0,(struct sockaddr *)server_addr,(socklen_t*)&addr_len)<0)
		{
			perror("recvfrom error");
		    printf("select return %d ,line %d \n",ret,__LINE__);
			return (-1);
		}
		else
		{
			memcpy(&temp,content+40,4); // transmit_timestamp
			temp = (time_t)(ntohl(temp) - SECONDS_1900_1970);
			timet = (time_t)temp;
			memcpy(net_tm,gmtime(&timet),sizeof(struct tm));
			net_tm->tm_hour = net_tm->tm_hour + 8;
		}
	}
	else
	{
		perror("select error");
		printf("select return %d ,line %d \n",ret,__LINE__);
		return -1;
	}

	return 0;
}


int main(void)
{
	int sockfd,i;
	struct tm *net_tm;
	struct sockaddr_in addr;
	char ip[4][16]={{NTP_SERVER_0},{NTP_SERVER_1},{NTP_SERVER_2},{NTP_SERVER_3}};
	
	net_tm = (struct tm*)malloc(sizeof(struct tm));

	for (i =0;i<4;i++)
	{
		memset(&addr,0,sizeof(addr));

		addr.sin_addr.s_addr = inet_addr(ip[i]);
		addr.sin_port = htons(NTP_SRV_PORT);
		
		if((sockfd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))== -1)
		{
		    printf("create udp link failed ,ip is [%s],port is [%d],udp\n",ip[i],NTP_SRV_PORT);
			perror("socket error");
			return -1;
		}

		printf("IP [%s], udp port 123 success\n",ip[i]);
		
		int cnt =0;
		int get_time = 0;
		for (cnt = 0;cnt < 3;cnt ++)
		{
			memset(net_tm,0,sizeof(struct tm));
			if(get_ntp_time(sockfd,&addr,net_tm) == 0)
			{
				printf("ntp:[%d/%d/%d   %d:%d:%d]\n\n",
				net_tm->tm_year + 1900,
				net_tm->tm_mon +1,
				net_tm->tm_mday,
				net_tm->tm_hour,
				net_tm->tm_min,
				net_tm->tm_sec);
				get_time = 1;
				break;
			}
		}
		if (0 == get_time)
		{
			printf("Get ntp time failed !!! \n\n");
		}
		else
		{
			mktime(net_tm);
			close(sockfd);
			return 0;
		}
		close(sockfd);
	}

	return 0;
}














