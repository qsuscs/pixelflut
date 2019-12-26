#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"

static int sock;

/* returns 0 on success, -1 otherwise */
int pf_socket_setup(const struct addrinfo *addr, int nsock)
{
	if (nsock != 1) {
		return -1;
	}
	if ((sock = socket(addr->ai_family, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Failed to create socket: %s\n",
			strerror(errno));
		return -1;
	}

	if (connect(sock, addr->ai_addr, addr->ai_addrlen) != 0) {
		fprintf(stderr, "Could not connect: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

/* as write(2) */
int pf_socket_write_any(const void *p, size_t size)
{
	return write(sock, p, size);
}

/* shouldn’t return unless in case of error */
void pf_socket_loop_write(const void *p, size_t size)
{
	while (1) {
		if (pf_socket_write_any(p, size) == 0) {
			break;
		}
	}
}

/* as close(2) */
int pf_socket_destroy(void)
{
	int ret = close(sock);
	sock = -1;
	return ret;
}
