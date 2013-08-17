typedef struct response_preamble {
    int     status; /* required; one of 1xx - 5xx */
    char    *status_desc; /* e.g. "OK" for status code 200 */
    int     content_length; /* required for 200 OK non-chunked; set to 0 if we expect to use chunked encoding */
    char    *content_type; /* required for 200 OK */
    char    *resource; /* required for 404: resource that was requested */
    char    *method_name; /* required for 501: the method used to request the resource, e.g. HTTP_GET */
    char    **optional_headers;
    int     num_headers;
} preamble_t;

int send_preamble(int client, preamble_t *h);
void init_response_preamble(preamble_t *h);
void cleanup_response_preamble(preamble_t *h);
void add_response_header(preamble_t *h, char* hdr);

int send_response_chunk(int client, char *buf, int size);
int end_response_chunks(int client);
