#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h> /* not needed when printf is removed */
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include "response.h"
#include "request.h"
#include "util.h"

extern const char* r403_template;
extern const char* r404_template;

int put_transaction(int fd, struct message *m, char **splat, int splat_len)
{
    int     i;
    syslog(LOG_DEBUG, "%s():...", __func__);

    for(i = 0; i < splat_len; i++)
    {
        printf("splat[%d]: '%s'\n", i, splat[i]);
    }
    return 0;
}

int get_all_transactions(int fd, struct message *m, char **splat, int splat_len)
{
    int     i;
    syslog(LOG_DEBUG, "%s():...", __func__);

    for(i = 0; i < splat_len; i++)
    {
        printf("splat[%d]: '%s'\n", i, splat[i]);
    }
    return 0;
}

int get_transaction(int fd, struct message *m, char **splat, int splat_len)
{
    header_t    h;

    syslog(LOG_DEBUG, "%s():...", __func__);

    init_response_header(&h);
    add_response_header(&h, "Connection: close");

/*    h.content_length = strlen(splat[1]) + 2;
*/
    send_header(fd, &h);

/*    write(fd, splat[1], strlen(splat[1]));
    write(fd, "\r\n", 2);
*/
    send_response_chunk(fd, splat[1], strlen(splat[1]));
    send_response_chunk(fd, "\r\n", 2);
    end_response_chunks(fd);

    cleanup_response_header(&h);

    return 0;
}

int get_static(int fd, struct message *m, char **splat, int splat_len)
{
    header_t    h;
    off_t       size;
    char        filename[1024];
    char        *response_buf;
    int         error;
    int         fh;

    syslog(LOG_DEBUG, "%s():...", __func__);

    init_response_header(&h);

    snprintf(filename, 1024, "./static/%s", splat[1]);
    error = access(filename, R_OK);
    if(error == -1)
    {
        switch(errno){
            case EACCES:
                size = strlen(filename) + strlen(r403_template) + 1;
                response_buf = malloc(size);
                snprintf(response_buf, size, r403_template, splat[0]);
                size = strlen(response_buf);
                h.status = 403;
                h.content_type = "text/html";
                h.status_desc = "Forbidden";
                h.content_length = size;

                send_header(fd, &h);
                write(fd, response_buf, size);
                free(response_buf);

                error = 0;
            default:
                size = strlen(filename) + strlen(r404_template) + 1;
                response_buf = malloc(size);
                snprintf(response_buf, size, r404_template, splat[0]);
                size = strlen(response_buf);
                h.status = 404;
                h.content_type = "text/html";
                h.status_desc = "Not Found";
                h.content_length = size;

                send_header(fd, &h);
                write(fd, response_buf, size);
                free(response_buf);

                error = 0;
        }
        goto cleanup;
    }

    size = fsize(filename);
    fh = open(filename, O_RDONLY);
    if(fh < 0)
    {
        syslog(LOG_ERR, "%s(): Failed to open file %s: %s", __func__, filename, strerror(errno));
        error = -1;
        goto cleanup;
    }
    response_buf = mmap(0, (int)size, PROT_READ, MAP_FILE|MAP_PRIVATE, fh, 0);
    if(response_buf == MAP_FAILED)
    {
        syslog(LOG_ERR, "%s(): mmap() failed on file %s (fd #%d): %s", __func__, filename, fh, strerror(errno));
        error = -2;
        goto cleanup;
    }
    /*** FIXME add code to send smaller chunks here ***/
    send_header(fd, &h);
    send_response_chunk(fd, response_buf, size);
    end_response_chunks(fd);
    munmap(response_buf, size);
    error = 0;

    cleanup:
    close(fh);
    cleanup_response_header(&h);
    return error;   
}
