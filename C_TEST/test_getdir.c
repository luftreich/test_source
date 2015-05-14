#include <stdio.h> 
//#include <dir.h> 
#define MAXPATH (256)
int main(void) 
{ 
	char buffer[MAXPATH]; 
	getcwd(buffer, MAXPATH); 
	printf("The current directory is: %s\n", buffer); 
	return 0; 
} 
