#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include "pcre.h"
#include "plugin.h"
#include "route.h"
#include "request.h"
#include "util.h"

route_t **GROUTES;
int     GNUM_ROUTES;

int setup_routes()
{
    int         status;
    int         numroutes;
    route_t     **routes;

    status = parse_routes("routes.txt", &routes, &numroutes);
    if(status)
    {
        syslog(LOG_WARNING, "%s(): parse_routes returned %d, leaving global route structure unchanged", __func__, status);
        return status;
    }

    GROUTES = routes;
    GNUM_ROUTES = numroutes;

    return 0;
}

int parse_routes(const char *filename, route_t ***routelist, int *numroutes)
{
    char    *line;
    FILE    *input;
    int     status;
    int     linenum;
    int     retcode;
    route_t **routes;
    route_t *route;
    void    *tmp;

    line = malloc(1024);

    input = fopen(filename, "r");
    if(input == NULL)
    {
        syslog(LOG_ERR, "%s(): failed to open file '%s' for reading", __func__, filename);
        retcode = 1;
        goto end;
    }

    linenum = 0;
    routes = NULL;
    while(fgets(line, 1024, input))
    {
        chomp(line);
        tmp = realloc(routes, sizeof(route_t*) * (linenum + 1));
        if(tmp == NULL)
        {
            syslog(LOG_ALERT, "%s(): reallocf() failed prepping to parse line #%d", __func__, linenum);
            retcode = 2;
            goto end1;
        }
        routes = tmp;
        status = parse_routeline(line, &routes[linenum]);
        if(status)
        {
            syslog(LOG_WARNING, "%s(): parse_routeline() returned %d, moving to next line", __func__, status);
            //retcode = 3;
            //goto end;
            continue;
        }
        route = routes[linenum];
        syslog(LOG_DEBUG, "%s(): route->handler: %p", __func__, route->handler);
        syslog(LOG_DEBUG, "%s(): route->method: %d", __func__, route->method);
        linenum++;
    }
    if(!feof(input))
    {
        syslog(LOG_WARNING, "%s(): fgets() encountered an error (not EOF)", __func__);
        retcode = 3;
        goto end1;
    }

    *routelist = routes;
    *numroutes = linenum;
    retcode = 0;

    end1:
        fclose(input);
    end:
        free(line);
        return retcode;
}

int parse_routeline(char* line, route_t **route)
{
    int         status, matches;
    pcre        *test_re, *route_re;
    char        *pattern;
    const char  *errmsg;
    int         erroffset;
    void        *FUNC;
    int         outputvec[MAX_OVECS];
    char        method[16], path[256], handler[256];
    int         method_code;
    route_t     *myroute;

    /* Build the pattern we use to extract fields from the line */
    pattern = "^(\\w+)\\s+(\\S+)\\s+(\\S+)";
    test_re = pcre_compile( pattern,
                            0,
                            &errmsg,
                            &erroffset,
                            NULL);
    if(test_re == NULL)
    {
        syslog(LOG_CRIT, "%s(): PCRE compilation error", __func__);
        return 1;
    }
    // FIXME probable memory leak on this RE; we never free() it

    /* Invoke the pattern-match against the line */
    status = pcre_exec( test_re,
                        NULL,
                        line,
                        strlen(line),
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
                syslog(LOG_WARNING, "%s(): line '%s' does not conform to regex '%s'; aborting", __func__, line, pattern);
                return 2;
            default:
                syslog(LOG_ERR, "%s(): PCRE matching error %d on line '%s'", __func__, status, line);
                return 2;
        }
    }
    matches = status;

    /* Extract fields from the line */
    status = pcre_copy_substring(line, outputvec, matches, 1, method, 16);
    if(status <= 0)
        return 3;
    status = pcre_copy_substring(line, outputvec, matches, 2, path, 256);
    if(status <= 0)
        return 3;
    status = pcre_copy_substring(line, outputvec, matches, 3, handler, 256);
    if(status <= 0)
        return 3;

    /* Resolve the handler-symbol into a function pointer */
    #ifdef RTLD_SELF
        /* Darwin's dlsym() searches all shared objects that were linked to the executable,
           but NOT those that have been linked with dlopen() */
        /* First search linked-in symbols */
        FUNC = dlsym(RTLD_SELF, handler);
        if(FUNC != NULL)
            syslog(LOG_DEBUG, "%s(): Found symbol '%s' in builtins", __func__, handler);
        /* Next search among loaded plugins */
        for(int i = 0; i < NUMPLUGINS && FUNC == NULL; i++)
        {
            FUNC = dlsym(PLUGINS[i], handler);
            if(FUNC != NULL)
                syslog(LOG_DEBUG, "%s(): Found symbol '%s' in plugin #%d'", __func__, handler, i);
        }
    #else
        /* Linux's dlsym() searches all loaded libraries for us */
        Dl_info     info;
        FUNC = dlsym(RTLD_DEFAULT, handler);
        if(FUNC != NULL)
        {
            dladdr(FUNC, &info);
            syslog(LOG_DEBUG, "%s(): Found symbol '%s' in module '%s'", __func__, handler, info.dli_fname);
        }
    #endif
    /* Finally, fail if it remains unfound */
    if(FUNC == NULL)
    {
        syslog(LOG_WARNING, "%s(): Failed to find symbol '%s' in builtins or plugins", __func__, handler);
        return 4;
    }

    /* Compile the path component into an RE we can use for matching */
    route_re = pcre_compile( path,
                             0,
                             &errmsg,
                             &erroffset,
                             NULL);
    if(route_re == NULL)
    {
        syslog(LOG_WARNING, "%s(): PCRE compilation failed at offset %d with error '%s' in pattern '%s'", __func__, erroffset, errmsg, path);
        return 5;
    }
    // FIXME call pcre_inspect()

    /* Convert the method component into a symbolic integer */
    method_code = match_http_method(method);
    if(method_code == -1)
    {
        syslog(LOG_WARNING, "%s(): Unrecognized HTTP method '%s'", __func__, method);
        return 6;
    }

    /* Populate and return the route object */
    myroute = calloc(1, sizeof(route_t));
    myroute->re = route_re;
    myroute->method = method_code;
    myroute->handler = FUNC;

    *route = myroute;

    return 0;
}

int route_request(client_t *c)
{
    route_t     **routes;
    int         i;
    int         outputvec[MAX_OVECS];
    int         status;
    char        *splat[MAX_OVECS / 2];
    int         substring_len;
    int         splat_len;
    message_t   *m;
    int (*handler)(client_t*, char**, int);
    
    m = c->msg;

    syslog(LOG_DEBUG, "%s():...", __func__);

    /*** I keep this here, hoping one day to do away with the global...
         Thank you http-parser for forcing me to use globals. ***/
    routes = GROUTES;

    /* Find a matching route, matching on the (method, request_path) tuple */
    for(i = 0; i < GNUM_ROUTES; i++)
    {
        if(m->method != routes[i]->method)
            continue;

        status = pcre_exec( routes[i]->re,
                            NULL,
                            m->request_path,
                            m->request_pathlen,
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
    handler = routes[i]->handler;

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
    status = handler(c, splat, splat_len);
    syslog(LOG_DEBUG, "%s(): Handler returned %d", __func__, status);

    /* Clean-up any matched strings we saved, since they were heap-allocated */
    for(i=0; i < splat_len; i++)
        free(splat[i]);

    return 0;
}
