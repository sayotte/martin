#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int path_and_query_string_parsed_correctly()
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

//        printf("\nrequest_path: %s\n", req.msg->request_path);
//        printf("query_string: %s\n", req.msg->query_string);

        if(strcmp(req.msg->request_path, "/api/transactions/asdlfkjasdf"))
            exit(1);
        if(strcmp(req.msg->query_string, "ok=blah"))
            exit(2);

        exit(0);
    )
}

int http_version_is_parsed_correctly()
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
    //    printf("Major: %d\tMinor: %d\n", req.msg->http_major, req.msg->http_minor);

        if(req.msg->http_major == 1 && req.msg->http_minor == 1)
            exit(0);
        else
            exit(1);
    )
}

int headers_parsed_correctly()
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

//        puts("");
//        for(bytes = 0; bytes < req.msg->num_headers; bytes++)
//        {   
//            printf("Header[%d], Name->: '%s', Value: '%s'\n", bytes, req.msg->headers[bytes][0], req.msg->headers[bytes][1]);
//        }

        if(strcmp(req.msg->headers[0].name, "User-Agent"))
            exit(1);
        if(strcmp(req.msg->headers[0].value, "curl/7.19.7 (universal-apple-darwin10.0) libcurl/7.19.7 OpenSSL/0.9.8x zlib/1.2.3"))
            exit(2);
        if(strcmp(req.msg->headers[1].name, "Host"))
            exit(3);
        if(strcmp(req.msg->headers[1].value, "localhost:8080"))
            exit(4);
        if(strcmp(req.msg->headers[2].name, "Accept"))
            exit(5);
        if(strcmp(req.msg->headers[2].value, "*/*"))
            exit(6);
        if(req.msg->num_headers != 3)
            exit(7);

        exit(0);
    )
}

int body_is_parsed_correctly()
{
    char        buf[4096];
    FILE        *fd;
    int         bytes;
    char        *expected_body;

    fd = fopen("curl-http1.1-post.txt", "r");
    bytes = fread(buf, 1, 4096, fd);
    fclose(fd);

    expected_body = "void gen_date_header(char* dst);\n"
                    "off_t fsize(const char* filename);\n";

    fork_to_test(
        http_parser_init(c.parser, HTTP_REQUEST);

        bytes = handle_read_data(&c, buf, bytes);

        if(strcmp(req.msg->body, expected_body))
            exit(1);

        exit(0);
    )
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

        if(req.msg->method == HTTP_GET)
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
    req.msg = create_message();
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
    returns_zero(count, fail, "HTTP version is parsed correctly", http_version_is_parsed_correctly);
    returns_zero(count, fail, "Path and query string are parsed correctly", path_and_query_string_parsed_correctly);
    returns_zero(count, fail, "Headers are parsed correctly", headers_parsed_correctly);
    returns_zero(count, fail, "Body is parsed correctly", body_is_parsed_correctly);

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
