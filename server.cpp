#include "server.hpp"

#include "debug.hpp"
#include "utils.hpp"

int epollfd;
int tcp_listenfd;
int udp_listenfd;
unsigned short port;

void init_listeners() {
	tcp_listenfd = tcp_create_listener(port, 5);
	udp_listenfd = udp_create_listener(port);
}

void init_epoll() {
	epollfd = epoll_create1(0);
	DIE(epollfd < 0, "epoll_create1");

	epoll_add(epollfd, tcp_listenfd, EPOLLIN);
	epoll_add(epollfd, udp_listenfd, EPOLLIN);
	epoll_add(epollfd, STDIN_FILENO, EPOLLIN);
}

void close_listeners() {
	close(tcp_listenfd);
	close(udp_listenfd);
	close(epollfd);
}

void set_port(char* port_string) {
	try {
		int temp = std::stoi(port_string);

		if (temp < 1024 || temp > 65535) {
			throw -1;
		}
	} catch (...) {
		fprintf(stderr, "Port must be an integer between 1024 and 65535.\n");
		exit(1);
	}

	port = (unsigned short)std::stoi(port_string);
}

int main(int argc, char* argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		return 1;
	}

	set_port(argv[1]);
	init_listeners();
	init_epoll();

	// Server server(argv[1]);
	// server.run();

	close_listeners();

	return 0;
}
