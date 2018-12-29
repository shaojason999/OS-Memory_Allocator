//#include "lib/hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


void *start_brk;
struct BIN {
    void *prev;	//is the same as chunk header
    void *nxt;	//is the same as chunk header
    void *min;
    int length;
}*bin[11];

void init()
{
    int i;
    for(i=0; i<11; ++i) {
        bin[i]=(struct BIN*)malloc(sizeof(struct BIN));
        printf("%p\n",bin[i]);
        bin[i]->prev=bin[i];
        bin[i]->nxt=bin[i];
        bin[i]->min=NULL;
        bin[i]->length=0;
    }
    /*pointer*/
    *(long int*)start_brk=bin[10];	//prev chunk
    *(long int*)(start_brk+8)=start_brk+32*1024;	//next chunk;
    *(long int*)(start_brk+32*1024)=start_brk;
    *(long int*)(start_brk+32*1024+8)=bin[10];
    bin[10]->nxt=start_brk;
    bin[10]->prev=start_brk+32*1024;
    bin[10]->min=start_brk;
    /*info*/
    *(int*)(start_brk+16)=32*1024<<1+0;	//31 bits for prev size(<<1) + 1 bit for allocated flag(0 for free)
    *(int*)(start_brk+20)=32*1024<<1+0;	//31 bits for curr size(<<1) + 1 bit for mmap flag(0 for not mmap)
    *(int*)(start_brk+32*1024+16)=32*1024<<1+0;	//31 bits for prev size(<<1) + 1 bit for allocated flag(0 for free)
    *(int*)(start_brk+32*1024+20)=32*1024<<1+0;	//31 bits for curr size(<<1) + 1 bit for mmap flag(0 for not mmap)
    bin[10]->length=2;


    printf("%p %p 0x%lx\n",start_brk,start_brk+32*1024,*(long int*)start_brk);
    printf("%p\n",sbrk(0));
//	printf("%p %p\n",start_brk,*(long int*)start_brk);
    printf("0x%lx %p 0x%lx\n",*(long int*)((void*)(*(long int*)(bin[10]->min))),(void*)(*(long int*)(bin[10]->min)),*(long int*)(bin[10]->min));
    printf("0x%lx %p 0x%lx\n",*(long int*)((*(long int*)(bin[10]->min))),(void*)(*(long int*)(bin[10]->min)),*(long int*)(bin[10]->min));

}

int main(int argc, char *argv[])
{
    start_brk=sbrk(64*1024);
    init(bin);
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
