#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include "pcre.h"
#include "plugin.h"

void    **PLUGINS = NULL;
int     NUMPLUGINS = 0;

int load_plugins_dir(char *plugin_path)
{
    DIR             *dirp;
    struct dirent   *ent;
    char            *pattern;
    const char      *errmsg;
    int             erroffset;
    pcre            *re;
    int             status;
    int             outputvec[3];
    char            dlpath[256];

    dirp = opendir(plugin_path);
    if(dirp == NULL)
    {
        syslog(LOG_WARNING, "%s(): opendir failed", __func__);
        return 1;
    }

    pattern = "^[\\w\\.]+\\.so$";
    re = pcre_compile(  pattern,
                        0,
                        &errmsg,
                        &erroffset,
                        NULL);
    if(re == NULL)
    {
        syslog(LOG_CRIT, "%s(): PCRE compilation error", __func__);
        return 2;
    }
    // FIXME all erroneous returns farther down leak the memory for re

    while((ent = readdir(dirp)) != NULL)
    {
        status = pcre_exec( re,
                            NULL,
                            ent->d_name,
                            strlen(ent->d_name),
                            0, 0, outputvec, 3);
        /* Status > 0 is a match */
        /*** Note that pcre_exec() never returns 0 ***/
        if(status > 0)
        {
            /* Load the plugin! */
            sprintf(dlpath, "%s/%s", plugin_path, ent->d_name);
            load_plugin(dlpath);
        }
        /* Status < 0 is either a non-match or an error */
        else if(status < 0)
        {
            switch(status)
            {
                case PCRE_ERROR_NOMATCH:
                    break;
                default:
                    syslog(LOG_CRIT, "%s(): PCRE mattaching error %d on ent '%s'", __func__, status, ent->d_name);
                    return 3;
            }
        }
    }

    closedir(dirp);
    free(re);
 
    return 0;   
}

int load_plugin(char *path)
{
    void    *DL;

    DL = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    if(DL == NULL)
    {
        syslog(LOG_NOTICE, "%s(): dlopen errored: %s", __func__, dlerror());
        return 1;
    }

    /* Store the handle to the plugin */
    NUMPLUGINS++;
    PLUGINS = realloc(PLUGINS, sizeof(void*) * (NUMPLUGINS));
    PLUGINS[NUMPLUGINS - 1] = DL;
    syslog(LOG_INFO, "%s(): module '%s' loaded as plugin #%d", __func__, path, NUMPLUGINS - 1);

    return 0;
}
