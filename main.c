#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "server.h"
#include "route.h"

int main(int argc, char** argv)
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


