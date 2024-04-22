#include "utils.hpp"

#include "debug.hpp"

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

void send_msg(int sockfd, char *buf, int len) {
	ssize_t bytes_sent, total_bytes_sent = 0;

	while (total_bytes_sent < sizeof(int)) {
		bytes_sent =
			send(sockfd, &len + total_bytes_sent, sizeof(int) - bytes_sent, 0);
		DIE(bytes_sent < 0, "send ");
		total_bytes_sent += bytes_sent;
	}

	total_bytes_sent = 0;
	while (total_bytes_sent < len) {
		bytes_sent = send(sockfd, buf + total_bytes_sent, len - bytes_sent, 0);
		DIE(bytes_sent < 0, "send");
		total_bytes_sent += bytes_sent;
	}
}

int recv_msg(int sockfd, char *buf) {
	ssize_t size, bytes_recv, total_bytes_recv = 0;

	while (total_bytes_recv < sizeof(int)) {
		bytes_recv = recv(sockfd, &size + total_bytes_recv,
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

int udp_recv(int sockfd, char *buf, struct sockaddr_in &addr_udp,
			 socklen_t addr_udp_len) {
	int bytes = recvfrom(sockfd, buf, addr_udp_len, 0,
						 (struct sockaddr *)&addr_udp, &addr_udp_len);
	if (!bytes) {
		return 0;
	}
	DIE(bytes < 0, "recv");

	return bytes;
}
