#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h> /* not needed when printf is removed */
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include "message.h"
#include "response.h"
#include "request.h"

int clean_segfault(client_t *c, char **splat, int splat_len)
{
    char    *foobar;
    syslog(LOG_DEBUG, "%s():...", __func__);

    foobar = (char *)0xDEADBEEF;
    *foobar = 'a';

    return 0;
}

