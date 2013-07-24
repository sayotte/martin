typedef struct response_header {
    int     status; /* required; one of 1xx - 5xx */
    char    *status_desc; /* e.g. "OK" for status code 200 */
    int     content_length; /* required for 200 OK non-chunked; set to 0 if we expect to use chunked encoding */
    char    *content_type; /* required for 200 OK */
    char    *resource; /* required for 404: resource that was requested */
    char    *method_name; /* required for 501: the method used to request the resource, e.g. HTTP_GET */
    char    **optional_headers;
    int     num_headers;
} header_t;

void send_header(int client, header_t *h);
void init_response_header(header_t *h);
void cleanup_response_header(header_t *h);
void add_response_header(header_t *h, char* hdr);

void send_response_chunk(int client, char *buf, int size);
void end_response_chunks(int client);
