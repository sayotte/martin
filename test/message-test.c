#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "message.h"

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
