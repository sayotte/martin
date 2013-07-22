#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdlib.h>
#include "request.h"
#include "http_parser.h"
#include "route.h"

struct message GMSG;

void describe_request(struct message *m)
{
    int     i;
    char    *method_name;

    switch(m->method)
    {
        case(HTTP_GET):
            method_name = "GET";
            break;
        case(HTTP_PUT):
            method_name = "PUT";
            break;
        case(HTTP_POST):
            method_name = "POST";
            break;
        case(HTTP_DELETE):
            method_name = "DELETE";
            break;
        default:
            method_name = "UNSUPPORTED";
            break;
    }

    printf("Method: %s\n", method_name);
    printf("URL: %s\n", m->request_url);
    printf("Path: %s\n", m->request_path);
    printf("Query-string: %s\n", m->query_string);

    for(i = 0; i < m->num_headers; i++)
    {
        printf("Header[%d], Name->: '%s', Value: '%s'\n", i, m->headers[i][0], m->headers[i][1]);
    }
}

int on_message_begin(http_parser *parser)
{
    syslog(LOG_DEBUG, "%s()...", __func__);
    memset(&GMSG, 0, sizeof(struct message));
    return 0;
}

int on_message_complete(http_parser *parser)
{
    syslog(LOG_DEBUG, "%s()...", __func__);

    puts("-------- response should be generated at this point ---------");

    route_request(parser->data, &GMSG);

    return 0;
}

int on_url(http_parser *parser, const char *at, size_t len)
{
    char    tok[256];
    /* Append to the growing URL; we'll break it down later */
    strncat(GMSG.request_url, at, len);

    /* debugging stuff...*/
    strncpy(tok, at, len);
    tok[len] = '\0';

    syslog(LOG_DEBUG, "%s(): token is %s", __func__, tok);
    return 0;
}

int on_status_complete(http_parser *parser)
{
    syslog(LOG_DEBUG, "%s()...", __func__);
    return 0;
}

int on_header_field(http_parser *parser, const char *at, size_t len)
{
    char    tok[256];

    /* If we were last working on a value, we're in a new header now.
       Otherwise, append to the one we've partially completed. */
    if(GMSG.last_header_element == VALUE)
    {
        GMSG.num_headers++;
    }
    strncat(GMSG.headers[GMSG.num_headers][0], at, len);
    GMSG.last_header_element = FIELD;

    /* debugging stuff...*/
    strncpy(tok, at, len);
    tok[len] = '\0';

    syslog(LOG_DEBUG, "%s(): token is %s", __func__, tok);
    return 0;
}

int on_header_value(http_parser *parser, const char *at, size_t len)
{
    char    tok[256];

    strncat(GMSG.headers[GMSG.num_headers][1], at, len);
    GMSG.last_header_element = VALUE;

    /* debugging stuff...*/
    strncpy(tok, at, len);
    tok[len] = '\0';

    syslog(LOG_DEBUG, "%s(): token is %s", __func__, tok);
    return 0;
}

int on_headers_complete(http_parser *parser)
{
    int     pathlen;
    syslog(LOG_DEBUG, "%s()...", __func__);

    /* Break down the url into the path and query-string */
    pathlen = strcspn(GMSG.request_url, "?");
    strncpy(GMSG.request_path, GMSG.request_url, pathlen);
    GMSG.request_path[pathlen] = '\0';
    strcpy(GMSG.query_string, &GMSG.request_url[pathlen+1]);

    GMSG.method = parser->method;

    describe_request(&GMSG);
    return 0;
}

int on_body(http_parser *parser, const char *at, size_t len)
{
    strncat(GMSG.body, at, len);

    syslog(LOG_DEBUG, "%s():...", __func__);
    return 0;
}


