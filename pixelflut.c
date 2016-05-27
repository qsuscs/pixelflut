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
	for (int i = x; i < xlen + x; i++) {
		for (int j = y; j < ylen + y; j++) {
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

	while (1) {
		sendblock(sockfd, 0, 0, 333, 35, "E40303");
		sendblock(sockfd, 0, 36, 333, 35, "FF8C00");
		sendblock(sockfd, 0, 71, 333, 35, "FFED00");
		sendblock(sockfd, 0, 106, 333, 35, "008026");
		sendblock(sockfd, 0, 141, 333, 35, "004DFF");
		sendblock(sockfd, 0, 176, 333, 35, "750787");
	}

	close(sockfd);

	return 0;
}
