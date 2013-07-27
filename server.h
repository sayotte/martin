#include <sys/types.h>
typedef struct master_socket {
    int             fd;
    pthread_mutex_t *mtx;
} master_socket_t;

int handle_client(int client);
void init_logging();
int start_server();
pthread_mutex_t *setup_shared_mutex();
int setup_listen_socket(master_socket_t *ms);
int worker_loop(master_socket_t *ms);

#define MAX_WORKERS 4
#define MAX_CONNS_PROCESSED 10
