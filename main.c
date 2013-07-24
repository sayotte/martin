#include "server.h"

int main(int argc, char** argv)
{
    init_logging();
    return(start_server());
}

