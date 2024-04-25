#include "utils.hpp"

string data_type_to_string(data_type type) {
	switch (type) {
		case INT:
			return "INT";
		case SHORT_REAL:
			return "SHORT_REAL";
		case FLOAT:
			return "FLOAT";
		case STRING:
			return "STRING";
		default:
			return "UNKNOWN";
	}
}

unsigned short int get_port(char *port_string) {
	try {
		int temp = std::stoi(port_string);

		if (temp < 1024 || temp > 65535) {
			throw -1;
		}
	} catch (...) {
		fprintf(stderr, "Port must be an integer between 1024 and 65535.\n");
		exit(1);
	}

	return (unsigned short)std::stoi(port_string);
}

int epoll_add(int epollfd, int fd, uint32_t events) {
	struct epoll_event ev;

	ev.events = events;
	ev.data.fd = fd;

	return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

int epoll_remove(int epollfd, int fd) {
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = fd;

	return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

int epoll_wait_infinite(int epollfd, struct epoll_event *rev) {
	return epoll_wait(epollfd, rev, 1, EPOLL_TIMEOUT_INFINITE);
}

void send_msg(int sockfd, char *buf, int len) {
	int bytes_sent, total_bytes_sent = 0;

	while (total_bytes_sent < (int)sizeof(int)) {
		bytes_sent = send(sockfd, (char *)&len + total_bytes_sent,
						  sizeof(int) - total_bytes_sent, 0);
		DIE(bytes_sent < 0, "send");

		total_bytes_sent += bytes_sent;
	}

	total_bytes_sent = 0;
	while (total_bytes_sent < len) {
		bytes_sent = send(sockfd, buf + total_bytes_sent, len - total_bytes_sent, 0);
		DIE(bytes_sent < 0, "send");

		total_bytes_sent += bytes_sent;
	}
}

int recv_msg(int sockfd, char *buf) {
	int size, bytes_recv, total_bytes_recv = 0;

	while (total_bytes_recv < (int)sizeof(int)) {
		bytes_recv = recv(sockfd, (char *)&size + total_bytes_recv,
						  sizeof(int) - total_bytes_recv, 0);

		if (!bytes_recv) {
			return 0;
		}
		DIE(bytes_recv < 0, "recv");

		total_bytes_recv += bytes_recv;
	}

	total_bytes_recv = 0;
	while (total_bytes_recv < size) {
		bytes_recv =
			recv(sockfd, buf + total_bytes_recv, size - total_bytes_recv, 0);

		if (!bytes_recv) {
			return 0;
		}
		DIE(bytes_recv < 0, "recv");

		total_bytes_recv += bytes_recv;
	}

	return total_bytes_recv;
}
