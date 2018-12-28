//#include "lib/hw_malloc.h"
#include <stdio.h>
#include <unistd.h>


void *start_brk;
struct BIN {
    void *prev;
    void *nxt;
} bin[11];
void init()
{
    int i;
    for(i=0; i<11; ++i) {
        bin[i].prev=NULL;
        bin[i].nxt=NULL;
    }
//	*(long int*)start_brk=0x9a82b3193823;
//	printf("%p %p\n",start_brk,*(long int*)start_brk);
    *(long int*)start_brk=start_brk+32*1024;	//prev chunk
    *(long int*)(start_brk+8)=start_brk+32*1024;	//next chunk;
    *(long int*)(start_brk+32*1024)=start_brk;
    *(long int*)(start_brk+32*1024+8)=start_brk;
    *(int*)(start_brk+16)=32*1024<<1+0;	//31 bits for size(<<1) + 1 bit for allocated flag(0 for free)
    *(int*)(start_brk+20)=32*1024<<1+0;	//31 bits for size(<<1) + 1 bit for mmap flag(0 for not mmap)
    *(int*)(start_brk+32*1024+16)=32*1024<<1+0;	//31 bits for size(<<1) + 1 bit for allocated flag(0 for free)
    *(int*)(start_brk+32*1024+20)=32*1024<<1+0;	//31 bits for size(<<1) + 1 bit for mmap flag(0 for not mmap)

    printf("%p 0x%lx\n",start_brk,*(long int*)start_brk);
//	printf("%p %p\n",start_brk,*(long int*)start_brk);

}

int main(int argc, char *argv[])
{
    start_brk=sbrk(64*1024);
    init();
    int size,result;
    char input[10],bin_or_mmap[20];
    void *address;
    while(1) {
        scanf("%s",input);
        switch(input[0]) {
        case 'a':
            scanf("%d",&size);
            address=hw_malloc(size);
            break;
        case 'f':
            scanf("%p",&address);
            //printf("%p\n",address);
            result=hw_free(address);
            if(result==1)
                printf("success\n");
            else
                printf("fail\n");
            break;
        case 'p':
            scanf("%s",bin_or_mmap);
            break;
        }

    }
    return 0;
}
