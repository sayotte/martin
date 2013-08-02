#include <sys/types.h>

int handle_client(int client);
void init_logging();
int start_server();
int setup_listen_socket(int *fd);

#define MAX_WORKERS 4
#define MAX_CONNS_PROCESSED 10
