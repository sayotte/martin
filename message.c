#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "message.h"

void describe_message(message_t *m)
{
    int     i;

    syslog(LOG_DEBUG, "Method: %s", m->method_name);
    syslog(LOG_DEBUG, "HTTP version: %hd.%hd", m->http_major, m->http_minor);
    syslog(LOG_DEBUG, "URL: %s", m->request_url);
    syslog(LOG_DEBUG, "Path: %s", m->request_path);
    syslog(LOG_DEBUG, "Query-string: %s", m->query_string);

    for(i = 0; i < m->num_headers; i++)
    {  
        syslog(LOG_DEBUG, "Header[%d], Name->: '%s', Value: '%s'", i, m->headers[i][0], m->headers[i][1]);
    }
}

//int set_message_path

message_t *create_message()
{
    message_t   *m;

    syslog(LOG_DEBUG, "%s(): ...", __func__);

    m = valloc(sizeof(message_t));
    memset(m, 0, sizeof(message_t));

    return m;
}

void destroy_message(message_t *m)
{
    syslog(LOG_DEBUG, "%s(): ...", __func__);

    if(m->request_pathlen)
        free(m->request_path);

    free(m);

    return;
}
