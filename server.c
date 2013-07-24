#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "http_parser.h"
#include "request.h"
#include "route.h"

int handle_client(int client)
{
    http_parser_settings    settings;
    http_parser             *parser;
    int                     nparsed;
    char                    buf[MAX_ELEMENT_SIZE];
    ssize_t                 recved;

    settings.on_message_begin = on_message_begin;
    settings.on_message_complete = on_message_complete;
    settings.on_url = on_url;
    settings.on_status_complete = on_status_complete;
    settings.on_header_field = on_header_field;
    settings.on_header_value = on_header_value;
    settings.on_headers_complete = on_headers_complete;
    settings.on_body = on_body;
    
    parser = malloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);
    parser->data = &client;

    while(1)
    {
        recved = recv(client, buf, MAX_ELEMENT_SIZE, 0);
        if(recved < 0)
        {
            syslog(LOG_DEBUG, "%s(): recv() spit an error: %s", __func__, strerror(errno));
            break;
        }
        else if(recved == 0)
        {
            syslog(LOG_DEBUG, "%s(): remote peer closed the connection", __func__);
            break;
        }
        nparsed = http_parser_execute(parser, &settings, buf, recved);

        /* FIXME if(nparsed != recved) */
    }

    close(client);
    free(parser);

    return 0;
}

void init_logging()
{
        openlog("martin", LOG_PID | LOG_PERROR, LOG_LOCAL0);
        setlogmask(LOG_UPTO(LOG_DEBUG));

        return;
}


