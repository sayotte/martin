#include <syslog.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
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

int start_server()
{
    struct sockaddr_in servaddr, clientaddr;
    int error = 0, server, client;
    int port = 8080; /* statically defined! */
    socklen_t clientaddr_len;
    int optval;

    init_logging();

    error = setup_routes();
    if(error != 0)
    {
        syslog(LOG_CRIT, "%s(): Failed to compile route regexes, aborting!", __func__);
        return 1;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    clientaddr_len = sizeof(clientaddr);
    
    server = socket(AF_INET, SOCK_STREAM, 0);
    if(server == -1)
    {
        syslog(LOG_CRIT, "%s(): Failed to allocate socket: %s\n", __func__, strerror(errno));
        return 2;
    }
    
    servaddr.sin_family         = AF_INET;
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    servaddr.sin_port           = htons(port);

    optval = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    
    error = bind(server, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    if(error)
    {
        syslog(LOG_ERR, "%s(): Failed to bind to socket: %s\n", __func__, strerror(errno));
        return 3;
    }
    syslog(LOG_DEBUG, "%s(): Bound to socket", __func__);
    
    error = listen(server, 1000);
    if(error)
    {
        syslog(LOG_CRIT, "%s(): Failed to listen on socket?!: %s\n", __func__, strerror(errno));
        return 4;
    }
    syslog(LOG_DEBUG, "%s(): Listening on socket", __func__);
    
    while(1)
    {
        client = accept(server, (struct sockaddr *)&clientaddr, &clientaddr_len);
        if(client == -1)
        {
            syslog(LOG_ERR, "%s(): Failed to accept connection on socket: %s\n", __func__, strerror(errno));
            continue;
        }

        error = handle_client(client);
        if(error)
        {
            syslog(LOG_DEBUG, "%s(): handle_client() returned %d", __func__, error);
        }
    }

    return 0;
}


