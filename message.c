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

int extend_string(char **dst, int *dstlen, const char *src, int srclen, int unitsize)
{
    int     total_len;
    int     units_needed;
    int     units_allocated;
    char    *ptr;

    units_allocated = *dstlen / unitsize;
    if((*dstlen % unitsize) != 0)
        units_allocated++;
    
    total_len = *dstlen + srclen;
    units_needed = total_len / unitsize;
    if((total_len % unitsize) != 0)
        units_needed++;

    if(units_needed > units_allocated)
    {
        ptr = realloc(*dst, unitsize * units_needed);
        if(!ptr)
        {
            syslog(LOG_ERR, "%s(): realloc FAILED, likely running out of heap!", __func__);
            return -1;
        }
        else
            *dst = ptr;
    }

    strncat(*dst, src, srclen);
    *dstlen += srclen;

    return 0;
}

int extend_message_url(message_t *m, const char *buf, int len)
{
    syslog(LOG_DEBUG, "%s(): ...", __func__);
    return(extend_string(&m->request_url, &m->request_urllen, buf, len, MSG_ALLOC_UNIT));
}

int extend_message_body(message_t *m, const char *buf, int len)
{
    syslog(LOG_DEBUG, "%s(): ...", __func__);
    return(extend_string(&m->body, &m->bodylen, buf, len, MSG_ALLOC_UNIT));
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
    if(m->request_pathlen)
        free(m->request_path);
    if(m->query_stringlen)
        free(m->query_string);
    if(m->bodylen)
        free(m->body);

    free(m);

    return;
}
