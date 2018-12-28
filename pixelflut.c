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

//	signal(SIGPIPE, SIG_IGN);

	int width = 777;
	int height = 480;
	int b_height = height / 6;
	int xs = 1200;
	int ys = 600;

	char *buf = calloc(1 + width * height * snprintf(NULL, 0, "PX %d %d 000000\n", xs + width, ys + height), sizeof(char));
	if (buf == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		exit(1);
	}
	char *buf_orig = buf;
	int n = 8;
	for (int ix = 0; ix < n; ix++) {
		int s, x, y;
		char *str = NULL;
		for (x = xs + ix; x < xs + width; x += n) {
			for (int iy = 0; iy < n; iy++) {
				for (y = ys + iy; y < ys + height; y += n) {
					switch ((y - ys) / b_height) {
					case 0:
						str = "E40303";
						break;
					case 1:
						str = "FF8C00";
						break;
					case 2:
						str = "FFED00";
						break;
					case 3:
						str = "008026";
						break;
					case 4:
						str = "004DFF";
						break;
					case 5:
						str = "750787";
						break;
					default:
						exit(42);
					}
					s = sprintf(buf, "PX %d %d %s\n", x, y, str);
					buf += s;
				}
			}
		}
	}

	sendblock(sockfd, xs, ys, width, height, "000000");
	while (1) {
#if 0
		sendblock(sockfd, xs, ys, width, b_height, "E40303");
		sendblock(sockfd, xs, ys+b_height, width, b_height, "FF8C00");
		sendblock(sockfd, xs, ys+b_height*2, width, b_height, "FFED00");
		sendblock(sockfd, xs, ys+b_height*3, width, b_height, "008026");
		sendblock(sockfd, xs, ys+b_height*4, width, b_height, "004DFF");
		sendblock(sockfd, xs, ys+b_height*5, width, b_height, "750787");
#endif
		size_t size = buf - buf_orig;
		write(sockfd, buf_orig, size);
	}

	close(sockfd);

	return 0;
}
