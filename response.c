#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "response.h"
#include "util.h"

#define MAX_HEADER_LENGTH 2048


static char* r400_response = 
    "HTTP/1.1 400 Bad Request\r\n"
    "Content-type: text/html\r\n"
    "\r\n"
    "<html>\r\n"
    " <body>\r\n"
    "  <h1>Bad Request</h1>\r\n"
    "  <p>This server did not understand your request.</p>\r\n"
    " </body>\r\n"
    "</html>\r\n";

static char* r501_template = 
    "HTTP/1.1 501 Method Not Implemented\r\n"
    "Content-type: text/html\r\n"
    "\r\n"
    "<html>\r\n"
    " <body>\r\n"
    "  <h1>Method Not Implemented</h1>\r\n"
    "  <p>The method %s is not implemented by this server.</p>\r\n"
    " </body>\r\n"
    "</html>\r\n";

void init_response_header(header_t *h)
{
    char    date[40];

    memset(h, 0, sizeof(header_t));
    h->status = 200;
    h->status_desc = "OK";
    h->content_type = "text/plain";
    h->content_length = 0;
    
    add_response_header(h, "Server: martin/1.0");
    gen_date_header(date);
    add_response_header(h, date);

    return;
}

void cleanup_response_header(header_t *h)
{
    int     i;

    for(i=0; i < h->num_headers; i++)
    {
        free(h->optional_headers[i]);
    }
    free(h->optional_headers);

    return;
}

void add_response_header(header_t *h, char* hdr)
{
    char    *newhdr;

    h->optional_headers = realloc(h->optional_headers, sizeof(char*) * h->num_headers + 1);

    newhdr = malloc(strlen(hdr));
    strcpy(newhdr, hdr);

    h->optional_headers[h->num_headers] = newhdr;

    h->num_headers++;

    return;
}

static char* response_template =
    "HTTP/1.1 %d %s\r\n"
    "Content-type: %s\r\n" 
    "%s"; /* additional header fields go here, e.g. Cache-Control and Pragma */

void send_header(int client, header_t *h)
{
    char    *header, *optional_buf;
    char    length_buf[64];
    int     i;

    syslog(LOG_DEBUG, "%s(): ...", __func__);

    /* Build the content-length string, or send Transfer-Encoding: chunked */
    if(h->content_length)
    {
        snprintf(length_buf, 64, "Content-Length: %d\r\n", h->content_length);
    } 
    else {
        snprintf(length_buf, 64, "Transfer-Encoding: chunked\r\n");       
    }
    add_response_header(h, length_buf);

    /* Build the string of headers */
    if(h->num_headers)
    {
        optional_buf = calloc(MAX_HEADER_LENGTH, sizeof(char));
        for(i=0; i < h->num_headers; i++)
        {
            strcat(optional_buf, h->optional_headers[i]);
            strcat(optional_buf, "\r\n");
        }
    }
    else
        optional_buf = NULL;


    /* Build the final header */
    header = malloc(MAX_HEADER_LENGTH);
    snprintf(header, MAX_HEADER_LENGTH, response_template, h->status, h->status_desc, h->content_type, optional_buf);

    /* Send the whole header */
    write(client, header, strlen(header));

    /* Clean up the heap */
    free(optional_buf);
    free(header);

    return;
}


void send_response_chunk(int client, char *buf, int size)
{
    char    sizeline[64];

    syslog(LOG_DEBUG, "%s():...", __func__);
    /*syslog(LOG_DEBUG, "%s(%d, %s, %d):...", __func__, client, buf, size);
    */
   
    /* Send the size header, e.g. "4\r\n" if we're about to send a 4-byte chunk */
    snprintf(sizeline, 64, "%X\r\n", size);
    write(client, sizeline, strlen(sizeline));

    write(client, buf, size);
    write(client, "\r\n", 2);

    return;
}

void end_response_chunks(int client)
{
    syslog(LOG_DEBUG, "%s():...", __func__);
    write(client, "0\r\n\r\n", 5);

    return;
}
