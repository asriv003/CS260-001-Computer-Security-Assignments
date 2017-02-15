#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>


int a_strcmp(char * s1, char * s2)
{
    for ( ; *s1 == *s2; s1++, s2++)
	if (*s1 == '\0')
	    return 0;
    return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : +1);

}

int main(int argc, char *argv[]) {
    char a_buf[8] = {0};
    char b_buf[8] = {0};
    char c;
    int n = 0;

    printf("Please input 6 characters\n");
    read(0, a_buf, 6);
    while ( (c = getchar()) != '\n' && c != EOF ) ;

    
    memcpy(b_buf, a_buf, 3);
    
    /*Coverage*/
    n = a_strcmp("cs.", b_buf);
    if(n == 0) {
	
        /*Coverage*/        
	n = a_strcmp("ucr", a_buf+3);
        if(n==0){
            printf("You got the crash\n");
            raise(SIGSEGV);
        }        
    }
    else
      printf("Do not match!\n");
    return 0;
}