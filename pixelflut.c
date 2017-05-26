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

size_t n;

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

	int width = 333;
	int height = 206;
	int b_height = height / 6;
	int xs = 1200;
	int ys = 600;

	sendblock(sockfd, xs, ys, width, height, "000000");
	while (1) {
		sendblock(sockfd, xs, ys, width, b_height, "E40303");
		sendblock(sockfd, xs, ys+b_height, width, b_height, "FF8C00");
		sendblock(sockfd, xs, ys+b_height*2, width, b_height, "FFED00");
		sendblock(sockfd, xs, ys+b_height*3, width, b_height, "008026");
		sendblock(sockfd, xs, ys+b_height*4, width, b_height, "004DFF");
		sendblock(sockfd, xs, ys+b_height*5, width, b_height, "750787");
	}

	close(sockfd);

	return 0;
}
