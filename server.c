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
#include "ev.h"
#include "message.h"
#include "request.h"
#include "route.h"
#include "server.h"

#include <stdio.h>

void init_logging();
int setup_listen_socket(int *fd);
int start_server();
static void accept_cb(struct ev_loop *loop, ev_io *w, int revents);
static void clientread_cb(struct ev_loop *loop, ev_io *w, int revents);

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
    int             listen;
    int             error;
    ev_io           *accept_watcher;
    struct ev_loop  *loop;

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

    accept_watcher = malloc(sizeof(ev_io));
    loop = EV_DEFAULT;
    ev_io_init(accept_watcher, accept_cb, listen, EV_READ);
    ev_io_start(loop, accept_watcher);

    error = ev_run(loop, 0);
    syslog(LOG_DEBUG, "%s(): ev_run() returned %d", __func__, error);

    free(accept_watcher);
    close(listen);
    return 0;
}

static void accept_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    int                 clientfd; 
    struct sockaddr     clientaddr;
    socklen_t           clientaddr_len;
    ev_io               *client_read_watcher;
    client_t            *c;
    struct request      *req;

    clientaddr_len = sizeof(clientaddr);
    syslog(LOG_DEBUG, "%s(): accepting connection\n", __func__);
    clientfd = accept(w->fd, (struct sockaddr *)&clientaddr, &clientaddr_len);
    if(clientfd == -1)
    {
        syslog(LOG_ERR, "%s(): Failed to accept connection on socket: %s\n", __func__, strerror(errno));
        return;
    }
    syslog(LOG_DEBUG, "%s(): accepted a connection", __func__);

    c = malloc(sizeof(client_t));
    c->parser = malloc(sizeof(http_parser));
    c->parser_settings = malloc(sizeof(http_parser_settings));
    req = malloc(sizeof(request_t));
    req->msg = create_message();
    req->fd = clientfd;
    c->parser->data = req;

    c->parser_settings->on_message_begin = on_message_begin;
    c->parser_settings->on_message_complete = on_message_complete;
    c->parser_settings->on_url = on_url;
    c->parser_settings->on_status_complete = on_status_complete;
    c->parser_settings->on_header_field = on_header_field;
    c->parser_settings->on_header_value = on_header_value;
    c->parser_settings->on_headers_complete = on_headers_complete;
    c->parser_settings->on_body = on_body;

    http_parser_init(c->parser, HTTP_REQUEST);

    client_read_watcher = malloc(sizeof(ev_io));
    client_read_watcher->data = c;
    ev_io_init(client_read_watcher, clientread_cb, clientfd, EV_READ);

    ev_io_start(loop, client_read_watcher);
    return;
}

static void clientread_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    int                     nparsed;
    char                    buf[MAX_ELEMENT_SIZE];
    ssize_t                 recved;
    client_t                *c;
    request_t               *req;

    c = w->data;
    req = c->parser->data;

    recved = recv(req->fd, buf, MAX_ELEMENT_SIZE, 0);
    if(recved < 0)
    {   
        syslog(LOG_DEBUG, "%s(): recv() spit an error: '%s', cleaning up and closing the socket", __func__, strerror(errno));
        ev_io_stop(loop, w);
        close(w->fd);
        free(w);
        destroy_message(req->msg);
        free(c->parser->data);
        free(c->parser);
        free(c->parser_settings);
        free(c);
    }
    else if(recved == 0)
    {   
        syslog(LOG_DEBUG, "%s(): remote peer closed the connection, cleaning up and closing the socket", __func__);
        ev_io_stop(loop, w);
        close(w->fd);
        free(w);
        destroy_message(req->msg);
        free(c->parser->data);
        free(c->parser);
        free(c->parser_settings);
        free(c);
    }
    else
    {
        nparsed = handle_read_data(c, buf, recved);

        if(nparsed != recved)
            ; /* FIXME do something here */
    }

    return;
}

