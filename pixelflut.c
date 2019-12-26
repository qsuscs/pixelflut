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
#include <getopt.h>

#include "pf_png.h"
#include "socket.h"

static const struct option options_long[] = {
	{ "host", required_argument, NULL, 'h' },
	{ "port", required_argument, NULL, 'p' },
	{ "image", required_argument, NULL, 'i' },
	{ "xpos", required_argument, NULL, 'x' },
	{ "ypos", required_argument, NULL, 'y' },
	{ NULL, 0, NULL, 0 }
};
static const char *optstring = "h:p:i:x:y:";

void usage(const char *name)
{
	fprintf(stderr,
		"Usage: %s [options]\n"
		"\n"
		"-h|--host <host> (required)\n"
		"-p|--port <port> (required)\n"
		"-i|--image <file.png> (required)\n"
		"-x|--xpos <num>\n"
		"-y|--ypos <num>\n",
		name);
}

int main(int argc, char *argv[])
{
	int c;
	char *host = NULL;
	char *service = NULL;
	char *image = NULL;
	int xs = 0, ys = 0;
	while ((c = getopt_long(argc, argv, optstring, options_long, NULL)) !=
	       -1) {
		switch (c) {
		case 'h':
			host = optarg;
			break;
		case 'p':
			service = optarg;
			break;
		case 'i':
			image = optarg;
			break;
		case 'x':
			xs = atoi(optarg);
			break;
		case 'y':
			ys = atoi(optarg);
			break;
		default:
			usage(argv[0]);
			exit(1);
		}
	}
	struct addrinfo hints = { 0, AF_UNSPEC, SOCK_STREAM, 0,
				  0, NULL,	NULL,	     NULL };
	struct addrinfo *res = NULL;
	int err;
	if ((err = getaddrinfo(host, service, &hints, &res))) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		exit(1);
	}

	if (pf_socket_setup(res, 1) < 0) {
		fprintf(stderr, "Error in pf_socket_setup()\n");
		exit(1);
	}

	signal(SIGPIPE, SIG_IGN);

	if (pf_png_open(image)) {
		fprintf(stderr, "Failed to open file\n");
		exit(1);
	}

	if (pf_png_read()) {
		fprintf(stderr, "Failed to read file\n");
		exit(1);
	}

	char *buffer =
		calloc(1 + pf_png_height() * pf_png_width() *
				       snprintf(NULL, 0, "PX %d %d 000000\n",
						xs + pf_png_width(),
						ys + pf_png_height()),
		       sizeof(char));
	if (buffer == NULL) {
		fprintf(stderr, "Could not allocate memory\n");
		exit(1);
	}

	char *p = buffer;
	for (unsigned int x = 0; x < pf_png_width(); x++) {
		for (unsigned int y = 0; y < pf_png_height(); y++) {
			int s = sprintf(p, "PX %d %d %06x\n", x + xs, y + ys,
					pf_png_get_rgb(x, y));
			p += s;
		}
	}

	if (pf_png_close()) {
		fprintf(stderr, "Could not close file\n");
		exit(1);
	}

	size_t s = strlen(buffer);

	pf_socket_loop_write(buffer, s);
	pf_socket_destroy();

	return 0;
}
