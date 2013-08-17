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

    syslog(LOG_DEBUG, "Method: %s", method_name);
    syslog(LOG_DEBUG, "URL: %s", m->request_url);
    syslog(LOG_DEBUG, "Path: %s", m->request_path);
    syslog(LOG_DEBUG, "Query-string: %s", m->query_string);

    for(i = 0; i < m->num_headers; i++)
    {
        syslog(LOG_DEBUG, "Header[%d], Name->: '%s', Value: '%s'", i, m->headers[i][0], m->headers[i][1]);
    }
}

int handle_read_data(client_t *c, char* buf, int len)
{   
    return http_parser_execute(c->parser, c->parser_settings, buf, len);
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

    syslog(LOG_DEBUG, "-------- response should be generated at this point ---------");

    route_request((struct request *)parser->data);

    return 0;
}

int on_url(http_parser *parser, const char *at, size_t len)
{
    char    tok[256];
    struct request  *req;

    req = parser->data;

    /* Append to the growing URL; we'll break it down later */
    strncat(&req->msg.request_url[0], at, len);

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

    /* If we were last working on a value, we're in a new header now. */
    if(msg->last_header_element == VALUE)
    {
        msg->num_headers++;
    }
    /* Append to the header field-name; this might be a continuation due to a partial read */
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

    /* The on_header_value() call cannot know when it has received the last bytes
        of a header value, so it cannot increment the num_headers counter.
        Now that we're here, we can increment it, but we only want to do that if
        we've actually received at least one header! */
    /** XXX Note: this counts on the message structure being initialized to zero! **/
    if(msg->headers[0][0][0] != 0)
        msg->num_headers++;

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


