#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


void gen_date_header(char* dest)
{
    time_t  now;
    char*   datestr;

    time(&now);
    datestr = ctime(&now);

    sprintf(dest, "Date: %s", datestr);
    dest[strlen(dest) - 1] = '\0'; // drop the trailing newline generated by ctime()

    return;
}

#define _FILE_OFFSET_BITS 64

off_t fsize(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}
    
