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

extern void* hw_malloc(size_t);
extern int hw_free(void*);
extern void* hw_get_start_brk();

void init()
{
    int i;
    for(i=0; i<11; ++i) {
        bin[i]=(struct BIN*)malloc(sizeof(struct BIN));
        bin[i]->prev=bin[i];
        bin[i]->nxt=bin[i];
        bin[i]->min=NULL;
        bin[i]->length=0;
    }
    /*pointer*/
    *(long int*)start_brk=(long int)bin[10];	//prev chunk
    *(long int*)(start_brk+8)=(long int)start_brk+32*1024;	//next chunk;
    *(long int*)(start_brk+32*1024)=(long int)start_brk;
    *(long int*)(start_brk+32*1024+8)=(long int)bin[10];
    bin[10]->nxt=start_brk;
    bin[10]->prev=start_brk+32*1024;
    bin[10]->min=start_brk;
    /*info*/
    *(int*)(start_brk+16)=0;	//prev size=0, clear allocate flag
    *(int*)(start_brk+20)=(32*1024<<1)+0;	//31 bits for curr size(<<1) + 1 bit for mmap flag(0 for not mmap)
    *(int*)(start_brk+32*1024+16)=(32*1024<<1)+0;	//31 bits for prev size(<<1) + 1 bit for allocated flag(0 for free)
    *(int*)(start_brk+32*1024+20)=(32*1024<<1)+0;	//31 bits for curr size(<<1) + 1 bit for mmap flag(0 for not mmap)
    bin[10]->length=2;
}

int main(int argc, char *argv[])
{
    start_brk=sbrk(64*1024);
    init();
    printf("start_brk: %p\n",start_brk);
    int size,result;
    char input[10],bin_or_mmap[20],free[30];
    void *address;
    int bin_num;
    int i;
    while(1) {
        scanf("%s",input);
        switch(input[0]) {
        case 'a':
            scanf("%d",&size);
            address=hw_malloc(size);
            if(address!=NULL)
                printf("%.12p\n",(void*)((long int)address-(long int)start_brk));
            else
                printf("not enough space\n");
            break;
        case 'f':
            scanf("%p",&address);
            address=(void*)((long int)address+(long int)start_brk-24);
            if((*(int*)(address+20)&1)==0) {	//not mmap
                printf("123\n");
                result=hw_free(address);
                if(result==1)
                    printf("success\n");
                else
                    printf("fail\n");
            }
            break;
        case 'p':
            scanf("%s",bin_or_mmap);
            if(bin_or_mmap[0]=='b') {
                if(bin_or_mmap[5]=='0')	//bin[10]
                    bin_num=10;
                else	//bin[0~9]
                    bin_num=bin_or_mmap[4]-'0';
                address=(void*)*(long int*)((void*)bin[bin_num]+8);
                for(i=0; i<bin[bin_num]->length; ++i) {
                    printf("%p--------%d\n",(void*)(address-start_brk),(int)pow(2,bin_num+5));
                    address=(void*)(*(long int*)(address+8));
                }
            }
            break;
        }

    }
    return 0;
}
