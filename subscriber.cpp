#include "utils.hpp"

int sockfd;
int epollfd;

void close_fds() {
	close(sockfd);
	close(epollfd);
}

int connect_to_server(const char *server_ip, const char *server_port) {
	int rc;
	int sock_opt;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	memset(&addr, 0, addrlen);

	sock_opt = 1;
	rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int));
	DIE(rc < 0, "setsockopt SO_REUSEADDR");

	// disable Nagle's algorithm
	sock_opt = 1;
	rc = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &sock_opt, sizeof(int));
	DIE(rc < 0, "setsockopt TCP_NODELAY");

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(server_port));

	rc = inet_aton(server_ip, &addr.sin_addr);
	DIE(rc == 0, "inet_aton");

	rc = connect(sockfd, (struct sockaddr *)&addr, addrlen);
	DIE(rc < 0, "connect");

	return sockfd;
}

void subscribe() {
	string topic;
	cin >> topic;

	char buf[BUFLEN];
	strcpy(buf, "subscribe ");
	strcat(buf, topic.c_str());

	send_msg(sockfd, buf, strlen(buf) + 1);

	cout << "Subscribed to topic " << topic << endl;
}

void unsubscribe() {
	string topic;
	cin >> topic;

	char buf[BUFLEN];
	strcpy(buf, "unsubscribe ");
	strcat(buf, topic.c_str());

	send_msg(sockfd, buf, strlen(buf) + 1);

	cout << "Unsubscribed from topic " << topic << endl;
}

void handle_user_input() {
	string cmd;

	cin >> cmd;

	if (cmd == "exit") {
		close_fds();
		exit(0);
	}

	if (cmd == "subscribe") {
		subscribe();
	} else if (cmd == "unsubscribe") {
		unsubscribe();
	} else {
		cerr << "Invalid command.\n";
	}
}

void print_payload(message buf) {
	uint8_t byte;
	uint16_t short_real;
	uint32_t integer;
	int base = 1;
	uint8_t decimals, i;

	switch (buf.type) {
		case INT:
			memcpy(&byte, buf.payload, sizeof(uint8_t));
			memcpy(&integer, buf.payload + sizeof(uint8_t), sizeof(uint32_t));
			integer = ntohl(integer);

			if (byte && integer) {
				cout << "-";
			}
			
			cout << integer << endl;

			break;

		case SHORT_REAL:
			memcpy(&short_real, buf.payload, sizeof(uint16_t));
			short_real = ntohs(short_real);
			// cout << short_real / 100 << "." << short_real % 100 << endl;
			cout << fixed << setprecision(2) << (double)short_real / 100
				 << endl;

			break;

		case FLOAT:
			memcpy(&byte, buf.payload, sizeof(uint8_t));
			memcpy(&integer, buf.payload + sizeof(uint8_t), sizeof(uint32_t));
			integer = ntohl(integer);

			if (byte && integer) {
				cout << "-";
			}

			memcpy(&decimals, buf.payload + sizeof(uint8_t) + sizeof(uint32_t),
				   sizeof(uint8_t));
			for (i = 0; i < decimals; i++) {
				base *= 10;
			}
			// cout << integer / base << "." << integer % base << endl;
			cout << fixed << setprecision(decimals) << (double)integer / base
				 << endl;

			break;

		case STRING:
			cout << buf.payload << endl;
			break;

		default:
			cout << endl;
	}
}

void handle_server_input() {
	int len;
	sockaddr_in addr;

	len = recv_msg(sockfd, (char *)&addr);
	DIE(len < 0, "recv");

	if (len == 0) {
		close_fds();
		exit(0);
	}

	message buf;
	len = recv_msg(sockfd, (char *)&buf);
	DIE(len < 0, "recv");

	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << " - "
		 << buf.topic << " - " << data_type_to_string(buf.type) << " - ";
	print_payload(buf);
}

int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	if (argc != 4) {
		cerr << "Usage: " << argv[0]
			 << " <client_id> <server_ip> <server_port>\n";
		return 1;
	}

	if (strlen(argv[1]) > 10) {
		cerr << "Client ID too long.\n";
		return 1;
	}

	sockfd = connect_to_server(argv[2], argv[3]);
	send_msg(sockfd, argv[1], strlen(argv[1]) + 1);

	epollfd = epoll_create1(0);
	epoll_add(epollfd, STDIN_FILENO, EPOLLIN);
	epoll_add(epollfd, sockfd, EPOLLIN);

	while (true) {
		struct epoll_event rev;
		int rc = epoll_wait_infinite(epollfd, &rev);
		DIE(rc < 0, "epoll_wait");

		if (rev.data.fd == STDIN_FILENO) {
			handle_user_input();
		} else if (rev.data.fd == sockfd) {
			handle_server_input();
		}
	}
}
