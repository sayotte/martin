#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h> /* not needed when printf is removed */
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include "response.h"
#include "request.h"
#include "util.h"

extern const char* r403_template;
extern const char* r404_template;

int put_transaction(int fd, message_t *m, char **splat, int splat_len)
{
    int     i;
    syslog(LOG_DEBUG, "%s():...", __func__);

    for(i = 0; i < splat_len; i++)
    {
        printf("splat[%d]: '%s'\n", i, splat[i]);
    }
    return 0;
}

int get_all_transactions(int fd, message_t *m, char **splat, int splat_len)
{
    int     i;
    syslog(LOG_DEBUG, "%s():...", __func__);

    for(i = 0; i < splat_len; i++)
    {
        printf("splat[%d]: '%s'\n", i, splat[i]);
    }
    return 0;
}

int get_transaction(int fd, message_t *m, char **splat, int splat_len)
{
    preamble_t    h;

    syslog(LOG_DEBUG, "%s():...", __func__);

    init_response_preamble(&h);
    add_response_header(&h, "Connection: close");

    if(m->http_minor < 1)
    {
        h.content_length = strlen(splat[1]) + 2;
        send_preamble(fd, &h);
        write(fd, splat[1], strlen(splat[1]));
        write(fd, "\r\n", 2); /* Literally a new line, not just framing */
    }
    else
    {
        send_preamble(fd, &h);
        send_response_chunk(fd, splat[1], strlen(splat[1]));
        send_response_chunk(fd, "\r\n", 2); /* Literally a new line, not just framing */
        end_response_chunks(fd);
    }

    cleanup_response_preamble(&h);

    return 0;
}

