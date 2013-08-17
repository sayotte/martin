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

int extend_message_url(message_t *m, const char *buf, int len)
{
    int     total_len;
    int     units_needed;
    int     units_allocated;
    char    *ptr;

    units_allocated = m->request_urllen / MSG_ALLOC_UNIT;
    if((m->request_urllen % MSG_ALLOC_UNIT) != 0)
        units_allocated++;
    
    total_len = m->request_urllen + len;
    units_needed = total_len / MSG_ALLOC_UNIT;
    if((total_len % MSG_ALLOC_UNIT) != 0)
        units_needed++;

    if(units_needed > units_allocated)
    {
        ptr = realloc(m->request_url, MSG_ALLOC_UNIT * units_needed);
        if(!ptr)
        {
            syslog(LOG_ERR, "%s(): realloc FAILED, likely running out of heap!", __func__);
            return -1;
        }
        else
            m->request_url = ptr;
    }

    strncat(m->request_url, buf, len);
    m->request_urllen += len;

    return 0;
}

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

    if(m->request_urllen)
        free(m->request_url);
    else
        syslog(LOG_DEBUG, "%s(): not freeing m->request_url, because m->request_urllen is %d", __func__, m->request_urllen);
    if(m->request_pathlen)
        free(m->request_path);
    else
        syslog(LOG_DEBUG, "%s(): not freeing m->request_path, because m->request_pathlen is %d", __func__, m->request_pathlen);
    if(m->query_stringlen)
        free(m->query_string);
    else
        syslog(LOG_DEBUG, "%s(): not freeing m->query_string, because m->query_stringlen is %d", __func__, m->query_stringlen);

    free(m);

    return;
}
