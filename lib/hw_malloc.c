#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

struct BIN {
    void *prev;
    void *nxt;
    void *min;
    int length;
};

extern struct BIN *bin[11];
extern void* start_brk;

/*test use*/
void show(int i)
{
    int j;
    void *temp=bin[i];
    printf("%p\n",temp);
    for(j=0; j<=bin[i]->length; ++j) {
        temp=(void*)(*(long int*)(temp+8));
        printf("%p\n",temp);
    }
}
void update_min(int i)
{
    int j;
    void *temp=bin[i];
    bin[i]->min=(void*)0xffffffffffffffff;
    for(j=0; j<bin[i]->length; ++j) {
        temp=(void*)(*(long int*)(temp+8));
        if(temp<bin[i]->min)
            bin[i]->min=temp;
    }
}
void delete_bin_chunk(int i, void* address)	//delete the min one
{
    /*pointer*/
    *(long int*)(*(long int*)(address+8))=*(long int*)address;
    *(long int*)((*(long int*)address)+8)=*(long int*)(address+8);

    /*update*/
    --(bin[i]->length);
    update_min(i);
}
void add_bin_chunk(int i, void* p)	//add to the rear of the bin
{
    /*pointer*/
    *(long int*)(p+8)=(long int)bin[i];
    *(long int*)p=*(long int*)bin[i];
    *(long int*)((*(long int*)bin[i])+8)=(long int)p;
    *(long int*)bin[i]=(long int)p;
    /*info*/
    *(int*)(p+16)&=0xfffffffe;	//not change prev size, set free
    *(int*)(p+20)=((int)pow(2,i+5)<<1)+0;	//curren size, not mmap
    /*update*/
    ++(bin[i]->length);
    printf("length: %d\n",bin[i]->length);
    update_min(i);
}
void *select_bin_chunk(int power)
{
//    show(10);
    printf("\n");
    int i;
    void *allocated_chunk,*split_chunk;
    allocated_chunk=NULL;
    /*select the best fit chunk*/
    for(i=power-5; i<11; ++i) {	//bin[0] is 2^5
        if(bin[i]->length>0) {	//the first one is the best one
            allocated_chunk=bin[i]->min;
            delete_bin_chunk(i,bin[i]->min);	//remove from the bin[i]
            break;
        }
    }
    if(allocated_chunk!=NULL) {
        /*split if needed*/
        split_chunk=allocated_chunk+(int)pow(2,i+5);
        for(--i; i>=power-5; --i) {
            split_chunk=split_chunk-(int)pow(2,i+5);
            printf("bin[%d] %p\n",i,(void*)(split_chunk-start_brk));
            add_bin_chunk(i,split_chunk);	//add to bin, set cur size, set free
            /*after split the chunk, we have to change the prev size of next chunk*/
            if((void*)((long int)split_chunk+(long int)pow(2,i+5))<sbrk(0)) {	//avoid heap overflow
                *(int*)((long int)split_chunk+(long int)pow(2,i+5)+16)&=1;	//clear prev size of next chunk, not change the allocate flag
                *(int*)((long int)split_chunk+(long int)pow(2,i+5)+16)|=((int)pow(2,i+5)<<1)+0;	//set prev size of next chunk, not change the allocate flag
            }
        }
        /*set the prev size of last split_chunk*/
        *(int*)(split_chunk+16)&=1;	//clear the prev size, not change the allocate flag
        *(int*)(split_chunk+16)|=((int)pow(2,power)<<1)+0;	//prev size is the size of allocated chunk, not change the allocate flag
        /*set the allocated chunk*/
        *(int*)(allocated_chunk+16)|=1;	//not change prev size, set allocate flag
        *(int*)(allocated_chunk+20)=((int)pow(2,power)<<1)+0;	//curr size, not mmap
    }
    return allocated_chunk;
}
void *hw_malloc(size_t bytes)
{
    void *allocated_chunk;
    int power=(int)ceil(log(bytes+24)/log(2));
    allocated_chunk=select_bin_chunk(power);
    return (void*)((long int)allocated_chunk+24);
}

int hw_free(void *mem)
{
    void *prev_chunk,*next_chunk;
    int prev_size,curr_size;
    int bin_num;
    int prev_test=1,next_test=1;
    int continue_merge=1;
    if(*(int*)(mem+16)&1) {	//mem is equal to curr_chunk
        while(continue_merge) {
            printf("234\n");
            prev_size=*(int*)(mem+16)>>1;
            curr_size=*(int*)(mem+20)>>1;
            if(prev_size!=0)
                prev_chunk=(void*)(mem-prev_size);
            else
                prev_test=0;
            if(mem+curr_size<sbrk(0))
                next_chunk=(void*)(mem+curr_size);
            else
                next_test=0;
            bin_num=log(curr_size)/log(2)-5;
//			printf("%d %d %p %p %d\n",prev_size,curr_size,prev_chunk,next_chunk,bin_num);
            /*merge with prev if prev is free and prev has the same size as curr_size(size of mem)*/
            if(prev_test && (*(int*)(prev_chunk+16)&1)==0 && (*(int*)(prev_chunk+20))>>1==curr_size) {
                printf("1 123\n");
                delete_bin_chunk(bin_num,prev_chunk);
                add_bin_chunk(bin_num+1,prev_chunk);
                /*update the prev size of next_chunk*/
                *(int*)(next_chunk+16)&=1;
                *(int*)(next_chunk+16)|=curr_size<<2;	//equal to (prev+cur)<<1
                /*continute merge and the new mem address is prev_chunk*/
                printf("123\n");
                continue_merge=1;
                mem=prev_chunk;
            }
            /*merge with next if next is free and next has the same size as curr_size(size of mem)*/
            else if(next_test && (*(int*)(next_chunk+16)&1)==0 && (*(int*)(next_chunk+20))>>1==curr_size) {
                delete_bin_chunk(bin_num,next_chunk);
                add_bin_chunk(bin_num+1,mem);
                /*update the prev size of next_next_chunk*/
                *(int*)(next_chunk+curr_size+16)&=1;	//curr_size=next_size, so next_chunk+curr_size=next_next_chunk
                *(int*)(next_chunk+curr_size+16)|=curr_size<<2;
                /*continute merge and the new mem address is curr_chunk(not changed)*/
                continue_merge=1;
                mem=mem;
            }
            /*don't merge*/
            else if(1) {
                add_bin_chunk(bin_num,mem);
                /*stop merge*/
                continue_merge=0;
            } else
                continue_merge=0;
        }
        return 1;
    } else
        return 0;
}

void *get_start_sbrk(void)
{
    return NULL;
}
