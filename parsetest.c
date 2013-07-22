#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdlib.h>
#include "http_parser.h"

/* This structure definition lifted from https://github.com/joyent/http-parser/blob/master/test.c */
#define MAX_ELEMENT_SIZE 2048
#define MAX_HEADERS 64
struct message {
  const char *name; // for debugging purposes
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

  const char *upgrade; // upgraded body

  unsigned short http_major;
  unsigned short http_minor;

  int message_begin_cb_called;
  int headers_complete_cb_called;
  int message_complete_cb_called;
  int message_complete_on_eof;
  int body_is_final;
};

struct message GMSG;

void describe_request(struct message *m)
{
    int i;

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

    describe_request(&GMSG);
    return 0;
}

int on_body(http_parser *parser, const char *at, size_t len)
{
    strncat(GMSG.body, at, len);

    syslog(LOG_DEBUG, "%s():...", __func__);
    return 0;
}

void init_logging()
{
        openlog("parsetest", LOG_PID | LOG_PERROR, LOG_LOCAL0);
        setlogmask(LOG_UPTO(LOG_DEBUG));

        return;
}

int main(int argc, char** argv)
{
    http_parser_settings    settings;
    http_parser             *parser;
    char                    *request;
    int                     nparsed;

    init_logging();

    settings.on_message_begin = on_message_begin;
    settings.on_message_complete = on_message_complete;
    settings.on_url = on_url;
    settings.on_status_complete = on_status_complete;
    settings.on_header_field = on_header_field;
    settings.on_header_value = on_header_value;
    settings.on_headers_complete = on_headers_complete;
    settings.on_body = on_body;
    
    parser = malloc(sizeof(http_parser));

    puts("SIMPLE RUN------------------------------------------");
    http_parser_init(parser, HTTP_REQUEST);
    request = "GET /a/b HTTP/1.1\r\n"
              "User-Agent: curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n"
              "Host: localhost:8080\r\n"
              "Accept: */*\r\n"
              "\r\n";
    nparsed = http_parser_execute(parser, &settings, request, strlen(request));

    puts("SECOND RUN------------------------------------------");
    http_parser_init(parser, HTTP_REQUEST);
    request = "GET /a/";
    nparsed = http_parser_execute(parser, &settings, request, strlen(request));

    request = "b?asdf=123 HTTP/1.1\r\n"
              "User-Age";
    nparsed = http_parser_execute(parser, &settings, request, strlen(request));

    request = "nt: curl/7.22.0 (x86_64-pc-linux-gnu) ";
    nparsed = http_parser_execute(parser, &settings, request, strlen(request));

    request = "libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n"
              "Host: localhost:8080\r\n"
              "Accept: */*\r\n"
              "\r\n";
    nparsed = http_parser_execute(parser, &settings, request, strlen(request));

    puts("BIG BODY RUN------------------------------------------");
    http_parser_init(parser, HTTP_REQUEST);
    request = "PUT /blah/whatever?stuff=123 HTTP/1.1\r\n"
              "User-Agent: curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3\r\n"
              "Host: localhost:8080\r\n"
              "Accept: */*\r\n"
              "Content-Length: 1251\r\n"
              "Content-Type: application/x-www-form-urlencoded\r\n"
              "Expect: 100-continue\r\n"
              "\r\n"
              "http_parser.c is based on src/http/ngx_http_parse.c from NGINX copyright\n"
              "Igor Sysoev.\n"
              "\n"
              "Additional changes are licensed under the same terms as NGINX and\n"
              "copyright Joyent, Inc. and other Node contributors. All rights reserved.\n"
              "\n"
              "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
              "of this software and associated documentation files (the \"Software\"), to\n"
              "deal in the Software without restriction, including without limitation the\n"
              "rights to use, copy, modify, merge, publish, distribute, sublicense, and/or\n"
              "sell copies of the Software, and to permit persons to whom the Software is\n"
              "furnished to do so, subject to the following conditions:\n"
              "\n"
              "The above copyright notice and this permission notice shall be included in\n"
              "all copies or substantial portions of the Software.\n"
              "\n"
              "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
              "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
              "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
              "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
              "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
              "FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS\n"
              "IN THE SOFTWARE.\r\n";
    nparsed = http_parser_execute(parser, &settings, request, strlen(request));

    return 0;
}



//GET /blah?asdf=123 HTTP/1.1
//Host: localhost:8080
//Connection: keep-alive
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
//User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.70 Safari/537.17
//Accept-Encoding: gzip,deflate,sdch
//Accept-Language: en-US,en;q=0.8
//Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.


