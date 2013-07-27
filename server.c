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

pthread_mutex_t *setup_shared_mutex()
{
    pthread_mutex_t     *mtx;
    pthread_mutexattr_t attr;
    int                 error;
    
    mtx = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, -1, 0);
    if(mtx == MAP_FAILED)
    {
        syslog(LOG_CRIT, "%s(): mmap() failed: %s", __func__, strerror(errno));
        return NULL;
    }

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    error = pthread_mutex_init(mtx, &attr);
    if(error)
    {
        syslog(LOG_CRIT, "%s(): pthread_mutex_init() failed: %s", __func__, strerror(error));
        return NULL;
    }

    return mtx;
}

int setup_listen_socket(master_socket_t *ms)
{
    struct sockaddr_in  servaddr;
    int                 error;
    int                 port = 8080; /* statically defined! */
    int optval;
    
    ms->mtx = setup_shared_mutex();
    if(ms->mtx == NULL)
        return 1;
    
    ms->fd = socket(AF_INET, SOCK_STREAM, 0);
    if(ms->fd == -1)
    {
        syslog(LOG_CRIT, "%s(): Failed to allocate socket: %s\n", __func__, strerror(errno));
        return 2;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family         = AF_INET;
    servaddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    servaddr.sin_port           = htons(port);

    optval = 1;
    setsockopt(ms->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    error = bind(ms->fd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    if(error)
    {
        syslog(LOG_ERR, "%s(): Failed to bind to socket: %s\n", __func__, strerror(errno));
        return 3;
    }
    syslog(LOG_DEBUG, "%s(): Bound to socket", __func__);

    error = listen(ms->fd, 1000);
    if(error)
    {
        syslog(LOG_CRIT, "%s(): Failed to listen on socket?!: %s\n", __func__, strerror(errno));
        return 4;
    }
    syslog(LOG_DEBUG, "%s(): Listening on socket", __func__);

    return 0;
}
    

#define fork_worker()\
  syslog(LOG_DEBUG, "%s(): Forking a worker process", __func__);\
  mypid = fork();\
  if(mypid == 0)\
    return(worker_loop(&ms));

int start_server()
{
    master_socket_t     ms;
    int                 error, status, i;
    pid_t               mypid;

    init_logging();

    error = setup_routes();
    if(error != 0)
    {
        syslog(LOG_CRIT, "%s(): Failed to compile route regexes, aborting!", __func__);
        return 1;
    }

    error = setup_listen_socket(&ms);
    if(error != 0)
        return 2;

    /* Fork some workers */
    for(i=0; i < MAX_WORKERS; i++)
    {
        fork_worker();
    }

    while(1)
    {
        error = waitpid(-1, &status, WNOHANG);
        if(error > 0)
        {
            if(WIFEXITED(status))
            {
                syslog(LOG_DEBUG, "%s(): Worker process exited with status %d", __func__, WEXITSTATUS(status));

                fork_worker();
            }
        }
        else if(error < 0)
        {
            syslog(LOG_ERR, "%s(): waitpid() failed: %s", __func__, strerror(errno));
        }
    }

    return 0;
}

int worker_loop(master_socket_t *ms)
{
    int                 i, error;
    int                 client;
    struct sockaddr_in  clientaddr;
    socklen_t           clientaddr_len;

    clientaddr_len = sizeof(clientaddr);
    for(i=0; i < MAX_CONNS_PROCESSED; i++)
    {
        error = pthread_mutex_lock(ms->mtx);
        if(error)
        {
            syslog(LOG_ERR, "%s(): pthread_mutex_lock() failed: %s", __func__, strerror(error));
            return 1;
        }
        syslog(LOG_DEBUG, "%s(): accepting connection %d of %d max-allowed\n", __func__, i, MAX_CONNS_PROCESSED);
        client = accept(ms->fd, (struct sockaddr *)&clientaddr, &clientaddr_len);
        pthread_mutex_unlock(ms->mtx);
        if(client == -1)
        {
            syslog(LOG_ERR, "%s(): Failed to accept connection on socket: %s\n", __func__, strerror(errno));
            return 2;
        }
        syslog(LOG_DEBUG, "%s(): accepted a connection", __func__);

        error = handle_client(client);
        if(error)
        {
            syslog(LOG_DEBUG, "%s(): handle_client() returned %d", __func__, error);
        }
    }

    return 0;
}

