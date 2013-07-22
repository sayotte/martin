#include <syslog.h>
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
    int     i;
    syslog(LOG_DEBUG, "%s():...", __func__);

    for(i = 0; i < splat_len; i++)
    {
        printf("splat[%d]: '%s'\n", i, splat[i]);
    }
    return 0;
}

