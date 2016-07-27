#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>

int get_host( char * name, char * host)
{
    char   *ptr, **pptr;
    struct hostent *hptr;
    char   str[32];
    ptr = name;

    if((hptr = gethostbyname(ptr)) == NULL)
    {
        printf(" gethostbyname error for host:%s\n", ptr);
        return 0;
    }

    for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
        printf(" alias:%s\n",*pptr);

    switch(hptr->h_addrtype)
    {
        case AF_INET:
        case AF_INET6:
            pptr=hptr->h_addr_list;
            sprintf(host,"%s",inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
        break;
        default:
            printf("unknown address type\n");
        break;
    }

    return 0;
}
