#include <stdio.h>

int main(int argc, char *argv[])
{
    char name[256] = {0};
    char data[3];
    char c;
    
    strcpy(name,argv[1]);
    
    int i=0;
    do {
    
    c = name[i];
    fprintf(stderr,"%02x", (unsigned char) c);
    i++;
    if (i>= strlen(name))
    {
        break;
    }
    } while(1);
    return 0;
}

