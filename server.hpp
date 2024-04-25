#include "utils.hpp"

int tcp_create_listener(unsigned short port, int backlog);
int udp_create_listener(unsigned short port);
int recv_udp(int sockfd, char *buf, int len, struct sockaddr_in *addr,
			 unsigned int addrlen);