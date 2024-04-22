#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

enum data_type { INT, SHORT_REAL, FLOAT, STRING };

int epoll_add(int epollfd, int fd, uint32_t events);
int epoll_remove(int epollfd, int fd);
void send_msg(int sockfd, char *buf, int len);
int recv_msg(int sockfd, char *buf);
int udp_recv(int sockfd, char *buf, struct sockaddr_in &addr_udp,
			 socklen_t addr_udp_len);
