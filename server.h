#ifndef _SERVER_H
#define _SERVER_H
#include "types.h"
struct _server {
    route_t     **routes;
    int         numroutes;
};

void init_logging();
int start_server();
void terminate_client(client_t *c);
#endif
