#ifndef MESSAGE_H 
#define MESSAGE_H

#include "http_parser.h"

#define MSG_ALLOC_UNIT  256
#define MAX_ELEMENT_SIZE 2048
#define MAX_HEADERS 64

/* This structure definition modified from https://github.com/joyent/http-parser/blob/master/test.c */
typedef struct message {
//  const char *name; /* for debugging purposes */
//  const char *raw;
//  enum http_parser_type type;
  enum http_method method;
  const char *method_name;
  int status_code;
  int request_pathlen; 
  char *request_path;
  int request_urllen;
  char *request_url;
//  char fragment[MAX_ELEMENT_SIZE];
  int query_stringlen;
  char *query_string;
  int bodylen;
  char *body;
//  size_t body_size;
  const char *host;
//  const char *userinfo;
//  uint16_t port;
  int num_headers;
  enum { NONE=0, FIELD, VALUE } last_header_element;
  char headers [MAX_HEADERS][2][MAX_ELEMENT_SIZE];
//  int should_keep_alive;

//  const char *upgrade; /* upgraded body */

  unsigned short http_major;
  unsigned short http_minor;

/*
  int message_begin_cb_called;
  int headers_complete_cb_called;
  int message_complete_cb_called;
  int message_complete_on_eof;
  int body_is_final;
*/
} message_t;

void describe_message(message_t *m);
int extend_string(char **dst, int *dstlen, const char *src, int srclen, int unitsize);
int extend_message_url(message_t *m, const char *buf, int len);
int extend_message_body(message_t *m, const char *buf, int len);
message_t *create_message();
void destroy_message(message_t *m);

#endif


