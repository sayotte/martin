#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdlib.h>
#include "request.h"
#include "http_parser.h"
#include "route.h"

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
    struct request  *req;
    struct message  *msg;

    req = parser->data;
    msg = &req->msg;
    syslog(LOG_DEBUG, "%s()...", __func__);
    memset(msg, 0, sizeof(struct message));
    return 0;
}

int on_message_complete(http_parser *parser)
{
    syslog(LOG_DEBUG, "%s()...", __func__);

    puts("-------- response should be generated at this point ---------");

    route_request((struct request *)parser->data);

    return 0;
}

int on_url(http_parser *parser, const char *at, size_t len)
{
    char    tok[256];
    struct request  *req;

    req = parser->data;

    /* Append to the growing URL; we'll break it down later */
    strncat(&req->msg.request_url, at, len);

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
    struct request  *req;
    struct message  *msg;

    req = parser->data;
    msg = &req->msg;

    /* If we were last working on a value, we're in a new header now.
       Otherwise, append to the one we've partially completed. */
    if(msg->last_header_element == VALUE)
    {
        msg->num_headers++;
    }
    strncat(msg->headers[msg->num_headers][0], at, len);
    msg->last_header_element = FIELD;

    /* debugging stuff...*/
    strncpy(tok, at, len);
    tok[len] = '\0';

    syslog(LOG_DEBUG, "%s(): token is %s", __func__, tok);
    return 0;
}

int on_header_value(http_parser *parser, const char *at, size_t len)
{
    char    tok[256];
    struct request  *req;
    struct message  *msg;

    req = parser->data;
    msg = &req->msg;

    strncat(msg->headers[msg->num_headers][1], at, len);
    msg->last_header_element = VALUE;

    /* debugging stuff...*/
    strncpy(tok, at, len);
    tok[len] = '\0';

    syslog(LOG_DEBUG, "%s(): token is %s", __func__, tok);
    return 0;
}

int on_headers_complete(http_parser *parser)
{
    int     pathlen;
    struct request  *req;
    struct message  *msg;

    req = parser->data;
    msg = &req->msg;
    syslog(LOG_DEBUG, "%s()...", __func__);

    /* Break down the url into the path and query-string */
    pathlen = strcspn(msg->request_url, "?");
    strncpy(msg->request_path, msg->request_url, pathlen);
    msg->request_path[pathlen] = '\0';
    strcpy(msg->query_string, &msg->request_url[pathlen+1]);

    msg->method = parser->method;

    describe_request(msg);
    return 0;
}

int on_body(http_parser *parser, const char *at, size_t len)
{
    struct request  *req;

    req = parser->data;
    strncat(req->msg.body, at, len);

    syslog(LOG_DEBUG, "%s():...", __func__);
    return 0;
}


