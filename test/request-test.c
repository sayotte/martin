#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "test.h"
#include "server.h" /* for client_t */
/*
typedef struct {
    http_parser             *parser;
    http_parser_settings    *parser_settings;
} client_t;
*/
#include "request.h"
#include "http_parser.h"

client_t                c;
request_t               req;
http_parser             parser;
http_parser_settings    settings;

int route_request(struct request *req)
{
    void    *shutup;
    shutup = req;

    return 0;
}

int method_is_parsed_correctly()
{
    char        buf[4096];
    FILE        *fd;
    int         bytes;

    fd = fopen("curl-http1.1-get.txt", "r");
    bytes = fread(buf, 1, 4096, fd);
    fclose(fd);

    fork_to_test(
        http_parser_init(c.parser, HTTP_REQUEST);

        bytes = handle_read_data(&c, buf, bytes);

        if(req.msg.method == HTTP_GET)
            exit(0);
        else
            exit(1);
    )
}

int main()
{
    int count, fail;
//    FILE    *success;

    count = 0;
    fail = 0;

    c.parser = &parser;
    c.parser_settings = &settings;
    c.parser->data = &req;

    c.parser_settings->on_message_begin = on_message_begin;
    c.parser_settings->on_message_complete = on_message_complete;
    c.parser_settings->on_url = on_url;
    c.parser_settings->on_status_complete = on_status_complete;
    c.parser_settings->on_header_field = on_header_field;
    c.parser_settings->on_header_value = on_header_value;
    c.parser_settings->on_headers_complete = on_headers_complete;
    c.parser_settings->on_body = on_body;

    returns_zero(count, fail, "Method is parsed correctly", method_is_parsed_correctly);

    printf("\n[request-test] Total tests passed/run: %d/%d\n", count - fail, count);
    puts("------------------------------------------------------------\n");

/*
    if(!fail)
    {
        success = fopen("request.pass", "w");
        fclose(success);
    }
*/

    return fail;
}
