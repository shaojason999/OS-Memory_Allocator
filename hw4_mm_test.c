//#include "lib/hw_malloc.h"
#include <stdio.h>
#include <unistd.h>

void *start_brk;

int main(int argc, char *argv[])
{
    start_brk=sbrk(64*1024);
    int *temp=(int*)start_brk;
    *(int*)start_brk=10;
    printf("%d\n",*temp);
    int *temp1=(int*)start_brk;
    printf("%d\n",*temp1);
    temp++;
    *temp=2;
    printf("%d\n",*temp);
    printf("%p\n",start_brk);
    start_brk+=4;
    printf("%p\n",start_brk);
    int *temp2=(int*)start_brk;
    printf("%d\n",*temp2);
    printf("%d\n",sizeof(int));

    return 0;
}
