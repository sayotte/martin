#include <syslog.h>

void syslog(int priority, const char *message, ...)
{
    priority++;
    message++;

    return;
}

