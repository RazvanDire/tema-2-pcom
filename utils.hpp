#ifndef UTILS_HPP
#define UTILS_HPP

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "debug.hpp"

using namespace std;

#define BUFLEN 1500
#define EPOLL_TIMEOUT_INFINITE -1
#define MAX_CONNECTIONS 1024

enum data_type : uint8_t { INT, SHORT_REAL, FLOAT, STRING };

struct message {
	char topic[50];
	data_type type;
	char payload[1500];
} __attribute__((packed));

string data_type_to_string(data_type type);
unsigned short int get_port(char *port_string);
int epoll_add(int epollfd, int fd, uint32_t events);
int epoll_remove(int epollfd, int fd);
int epoll_wait_infinite(int epollfd, struct epoll_event *rev);
void send_msg(int sockfd, char *buf, int len);
int recv_msg(int sockfd, char *buf);

#endif	// UTILS_HPP