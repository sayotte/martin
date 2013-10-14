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

int get_static(client_t *c, char **splat, int splat_len)
{
    preamble_t    h;
    off_t       size;
    char        filename[1024];
    char        *response_buf;
    int         error;
    int         fd, fh;
    message_t   *m;

    syslog(LOG_DEBUG, "%s():...", __func__);

    fd = c->fd;
    m = c->msg;

    init_response_preamble(&h);

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

                send_preamble(fd, &h);
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

                send_preamble(fd, &h);
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
    /*** code should use libev to setup a callback, sending as much data as
         possible on each callback, then polling on the write-ability of the
         socket...
         should probably *not* poll on the read-ability of the socket in the
         meantime, lest we start processing another request from this client
         before we finish responding to the last one ***/
    send_preamble(fd, &h);
    send_response_chunk(fd, response_buf, size);
    end_response_chunks(fd);
    munmap(response_buf, size);
    error = 0;

    cleanup:
    close(fh);
    cleanup_response_preamble(&h);
    return error;   
}
