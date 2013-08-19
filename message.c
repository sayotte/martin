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
        syslog(LOG_DEBUG, "Header[%d], Name->: '%s', Value: '%s'", i, m->headers[i].name, m->headers[i].value);
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
    return extend_string(&m->request_url, &m->request_urllen, buf, len, MSG_ALLOC_UNIT);
}

int extend_message_body(message_t *m, const char *buf, int len)
{
    syslog(LOG_DEBUG, "%s(): ...", __func__);
    return extend_string(&m->body, &m->bodylen, buf, len, MSG_ALLOC_UNIT);
}

int extend_message_header_fieldname(message_t *m, int index, const char *buf, int len)
{
    syslog(LOG_DEBUG, "%s(): ...", __func__);
    return extend_string(&m->headers[index].name, &m->headers[index].namelen, buf, len, MSG_ALLOC_UNIT);
}

int extend_message_header_value(message_t *m, int index, const char *buf, int len)
{
    syslog(LOG_DEBUG, "%s(): ...", __func__);
    return extend_string(&m->headers[index].value, &m->headers[index].valuelen, buf, len, MSG_ALLOC_UNIT);
}

int add_message_header(message_t *m, const char *name, int namelen, const char *value, int valuelen)
{
    struct header   *p;
    int             i;

    p = realloc(m->headers, sizeof(struct header) * (m->num_headers + 1));
    if(p == NULL)
    {
        syslog(LOG_CRIT, "%s(): realloc failed!", __func__);
        return 1;
    }
    m->headers = p;
    m->num_headers++;
    i = m->num_headers - 1;
    memset(&m->headers[i], 0, sizeof(struct header));

    if(name != NULL)
        extend_message_header_fieldname(m, i, name, namelen);

    if(value != NULL)
        extend_message_header_value(m, i, value, valuelen);

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
    int     i;

    syslog(LOG_DEBUG, "%s(): ...", __func__);

    if(m->request_urllen)
        free(m->request_url);
    if(m->request_pathlen)
        free(m->request_path);
    if(m->query_stringlen)
        free(m->query_string);
    if(m->bodylen)
        free(m->body);
    for(i = 0; i < m->num_headers; i++)
    {
        if(m->headers[i].namelen)
            free(m->headers[i].name);
        if(m->headers[i].valuelen)
            free(m->headers[i].value);
    }
    if(m->num_headers)
        free(m->headers);

    free(m);

    return;
}
