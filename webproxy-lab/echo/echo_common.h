#ifndef ECHO_COMMON_H
#define ECHO_COMMON_H

#include "../csapp.h"

// Initialize common module state (no-op placeholder for now).
void echo_common_init(void);

// Read one line from rio into buf. Returns bytes read, 0 on EOF, or <0 on error.
ssize_t echo_read_line(rio_t *rio, char *buf, size_t n);

// Send exactly n bytes over a connection descriptor.
void echo_write_line(int fd, const char *buf, size_t n);

// Read one request line and immediately echo it back.
// Returns number of bytes echoed, 0 on EOF, or <0 on error.
ssize_t echo_handle_one_line(int connfd, rio_t *rio, char *buf, size_t n);

#endif
