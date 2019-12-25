#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#include "pf_png.h"

int sockets[6] = { 0 };

void sendpx(int fd, int x, int y, const char *color)
{
	dprintf(fd, "PX %d %d %s\n", x, y, color);
}

void sendblock(int fd, int x, int y, int xlen, int ylen, const char *color)
{
	for (int j = y; j < ylen + y; j++) {
		for (int i = x; i < xlen + x; i++) {
			sendpx(fd, i, j, color);
		}
	}
}

void setup_socket(int i, const struct sockaddr_in *server_sin)
{
	if (sockets[i] != 0) {
		close(sockets[i]);
	}
	if ((sockets[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Failed to create socket: %s\n",
			strerror(errno));
		exit(1);
	}
	if (connect(sockets[i], server_sin, sizeof(*server_sin)) != 0) {
		fprintf(stderr, "Could not connect: %s\n", strerror(errno));
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <IP> <PORT> <FILE>\n", argv[0]);
		exit(1);
	}

	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Failed to create socket: %s\n",
			strerror(errno));
		exit(1);
	}

	int portno = atoi(argv[2]);

	struct in_addr *server_ina = malloc(sizeof(struct in_addr));
	if (inet_pton(AF_INET, argv[1], server_ina) != 1) {
		fprintf(stderr, "Could not parse address %s\n", argv[1]);
		exit(1);
	}

	struct sockaddr_in *server_sin = calloc(1, sizeof(struct sockaddr_in));
	server_sin->sin_family = AF_INET;
	server_sin->sin_port = htons(portno);
	server_sin->sin_addr = *server_ina;

	if (connect(sockfd, server_sin, sizeof(*server_sin)) != 0) {
		fprintf(stderr, "Could not connect: %s\n", strerror(errno));
		exit(1);
	}

	signal(SIGPIPE, SIG_IGN);

	if (pf_png_open(argv[3])) {
		fprintf(stderr, "Failed to open file\n");
		exit(1);
	}

	if (pf_png_read()) {
		fprintf(stderr, "Failed to read file\n");
		exit(1);
	}

	int xs = 100;
	int ys = 100;

	char *buffers[6] = { NULL };

	for (int i = 0; i < 1; i++) {
		setup_socket(i, server_sin);

		buffers[i] = calloc(
			1 + pf_png_height() * pf_png_width() *
					snprintf(NULL, 0, "PX %d %d 000000\n",
						 xs + pf_png_width(),
						 ys + pf_png_height()),
			sizeof(char));
		if (buffers[i] == NULL) {
			fprintf(stderr, "Could not allocate memory\n");
			exit(1);
		}

		char *buf_tmp = buffers[i];
		for (unsigned int x = 0; x < pf_png_width(); x++) {
			for (unsigned int y = 0; y < pf_png_height(); y++) {
				int s = sprintf(buffers[i], "PX %d %d %06x\n",
						x + xs, y + ys,
						pf_png_get_rgb(x, y));
				buffers[i] += s;
			}
		}
		buffers[i] = buf_tmp;
	}

	// sendblock(sockfd, xs, ys, pf_png_width(), pf_png_height(), "000000");

	if (pf_png_close()) {
		fprintf(stderr, "Could not close file\n");
		exit(1);
	}

	while (1) {
		for (int i = 0; i < 1; i++) {
			size_t s = strlen(buffers[i]);
			if (write(sockets[i], buffers[i], s) == 0) {
				setup_socket(i, server_sin);
			}
		}
	}

	close(sockfd);

	return 0;
}
