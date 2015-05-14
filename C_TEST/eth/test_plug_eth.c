#include <sys/socket.h>

#include <sys/ioctl.h>
#include <linux/if.h>
#include <string.h>
#include <stdio.h>

int net_detect(char* net_name)
{
    int skfd = 0;
    struct ifreq ifr;
    struct sockaddr_in *pAddr = NULL;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        printf("%s:%d Open socket error!\n", __FILE__, __LINE__);
        return -1;
    }
    strcpy(ifr.ifr_name, net_name);
    if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 )
    {
        printf("%s:%d IOCTL error!\n", __FILE__, __LINE__);
        printf("Maybe ethernet inferface %s is not valid!", ifr.ifr_name);
        close(skfd);
        return -1;
    }
    if(ifr.ifr_flags & IFF_RUNNING)
    {
        printf("%s is running :)\n", ifr.ifr_name);
    }
    else
    {
        printf("%s is not running :(\n", ifr.ifr_name);
    }
    if(ioctl(skfd,SIOCGIFADDR,&ifr)<0)
    {
        printf("SIOCGIFADDR IOCTL error!\n");
        printf("plug out !\n");
        close(skfd);
        return -1;
    }
    pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);
//    printf("ip addr :[%s]\n", inet_ntoa(pAddr->sin_addr));
    if(ioctl(skfd,SIOCGIFHWADDR,&ifr)<0)
    {
        printf("SIOCGIFHWADDR IOCTL error!\n");
        close(skfd);
        return -1;
    }
    printf("mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
        (unsigned char)ifr.ifr_hwaddr.sa_data[0],
        (unsigned char)ifr.ifr_hwaddr.sa_data[1],
        (unsigned char)ifr.ifr_hwaddr.sa_data[2],
        (unsigned char)ifr.ifr_hwaddr.sa_data[3],
        (unsigned char)ifr.ifr_hwaddr.sa_data[4],
        (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
    close(skfd);
    return 0;
}

void main()
{
    while (1)
    {
        net_detect("vlan2");
        sleep(1);
    }
}


