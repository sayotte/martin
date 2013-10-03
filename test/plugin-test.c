#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"
#include "plugin.h"
#include "test.h"

int bad_plugin_dir_returns_1()
{
    int     status;
    fork_to_test(
        status = load_plugin("./FUBAR_DIR");
        if(status != 1)
        {
            printf("%s(): load_plugin returned %d\n", __func__, status);
            exit(1);
        }
        exit(0);
    )
}

int bad_plugin_returns_1()
{
    int     status;
    fork_to_test(
        status = load_plugin("badplugin" SO_SUFFIX);
        if(status != 1)
        {
            printf("%s(): load_plugin returned %d\n", __func__, status);
            exit(1);
        }
        exit(0);
    )
}

int happy_path()
{
    int status;
    void    *DL;

    fork_to_test(
        status = load_plugins_dir(".");
        if(status)
        {
            printf("%s(): load_plugins_dir() returned %d\n", __func__, status);
            exit(1);
        }

        if(NUMPLUGINS != 2)
        {
            printf("%s(): wrong number of plugins found (%d)\n", __func__, NUMPLUGINS);
            exit(2);
        }
    
        DL = dlsym(PLUGINS[0], "ok");
        if(DL == NULL)
        {
            printf("%s(): dlsym() failed to find symbol 'ok': %s\n", __func__, dlerror());
            exit(3);
        }
    
        DL = dlsym(PLUGINS[1], "notok");
        if(DL == NULL)
        {
            printf("%s(): dlsym() failed to find symbol 'notok': %s\n", __func__, dlerror());
            exit(4);
        }
    
        exit(0);
    )
}

int main()
{
    int count, fail;

    count = 0;
    fail = 0;

    returns_zero(count, fail, "Happy path returns zero", happy_path);
    returns_zero(count, fail, "Bad plugin directory returns one", bad_plugin_dir_returns_1);
    returns_zero(count, fail, "Bad plugin returns one", bad_plugin_returns_1);

    printf("\n[plugin-test] Total tests passed/run: %d/%d\n", count - fail, count);
    puts("------------------------------------------------------------\n");

/*
    if(!fail)
    {
        success = fopen("message.pass", "w");
        fclose(success);
    }
*/

    return fail;
}
