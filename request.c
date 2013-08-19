#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdlib.h>
#include "message.h"
#include "request.h"
#include "http_parser.h"
#include "route.h"

int handle_read_data(client_t *c, char* buf, int len)
{   
    return http_parser_execute(c->parser, c->parser_settings, buf, len);
}

int on_message_begin(http_parser *parser)
{
    syslog(LOG_DEBUG, "%s()...", __func__);
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
    message_t       *msg;

    req = parser->data;
    msg = req->msg;

    /* Append to the growing URL; we'll break it down later */
    extend_message_url(msg, at, len);

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
    message_t       *msg;

    req = parser->data;
    msg = req->msg;

    /* If we were last working on a value, we're in a new header now. */
    if(msg->last_header_element == VALUE || msg->last_header_element == NONE)
        add_message_header(msg, at, len, NULL, 0);
    /* Append to the header field-name; this might be a continuation due to a partial read */
    else
        extend_message_header_fieldname(msg, msg->num_headers - 1, at, len);

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
    message_t       *msg;

    req = parser->data;
    msg = req->msg;

    extend_message_header_value(msg, msg->num_headers - 1, at, len);

    msg->last_header_element = VALUE;

    /* debugging stuff...*/
    strncpy(tok, at, len);
    tok[len] = '\0';

    syslog(LOG_DEBUG, "%s(): token is %s", __func__, tok);
    return 0;
}

int on_headers_complete(http_parser *parser)
{
    struct request  *req;
    message_t       *msg;

    req = parser->data;
    msg = req->msg;
    syslog(LOG_DEBUG, "%s()...", __func__);

    /* Store HTTP major/minor codes */
    msg->http_major = parser->http_major;
    msg->http_minor = parser->http_minor;

    /* Break down the url into the path and query-string */
    msg->request_pathlen = strcspn(msg->request_url, "?");
    if(msg->request_pathlen)
    {
        msg->request_path = malloc(msg->request_pathlen);
        strncpy(msg->request_path, msg->request_url, msg->request_pathlen);
    }
    msg->query_stringlen = strlen(&msg->request_url[msg->request_pathlen+1]);
    if(msg->query_stringlen)
    {
        msg->query_string = malloc(msg->query_stringlen);
        strcpy(msg->query_string, &msg->request_url[msg->request_pathlen+1]);
    }

    msg->method = parser->method;
    switch(parser->method)
    {
        case(HTTP_GET):
            msg->method_name = "GET";
            break;
        case(HTTP_PUT):
            msg->method_name = "PUT";
            break;
        case(HTTP_POST):
            msg->method_name = "POST";
            break;
        case(HTTP_DELETE):
            msg->method_name = "DELETE";
            break;
        default:
            msg->method_name = "UNSUPPORTED";
            break;
    }

    describe_message(msg);
    return 0;
}

int on_body(http_parser *parser, const char *at, size_t len)
{
    struct request  *req;

    req = parser->data;
    extend_message_body(req->msg, at, len);

    syslog(LOG_DEBUG, "%s():...", __func__);
    return 0;
}


