#include <stdlib.h>
#include <stdio.h>

struct BIN {
    void *prev;
    void *nxt;
};

extern struct BIN bin[11];
extern void* start_brk;

void *hw_malloc(size_t bytes)
{
    printf("%p\n",bin[0].prev);
    return NULL;
}

int hw_free(void *mem)
{
    return 0;
}

void *get_start_sbrk(void)
{
    return NULL;
}
