#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "message.h"

#define MAC_OS_X
#ifdef MAC_OS_X
#include <malloc/malloc.h> /* malloc_size() on Mac */
int extend_string_allocates_unitsize_blocks()
{
    char    *dst;
    char    *src;
    int     dstlen;
    int     unitsize, before, after;


    unitsize = malloc_good_size(1);
    dst = malloc(unitsize);
    memset(dst, 'a', unitsize);
    dst[unitsize - 1] = '\0';
    dstlen = unitsize - 1;

    src = malloc(unitsize * 2);
    memset(src, 'b', unitsize * 2);
    src[(unitsize * 2) - 1] = '\0';

    fork_to_test(
        before = malloc_size(dst);
        extend_string(&dst, &dstlen, src, strlen(src), unitsize);
        after = malloc_size(dst);
//        printf("\nunitsize: %d\tbefore: %d\tafter: %d\n", unitsize, before, after);
//        puts(dst);
        if(after <= before)
            exit(1);
        if((after % unitsize) != 0)
            exit(2);
        exit(0);
    )
}
#endif

int extend_string_allocates_memory_given_null_ptr()
{
    char    *dst;
    char    *src;
    int     dstlen;

    dst = NULL;
    dstlen = 0;
    src = "Dead beef";

    fork_to_test(
        extend_string(&dst, &dstlen, src, strlen(src), 256);
        if(dst == NULL)
            exit(1);
        exit(0);
    )
}

int main()
{
    int count, fail;
//    FILE    *success;

    count = 0;
    fail = 0;

    returns_zero(count, fail, "extend_string() allocates memory, given a NULL ptr", extend_string_allocates_memory_given_null_ptr);
    returns_zero(count, fail, "extend_string() allocates memory in the specified unit size", extend_string_allocates_unitsize_blocks);

    printf("\n[message-test] Total tests passed/run: %d/%d\n", count - fail, count);
    puts("------------------------------------------------------------\n");

/*
    if(!fail)
    {
        success = fopen("message.pass", "w");
        fclose(success);
    }
*/

    return fail;
}
