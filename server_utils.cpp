#include "debug.hpp"
#include "server.hpp"
#include "utils.hpp"

int tcp_create_listener(unsigned short port, int backlog) {
	struct sockaddr_in address;
	int listenfd;
	int sock_opt;
	int rc;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(listenfd < 0, "socket");

	sock_opt = 1;
	rc = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int));
	DIE(rc < 0, "setsockopt SO_REUSEADDR");

	sock_opt = 1;
	rc = setsockopt(listenfd, IPPROTO_TCP, TCP_NODELAY, &sock_opt, sizeof(int));
	DIE(rc < 0, "setsockopt TCP_NODELAY");

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	rc = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
	DIE(rc < 0, "bind");

	rc = listen(listenfd, backlog);
	DIE(rc < 0, "listen");

	return listenfd;
}

int udp_create_listener(unsigned short port) {
	struct sockaddr_in address;
	int listenfd;
	int sock_opt;
	int rc;

	listenfd = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(listenfd < 0, "socket");

	sock_opt = 1;
	rc = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int));
	DIE(rc < 0, "setsockopt SO_REUSEADDR");

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	rc = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
	DIE(rc < 0, "bind");

	return listenfd;
}