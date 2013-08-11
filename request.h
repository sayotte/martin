#ifndef REQUEST_H
#define REQUEST_H
#include "http_parser.h"

typedef struct {
    http_parser             *parser;
    http_parser_settings    *parser_settings;
} client_t;

int handle_read_data(client_t *c, char* buf, int len);

int on_message_begin(http_parser *parser);
int on_message_complete(http_parser *parser);
int on_url(http_parser *parser, const char *at, size_t len);
int on_status_complete(http_parser *parser);
int on_header_field(http_parser *parser, const char *at, size_t len);
int on_header_value(http_parser *parser, const char *at, size_t len);
int on_headers_complete(http_parser *parser);
int on_body(http_parser *parser, const char *at, size_t len);

#define MAX_ELEMENT_SIZE 2048
#define MAX_HEADERS 64

/* This structure definition lifted from https://github.com/joyent/http-parser/blob/master/test.c */
struct message {
  const char *name; /* for debugging purposes */
  const char *raw;
  enum http_parser_type type;
  enum http_method method;
  int status_code;
  char request_path[MAX_ELEMENT_SIZE];
  char request_url[MAX_ELEMENT_SIZE];
  char fragment[MAX_ELEMENT_SIZE];
  char query_string[MAX_ELEMENT_SIZE];
  char body[MAX_ELEMENT_SIZE];
  size_t body_size;
  const char *host;
  const char *userinfo;
  uint16_t port;
  int num_headers;
  enum { NONE=0, FIELD, VALUE } last_header_element;
  char headers [MAX_HEADERS][2][MAX_ELEMENT_SIZE];
  int should_keep_alive;

  const char *upgrade; /* upgraded body */

  unsigned short http_major;
  unsigned short http_minor;

  int message_begin_cb_called;
  int headers_complete_cb_called;
  int message_complete_cb_called;
  int message_complete_on_eof;
  int body_is_final;
};

typedef struct request {
    struct message  msg;
    int             fd;
} request_t;

#endif


