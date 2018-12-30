#include <stdlib.h>
#include <stdio.h>
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
void delete_bin_chunk(int i, void* min)	//delete the min one
{
    /*pointer*/
    *(long int*)(*(long int*)(min+8))=*(long int*)min;
    *(long int*)((*(long int*)min)+8)=*(long int*)(min+8);

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
//    *(int*)(p+16)&=0xfffffffe;	//free
//    *(int*)(p+20)=(int)pow(2,i+5)<<1;	//curren size
    *(int*)(p+16)=0;	//free
    *(int*)(p+20)=0;	//not mmap
    /*update*/
    ++(bin[i]->length);
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
        --i;
        /*split if neede*/
        split_chunk=allocated_chunk+(int)pow(2,i+5);
        while(i>=power-5) {
            printf("bin[%d] %p\n",i,(void*)(split_chunk-start_brk));
            add_bin_chunk(i,split_chunk);
            --i;
            split_chunk=split_chunk-(int)pow(2,i+5);
        }
        *(int*)(allocated_chunk+16)=1;	//allocated
        *(int*)(allocated_chunk+20)=0;	//not mmap
        /*allocated*/
    }
    return allocated_chunk;

}
void *hw_malloc(size_t bytes)
{
    void *allocated_chunk;
    int power=(int)ceil(log(bytes+24)/log(2));
    allocated_chunk=select_bin_chunk(power);
//	printf("%d\n",power);
    return allocated_chunk;
}

int hw_free(void *mem)
{
    return 0;
}

void *get_start_sbrk(void)
{
    return NULL;
}
