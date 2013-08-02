#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include "http_parser.h"
#include "request.h"
#include "route.h"
#include "server.h"

#include <stdio.h>

int handle_client(int client)
{
    http_parser_settings    settings;
    http_parser             *parser;
    int                     nparsed;
    char                    buf[MAX_ELEMENT_SIZE];
    ssize_t                 recved;
    request_t               *req;

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

    req = malloc(sizeof(request_t));
    req->fd = client;
    parser->data = req;

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

        if(nparsed != recved)
            ; /* FIXME do something here */
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

int setup_listen_socket(int *fd)
{
    struct sockaddr_in  servaddr;
    int                 error;
    int                 port = 8080; /* statically defined! */
    int optval;
    
    *fd = socket(AF_INET, SOCK_STREAM, 0);
    if(*fd == -1)
    {
        syslog(LOG_CRIT, "%s(): Failed to allocate socket: %s\n", __func__, strerror(errno));
        return 1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family         = AF_INET;
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    servaddr.sin_port           = htons(port);

    optval = 1;
    setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    error = bind(*fd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    if(error)
    {
        syslog(LOG_ERR, "%s(): Failed to bind to socket: %s\n", __func__, strerror(errno));
        return 2;
    }
    syslog(LOG_DEBUG, "%s(): Bound to socket", __func__);

    error = listen(*fd, 1000);
    if(error)
    {
        syslog(LOG_CRIT, "%s(): Failed to listen on socket?!: %s\n", __func__, strerror(errno));
        return 3;
    }
    syslog(LOG_DEBUG, "%s(): Listening on socket", __func__);

    return 0;
}
    
int start_server()
{
    int                 listen;
    int                 error;
    int                 client; 
    struct sockaddr     clientaddr;
    socklen_t           clientaddr_len;

    init_logging();

    error = setup_routes();
    if(error != 0)
    {
        syslog(LOG_CRIT, "%s(): Failed to compile route regexes, aborting!", __func__);
        return 1;
    }

    error = setup_listen_socket(&listen);
    if(error != 0)
        return 2;

    clientaddr_len = sizeof(clientaddr);
    while(1)
    {
        syslog(LOG_DEBUG, "%s(): accepting connection\n", __func__);
        client = accept(listen, (struct sockaddr *)&clientaddr, &clientaddr_len);
        if(client == -1)
        {
            syslog(LOG_ERR, "%s(): Failed to accept connection on socket: %s\n", __func__, strerror(errno));
            return 3;
        }
        syslog(LOG_DEBUG, "%s(): accepted a connection", __func__);
    
        error = handle_client(client);
        if(error)
        {
            syslog(LOG_DEBUG, "%s(): handle_client() returned %d", __func__, error);
        }
    }
}

