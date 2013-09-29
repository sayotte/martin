void gen_date_header(char* dst);
off_t fsize(const char* filename);
int chomp(char *str);
int match_http_method(char  *string);

#define write_attempt(status, fd, buf, len) \
    status = write(fd, buf, len); \
    if(status == -1) \
    { \
        syslog(LOG_INFO, "%s(): write() returned error: %s", __func__, strerror(errno)); \
        return -1;  \
    }

