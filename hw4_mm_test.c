//#include "lib/hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

void *start_brk;
struct BIN {
    void *prev;	//is the same as chunk header
    void *nxt;	//is the same as chunk header
    void *min;
    int length;
}*bin[11];
struct MMAP {	//seen as a chunk header
    void *prev;
    void *nxt;
    int prev_size_and_alloc_flag;	//prev size is not used int mmap
    int curr_size_and_mmap_flag;
}*mmap_list;

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

    /*mmap*/
    mmap_list=(struct MMAP*)malloc(sizeof(struct MMAP));
    mmap_list->prev=mmap_list;
    mmap_list->nxt=mmap_list;
    mmap_list->prev_size_and_alloc_flag=0;	//don't care
    mmap_list->curr_size_and_mmap_flag=0x7fffffff;	//set size to 2^30-1 to avoid infinite loop in add_mmap_alloc_list(), set flag to mmap	//note: the fist bit is a signed bit
}
void show_mmap_list()
{
    void *temp=mmap_list->nxt;
    while(temp!=mmap_list) {
        printf("%p--------%d\n",temp,(*(int*)(temp+20))>>1);
        temp=(void*)(*(long int*)(temp+8));
    }
}
int check_in_mmap_list(void *address)
{
    void *temp=mmap_list->nxt;
    while(temp!=mmap_list) {
        if(temp==address)
            return 1;
        temp=(void*)(*(long int*)(temp+8));
    }
    return 0;
}
int main(int argc, char *argv[])
{
    start_brk=sbrk(64*1024);
    init();
    int size,result;
    char input[10],bin_or_mmap[20];
    void *address;
    int bin_num,mmap;
    int i;
    while(1) {
        if(scanf("%s",input)!=1)
            printf("failed on scanf\n");
        switch(input[0]) {
        default:
            printf("wrong input\n");
        case 'a':
            if(scanf("%d",&size)!=1)
                printf("failed on scanf\n");
            address=hw_malloc(size);
            if(address!=NULL) {
                if(size<=32*1024)
                    printf("%.12p\n",(void*)((long int)address-(long int)start_brk));
                else
                    printf("%.12p\n",address);
            } else
                printf("not enough space\n");
            break;
        case 'f':
            if(scanf("%p",&address)!=1)
                printf("failed on scanf\n");
            mmap=check_in_mmap_list(address-24);	//to see it is a mmap(actual address) or a heap(relative address)
            if(mmap)
                address=(void*)((long int)address-24);		//transfer from data address to header address
            else {	//it may be a heap addreess or a wrong addreess
                if(address>=0 && address<=(void*)(sbrk(0)-start_brk))
                    address=(void*)((long int)address+(long int)start_brk-24);		//transfer from relative data part to actual header address
                else {
                    printf("not a usuable address, try again\n");
                    continue;
                }
            }
            result=hw_free(address);
            if(result==1)
                printf("success\n");
            else
                printf("fail\n");
            break;
        case 'p':
            if(scanf("%s",bin_or_mmap)!=1)
                printf("1 failed on scanf\n");
            if(bin_or_mmap[0]=='b') {
                if(bin_or_mmap[5]=='0')	//bin[10]
                    bin_num=10;
                else	//bin[0~9]
                    bin_num=bin_or_mmap[4]-'0';
                address=(void*)*(long int*)((void*)bin[bin_num]+8);
                for(i=0; i<bin[bin_num]->length; ++i) {
                    if((void*)(address-start_brk)==0)	//special print for address 0
                        printf("0x000000000000--------%d\n",(int)pow(2,bin_num+5));
                    else
                        printf("%.12p--------%d\n",(void*)(address-start_brk),(int)pow(2,bin_num+5));
                    address=(void*)(*(long int*)(address+8));
                }
            } else if(bin_or_mmap[0]=='m') {
                show_mmap_list();
            } else
                printf("wrong input\n");
            break;
        }
    }
    return 0;
}
