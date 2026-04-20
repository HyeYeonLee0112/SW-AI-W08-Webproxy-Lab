#include <stdio.h>
#include "echo_common.h"

void echo_common_init(void)
{
    // no global state needed for this lab
    return;
}

ssize_t echo_read_line(rio_t *rio, char *buf, size_t n)
{
    return Rio_readlineb(rio, buf, (int)n);
}

void echo_write_line(int fd, const char *buf, size_t n)
{
    if (n == 0) return;
    Rio_writen(fd, (void *)buf, n);
}

ssize_t echo_handle_one_line(int connfd, rio_t *rio, char *buf, size_t n)
{
    ssize_t rc = echo_read_line(rio, buf, n);
    if (rc <= 0)
        return rc;

    echo_write_line(connfd, buf, (size_t)rc);
    return rc;
}
