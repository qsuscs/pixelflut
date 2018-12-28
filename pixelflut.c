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

static const char *colors[] = { "E40303", "FF8C00", "FFED00", "008026", "004DFF", "750787" };

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

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <IP> <PORT>\n", argv[0]);
		exit(1);
	}

	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
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

	int width = 388;
	int height = 240;
	int b_height = height / 6;
	int xs = 100;
	int ys = 600;

	int sockets[6] = { 0 };
	char *buffers[6] = { NULL };

	#pragma omp parallel for
	for (int i = 0; i < 6; i++) {
		if ((sockets[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
			exit(1);
		}
		if (connect(sockets[i], server_sin, sizeof(*server_sin)) != 0) {
			fprintf(stderr, "Could not connect: %s\n", strerror(errno));
			exit(1);
		}

		buffers[i] = calloc(1 + width * b_height * snprintf(NULL, 0, "PX %d %d 000000\n", xs + width, ys + height), sizeof(char));
		if (buffers[i] == NULL) {
			fprintf(stderr, "Could not allocate memory\n");
			exit(1);
		}

		int n = 8;
		char *buf_tmp = buffers[i];
		for (int ix = 0; ix < n; ix++) {
			int s, x, y;
			for (x = xs + ix; x < xs + width; x += n) {
				for (int iy = 0; iy < n; iy++) {
					for (y = ys + iy + i * b_height; y < ys + (i + 1) * b_height; y += n) {
						s = sprintf(buffers[i], "PX %d %d %s\n", x, y, colors[i]);
						buffers[i] += s;
					}
				}
			}
		}
		buffers[i] = buf_tmp;
	}

	// sendblock(sockfd, xs, ys, width, height, "000000");

	#pragma omp parallel for
	for (int i = 0; i < 6; i++) {
		while (1) {
			size_t s = strlen(buffers[i]);
			write(sockets[i], buffers[i], s);
		}
	}

	close(sockfd);

	return 0;
}
