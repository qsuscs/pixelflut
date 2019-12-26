#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* returns 0 on success, -1 otherwise */
int pf_socket_setup(const struct addrinfo *addr, int nsock);

/* as write(2) */
int pf_socket_write_any(const void *p, size_t size);

/* shouldn’t return */
void pf_socket_loop_write(const void *p, size_t size);

/* as close(2) */
int pf_socket_destroy(void);
