#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "test.h"
#include "route.h"

int dummy_routine()
{
    return 0;
}

int dummy_routine2()
{
    return 0;
}

int bad_filename_returns_error()
{
    int     status;
    int     numroutes;
    route_t **routes;

    fork_to_test(
        status = parse_routes("/fubar", &routes, &numroutes);
        if(!status)
            exit(1);
        exit(0);
    )
}

int bad_lines_in_otherwise_good_file_ignored()
{
    char        *routefile_path;
    FILE        *routefile;
    int         status;
    int         numroutes;
    route_t     **routes;
    void        *FUNC, *FUNC2;

    char        *goodline = "GET /static dummy_routine\n";
    char        *badline = "FOO BAR BAZ\n";

    routefile_path = tempnam(".", "martin_unit_XXX");
    routefile = fopen(routefile_path, "w");
    if(!routefile)
    {
        printf("%s(): couldn't open tempfile '%s' for writing, aborting test!\n", __func__, routefile_path);
        status = -1;
        goto cleanup;
    }
    fwrite(goodline, strlen(goodline), 1, routefile);
    fwrite(badline, strlen(badline), 1, routefile);
    goodline = "PUT /static dummy_routine2\n";
    fwrite(goodline, strlen(goodline), 1, routefile);
    fwrite(badline, strlen(badline), 1, routefile);
    fclose(routefile);

    FUNC = dlsym(RTLD_DEFAULT, "dummy_routine");
    FUNC2 = dlsym(RTLD_DEFAULT, "dummy_routine2");
    if(FUNC == NULL || FUNC2 == NULL)
    {
        printf("%s(): couldn't resolve symbol 'dummy_routine' or 'dummy_routine2', aborting test!\n", __func__);
        status = -1;
        goto cleanup;
    }

    fork_and_get_exitcode(status,
        status = parse_routes(routefile_path, &routes, &numroutes);
        if(status)
        {
            printf("%s(): parse_routes returned %d\n", __func__, status);
            exit(status);
        }

        if(numroutes != 2)
        {
            printf("%s(): numroutes != 2\n", __func__);
            exit(1);
        }

        if(routes[0]->handler != FUNC || routes[1]->handler != FUNC2)
        {
            printf("%s(): handler resolved to an unexpected address\n", __func__);
            exit(1);
        }

        if(routes[0]->method != HTTP_GET || routes[1]->method != HTTP_PUT)
        {
            printf("%s(): method resolved incorrectly\n", __func__);
            exit(1);
        }

        exit(0);
    )

cleanup:
    unlink(routefile_path);
    free(routefile_path);

    return status;
}

int malformed_line_returns_2()
{
    int         status;
    route_t     *route;

    char        *line = "completely_fubar_line";

    fork_and_get_exitcode(status,
        status = parse_routeline(line, &route);
        if(status != 2)
            exit(1);

        exit(0);
    )

    return status;
}

int unrecognized_method_returns_6()
{
    int         status;
    route_t     *route;

    char        *line = "FUBAR /static dummy_routine";

    fork_and_get_exitcode(status,
        status = parse_routeline(line, &route);
        if(status != 6)
            exit(1);

        exit(0);
    )

    return status;
}

int malformed_path_regex_returns_5()
{
    int         status;
    route_t     *route;

    char        *line = "GET ^^[(^ dummy_routine";

    fork_and_get_exitcode(status,
        status = parse_routeline(line, &route);
        if(status != 5)
            exit(1);

        exit(0);
    )

    return status;
}

int line_with_unrecognized_handler_returns_4()
{
    int         status;
    route_t     *route;

    char        *goodline = "GET /static fubar_routine";

    fork_and_get_exitcode(status,
        status = parse_routeline(goodline, &route);
        if(status != 4)
            exit(1);

        exit(0);
    )

    return status;
}


int good_route_file_parses_correctly()
{
    char        *routefile_path;
    FILE        *routefile;
    int         status;
    int         numroutes;
    route_t     **routes;
    void        *FUNC;

    char        *goodline = "GET /static dummy_routine";

    routefile_path = tempnam(".", "martin_unit_XXX");
    routefile = fopen(routefile_path, "w");
    if(!routefile)
    {
        printf("%s(): couldn't open tempfile '%s' for writing, aborting test!\n", __func__, routefile_path);
        status = -1;
        goto cleanup;
    }
    fwrite(goodline, strlen(goodline), 1, routefile);
    fclose(routefile);

    FUNC = dlsym(RTLD_DEFAULT, "dummy_routine");
    if(FUNC == NULL)
    {
        printf("%s(): couldn't resolve symbol 'dummy_routine', aborting test!: %s\n", __func__, dlerror());
        status = -1;
        goto cleanup;
    }

    fork_and_get_exitcode(status,
        status = parse_routes(routefile_path, &routes, &numroutes);
        if(status)
        {
            printf("%s(): parse_routes returned %d\n", __func__, status);
            exit(status);
        }

        if(numroutes != 1)
        {
            printf("%s(): numroutes != 1\n", __func__);
            exit(1);
        }

        if(routes[0]->handler != FUNC)
        {
            printf("%s(): handler resolved to an unexpected address\n", __func__);
            exit(1);
        }

        if(routes[0]->method != HTTP_GET)
        {
            printf("%s(): method resolved incorrectly\n", __func__);
            exit(1);
        }

        exit(0);
    )

    cleanup:
        unlink(routefile_path);
        free(routefile_path);

    return status;
}

int main()
{
    int count, fail;

    count = 0;
    fail = 0;

    returns_zero(count, fail, "Vanilla (good) route file parses correctly", good_route_file_parses_correctly);
    returns_zero(count, fail, "Bad lines in otherwise good routes file are ignored", bad_lines_in_otherwise_good_file_ignored);
    returns_zero(count, fail, "Bad filename returns error", bad_filename_returns_error);
    returns_zero(count, fail, "Malformed line returns 2", malformed_line_returns_2);
    returns_zero(count, fail, "Otherwise good line with unrecognized HTTP method returns 6", unrecognized_method_returns_6);
    returns_zero(count, fail, "Otherwise good line with malformed path regex returns 5", malformed_path_regex_returns_5);
    returns_zero(count, fail, "Otherwise good line with unrecognized handler returns 4", line_with_unrecognized_handler_returns_4);

    printf("\n[route-test] Total tests passed/run: %d/%d\n", count - fail, count);
    puts("------------------------------------------------------------\n");

    return fail;
}

