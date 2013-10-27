#ifndef _REQUEST_H
#define _REQUEST_H
#include "types.h"
#include "http_parser.h"
#include "ev.h"

struct _client {
    http_parser             *parser;
    http_parser_settings    *parser_settings;
    int                     fd;
    struct ev_loop          *loop; /* Used if the ultimate request-handler needs to interact with libev */
    ev_io                   *io; /* Used if the ultimate request-handler needs to interact with libev */
    server_t                *srv;
    message_t               *msg;
};

int handle_read_data(client_t *c, char* buf, int len);

int on_message_begin(http_parser *parser);
int on_message_complete(http_parser *parser);
int on_url(http_parser *parser, const char *at, size_t len);
int on_status_complete(http_parser *parser);
int on_header_field(http_parser *parser, const char *at, size_t len);
int on_header_value(http_parser *parser, const char *at, size_t len);
int on_headers_complete(http_parser *parser);
int on_body(http_parser *parser, const char *at, size_t len);

#endif


