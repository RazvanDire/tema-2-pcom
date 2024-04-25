#include "server.hpp"

int epollfd;
int tcp_listenfd;
int udp_listenfd;
unsigned short port;
unordered_map<int, string> sockfd_to_id;
unordered_map<string, int> id_to_sockfd;
unordered_set<string> ids;
unordered_map<string, unordered_set<string>> topics;
unordered_map<string, unordered_set<string>> wildcards;

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

void close_fds() {
	close(tcp_listenfd);
	close(udp_listenfd);
	close(epollfd);
}

void handle_stdin() {
	char buf[BUFLEN];

	fgets(buf, BUFLEN, stdin);
	buf[strlen(buf) - 1] = '\0';

	if (!strcmp(buf, "exit")) {
		for (auto it = sockfd_to_id.begin(); it != sockfd_to_id.end(); it++) {
			close(it->first);
			epoll_remove(epollfd, it->first);
		}

		close_fds();
		exit(0);
	} else {
		cerr << "Invalid command.\n";
	}
}

void handle_new_tcp_connection() {
	static int sockfd;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	int rc;

	/* Accept new connection. */
	sockfd = accept(tcp_listenfd, (struct sockaddr *)&addr, &addrlen);
	DIE(sockfd < 0, "accept");

	rc = epoll_add(epollfd, sockfd, EPOLLIN);
	DIE(rc < 0, "epoll add");

	char id_array[11];
	recv_msg(sockfd, id_array);

	string id(id_array);

	if (ids.find(id) != ids.end()) {
		cout << "Client " << id << " already connected.\n";
		close(sockfd);
		epoll_remove(epollfd, sockfd);
		return;
	}

	ids.insert(id);
	sockfd_to_id[sockfd] = id;
	id_to_sockfd[id] = sockfd;

	cout << "New client " << id << " connected from "
		 << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "."
		 << endl;
}

void search_topics(message buf, struct sockaddr_in addr) {
	string topic(buf.topic);

	if (topics.find(topic) == topics.end()) {
		return;
	}

	for (auto it = topics[topic].begin(); it != topics[topic].end(); it++) {
		int sockfd = id_to_sockfd[*it];
		send_msg(sockfd, (char *)&addr, sizeof(struct sockaddr_in));
		send_msg(sockfd, (char *)&buf, sizeof(message));
	}
}

void search_wildcards(message buf, struct sockaddr_in addr) {
	// if (topics.find(topic) != topics.end()) {
	// 	for (auto it = topics[topic].begin(); it != topics[topic].end(); it++) {
	// 		int sockfd = id_to_sockfd[*it];
	// 		send_msg(sockfd, message.c_str());
	// 	}
	// }

	// if (wildcards.find(topic) != wildcards.end()) {
	// 	for (auto it = wildcards[topic].begin(); it != wildcards[topic].end();
	// it++) { 		int sockfd = id_to_sockfd[*it]; 		send_msg(sockfd,
	// message.c_str());
	// 	}
	// }
}

void handle_udp() {
	message buf;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	int rc;

	rc = recv_udp(udp_listenfd, (char *)&buf, sizeof(message), &addr, addrlen);
	DIE(rc < 0, "recv_udp");

	// cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << " - "
	// 	 << buf.topic << " - " << buf.payload << '\n';

	if (strchr(buf.topic, '*') || strchr(buf.topic, '+')) {
		search_wildcards(buf, addr);
	} else {
		search_topics(buf, addr);
	}
}

void handle_client(int sockfd) {
	char buf[BUFLEN];
	int rc;

	memset(buf, 0, BUFLEN);
	rc = recv_msg(sockfd, buf);
	DIE(rc < 0, "recv");

	if (rc == 0) {
		close(sockfd);
		epoll_remove(epollfd, sockfd);
		cout << "Client " << sockfd_to_id[sockfd] << " disconnected.\n";
		ids.erase(sockfd_to_id[sockfd]);
		id_to_sockfd.erase(sockfd_to_id[sockfd]);
		sockfd_to_id.erase(sockfd);
		return;
	}

	cout << "Received: " << buf << '\n';
}

int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " <port>\n";
		return 1;
	}

	port = get_port(argv[1]);
	init_listeners();
	init_epoll();

	topics["a_non_negative_int"].insert("salut");
	topics["a_negative_int"].insert("salut");
	topics["a_larger_value"].insert("salut");
	topics["that_is_small_short_real"].insert("salut");
	topics["a_negative_subunitary_float"].insert("salut");
	topics["ana_string_announce"].insert("salut");

	while (true) {
		struct epoll_event rev;
		int rc = epoll_wait_infinite(epollfd, &rev);
		DIE(rc < 0, "epoll wait");

		if (rev.data.fd == STDIN_FILENO) {
			handle_stdin();
		} else if (rev.data.fd == tcp_listenfd) {
			handle_new_tcp_connection();
		} else if (rev.data.fd == udp_listenfd) {
			handle_udp();
		} else {
			handle_client(rev.data.fd);
		}
	}

	return 0;
}
