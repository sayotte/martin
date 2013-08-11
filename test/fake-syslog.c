#include <syslog.h>

void syslog(int priority, const char *message, ...)
{
    int shutup;
    const char *shutup2;

    shutup = priority;
    shutup2 = message;
    return;
}

