#ifndef _ROUTE_H
#define _ROUTE_H
#include "types.h"
#include "pcre.h"
#include "http_parser.h"

#define MAX_OVECS   30

struct _route {
    enum http_method method;
    pcre    *re;
    int (*handler)(client_t*, char**, int);
};

int setup_routes();
int route_request(client_t *c);
int parse_routes(const char *filename, route_t ***routelist, int *numroutes);
int parse_routeline(char* line, route_t **route);
#endif
