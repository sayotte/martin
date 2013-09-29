#include "pcre.h"
#include "request.h"

#define MAX_OVECS   30

typedef struct route {
    enum http_method method;
    pcre    *re;
    int (*handler)(int, message_t*, char**, int);
} route_t;

extern route_t  **GROUTES;
extern int      GNUM_ROUTES;

int setup_routes();
int route_request(struct request *req);
int parse_routes(const char *filename, route_t ***routelist, int *numroutes);
int parse_routeline(char* line, route_t **route);

