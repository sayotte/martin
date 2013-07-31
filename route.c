#include <syslog.h>
#include <string.h>
#include "pcre.h"
#include "route.h"
#include "request.h"

route_t *GROUTES;

int route_request(struct request *req)
{
    route_t     *routes;
    int         i;
    int         outputvec[MAX_OVECS];
    int         status;
    int         pathlen;
    char        *splat[MAX_OVECS / 2];
    int         substring_len;
    int         splat_len;
    int         client;
    struct message  *m;
    int (*handler)(int, struct message*, char**, int);
    
    client = req->fd;
    m = &req->msg;

    syslog(LOG_DEBUG, "%s():...", __func__);

    /*** I keep this here, hoping one day to do away with the global...
         Thank you http-parser for forcing me to use globals. ***/
    routes = GROUTES;

    /* Find a matching route, matching on the (method, request_path) tuple */
    pathlen = strlen(m->request_path);
    for(i = 0; i < GNUM_ROUTES; i++)
    {
        if(m->method != routes[i].method)
            continue;

        status = pcre_exec( routes[i].re,
                            NULL,
                            m->request_path,
                            pathlen,
                            0,
                            0,
                            outputvec,
                            MAX_OVECS);

        /* pcre_exec returns 0 for both non-matches and true errors */
        if(status < 0)
        {
            switch(status)
            {
                case PCRE_ERROR_NOMATCH:
                    continue;
                default:
                    syslog(LOG_ERR, "%s(): PCRE matching error %d on path '%s'", __func__, status, m->request_path);
                    return 1;
            }
        }

        /* We have a match, break out of the loop rather than matching additional handlers */
        syslog(LOG_DEBUG, "%s(): Found a match!", __func__);
        break;
    }
    
    /* We may've broken out of the loop without finding a match, in which case we should send a 
       404 */
    if(status < 0)
    {
        ; /* TODO send a 404 here */
        return 2; /* FIXME returning error for the time being, there should be a default 404 handler though */
    }
    /* Otherwise, grab the handler associated with the route */
    handler = routes[i].handler;

    /* If we found a match but ran out of outputvectors, pcre_exec will return 0 */
    if(status == 0)
    {
        syslog(LOG_ERR, "%s(): PCRE route-match found, but some matches lost due to not enough output-vectors", __func__);
        splat_len = 0;
    }
    /* Otherwise we should save aside the strings PCRE matched for us */
    else
    {
        splat_len = status;
        for(i=0; i < splat_len; i++)
        {
            substring_len = outputvec[2*i+1] - outputvec[2*i];
            splat[i] = calloc(substring_len + 1, sizeof(char));
            strncpy(splat[i], &m->request_path[outputvec[2*i]], substring_len);

            /*printf("Match %d: '%s'\n", i, splat[i]);
            */
        }
    }

    /* Call the handler associated with the route */
    status = handler(client, m, splat, splat_len);
    syslog(LOG_DEBUG, "%s(): Handler returned %d", __func__, status);

    /* Clean-up any matched strings we saved, since they were heap-allocated */
    for(i=0; i < splat_len; i++)
        free(splat[i]);

    return 0;
}
