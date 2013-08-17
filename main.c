#include "server.h"
#include <signal.h>

int main(int argc, char** argv)
{
    init_logging();

    signal(SIGPIPE, SIG_IGN);

    return(start_server());
}

