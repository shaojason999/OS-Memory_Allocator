#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>

struct BIN {
    void *prev;
    void *nxt;
    void *min;
    int length;
};
struct MMAP {   //seen as a chunk header
    void *prev;
    void *nxt;
    int prev_size_and_alloc_flag;   //prev size is not used int mmap
    int curr_size_and_mmap_flag;
};
extern struct BIN *bin[11];
extern struct MMAP *mmap_list;
extern void *start_brk;

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
    update_min(i);
}
void *select_bin_chunk(int power)
{
//    show(10);
//    printf("\n");
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
//            printf("bin[%d] %p\n",i,(void*)(split_chunk-start_brk));
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
void add_mmap_alloc_list(int bytes,void *allocated_chunk)
{
    void *temp=mmap_list->nxt;
    while(bytes>=(*(int*)(temp+20)>>1)) {	//to avoid infinite loop, set the size of mmap_list to 2^30-1 in init()
        temp=(void*)*(long int*)(temp+8);
    }
    /*chunk pointer*/
    *(long int*)allocated_chunk=*(long int*)temp;
    *(long int*)(allocated_chunk+8)=(long int)temp;
    *(long int*)((*(long int*)temp)+8)=(long int)allocated_chunk;
    *(long int*)temp=(long int)allocated_chunk;
    /*chunk info*/
    *(int*)(allocated_chunk+16)=*(int*)((*(long int*)allocated_chunk)+20);	//prev size, set allocated flag
    *(int*)(allocated_chunk+20)=(bytes<<1)+1;	//curr size, set mmap flag
}
void delete_mmap_alloc_list(void *address)
{
    int length=(*(int*)(address+20))>>1;	//saved before clear the allocate flag
    /*pointer*/
    *(long int*)((*(long int*)address+8))=*(long int*)(address+8);
    *(long int*)*(long int*)(address+8)=*(long int*)address;
    /*info (this part is unnecessary, because we will call munmap)*/
    *(int*)(address+16)=0;	//clear allocate flag
    *(int*)(address+20)=0;	//clear mmap flag

    if(munmap(address,length)==-1)
        printf("failed on munmap\n");
}
void *hw_malloc(size_t bytes)
{
    void *allocated_chunk;
    bytes+=24;	//header+data
    if(bytes<=32*1024) {
        int power=(int)ceil(log(bytes)/log(2));
        allocated_chunk=select_bin_chunk(power);
        return (void*)((long int)allocated_chunk+24);	//return the address of data part
    } else {
        allocated_chunk=mmap(NULL,bytes,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,0,0);
        if(allocated_chunk==(void*)-1)
            printf("failed on mmap\n");
        else
            add_mmap_alloc_list(bytes,allocated_chunk);
        return (void*)((long int)allocated_chunk+24);	//return the address of data part
    }
}
int hw_free(void *mem)
{
    void *prev_chunk,*next_chunk;
    int prev_size,curr_size;
    int bin_num;
    int prev_test=1,next_test=1;
    int continue_merge=1;
    if(*(int*)(mem+16)&1) {	//check is allocated
        if((*(int*)(mem+20)&1)==0) {	//check is not mmap
            add_bin_chunk(log(*(int*)(mem+20)>>1)/log(2)-5,mem);	//add mem to bin in the beginning
//		if((*(int*)(mem+20)>>1)==32*1024)	//don't merge 2^15
//			return 1;
            /*merge (may not merge)*/
            while(continue_merge) {
                prev_size=*(int*)(mem+16)>>1;
                curr_size=*(int*)(mem+20)>>1;
                if(curr_size==32*1024)	//don't merge 2^15
                    break;
                if(prev_size!=0)
                    prev_chunk=(void*)(mem-prev_size);
                else
                    prev_test=0;
                if(mem+curr_size<sbrk(0))
                    next_chunk=(void*)(mem+curr_size);
                else
                    next_test=0;
                bin_num=log(curr_size)/log(2)-5;
                /*merge with prev if prev is free and prev has the same size as curr_size(size of mem)*/
                if(prev_test && (*(int*)(prev_chunk+16)&1)==0 && (*(int*)(prev_chunk+20))>>1==curr_size) {
                    delete_bin_chunk(bin_num,prev_chunk);
                    delete_bin_chunk(bin_num,mem);
                    add_bin_chunk(bin_num+1,prev_chunk);
                    /*update the prev size of next_chunk*/
                    *(int*)(next_chunk+16)&=1;
                    *(int*)(next_chunk+16)|=curr_size<<2;	//equal to (prev+cur)<<1
                    /*continute merge and the new mem address is prev_chunk*/
                    continue_merge=1;
                    mem=prev_chunk;
                }
                /*merge with next if next is free and next has the same size as curr_size(size of mem)*/
                else if(next_test && (*(int*)(next_chunk+16)&1)==0 && (*(int*)(next_chunk+20))>>1==curr_size) {
                    delete_bin_chunk(bin_num,next_chunk);
                    delete_bin_chunk(bin_num,mem);
                    add_bin_chunk(bin_num+1,mem);
                    /*update the prev size of next_next_chunk*/
                    *(int*)(next_chunk+curr_size+16)&=1;	//curr_size=next_size, so next_chunk+curr_size=next_next_chunk
                    *(int*)(next_chunk+curr_size+16)|=curr_size<<2;
                    /*continute merge and the new mem address is curr_chunk(not changed)*/
                    continue_merge=1;
                    mem=mem;
                } else	//can't merge
                    continue_merge=0;
            }
            return 1;
        } else {	//is mmap
            delete_mmap_alloc_list(mem);
            return 1;
        }
    } else
        return 0;
}

void *get_start_sbrk(void)
{
    return start_brk;
}
