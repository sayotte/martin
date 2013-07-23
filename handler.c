#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h> /* not needed when printf is removed */
#include "response.h"
#include "request.h"

int put_transaction(int fd, struct message *m, char **splat, int splat_len)
{
    int     i;
    syslog(LOG_DEBUG, "%s():...", __func__);

    for(i = 0; i < splat_len; i++)
    {
        printf("splat[%d]: '%s'\n", i, splat[i]);
    }
    return 0;
}

int get_all_transactions(int fd, struct message *m, char **splat, int splat_len)
{
    int     i;
    syslog(LOG_DEBUG, "%s():...", __func__);

    for(i = 0; i < splat_len; i++)
    {
        printf("splat[%d]: '%s'\n", i, splat[i]);
    }
    return 0;
}

int get_transaction(int fd, struct message *m, char **splat, int splat_len)
{
    header_t    h;
    char    *optional[4];

    syslog(LOG_DEBUG, "%s():...", __func__);

    init_response_header(&h);
    add_response_header(&h, "Connection: close");

//    h.content_length = strlen(splat[1]) + 2;

    send_header(fd, &h);

//    write(fd, splat[1], strlen(splat[1]));
//    write(fd, "\r\n", 2);

    send_response_chunk(fd, splat[1], strlen(splat[1]));
    send_response_chunk(fd, "\r\n", 2);
    end_response_chunks(fd);

    cleanup_response_header(&h);

    return 0;
}

int get_static(int fd, struct message *m, char **splat, int splat_len)
{
    header_t    h;
    char        *optional[4];

    return 0;   

}
