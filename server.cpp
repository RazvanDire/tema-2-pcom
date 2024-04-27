#include "server.hpp"

int epollfd;
int tcp_listenfd;
int udp_listenfd;
unsigned short port;

vector<pair<string, unordered_set<string>>> topics;
unordered_map<int, string> sockfd_to_id;
unordered_map<string, int> id_to_sockfd;
unordered_set<string> connected_clients;

void init_listeners() {
	tcp_listenfd = tcp_create_listener(port, MAX_CONNECTIONS);
	udp_listenfd = udp_create_listener(port);
}

void init_epoll() {
	int rc;

	epollfd = epoll_create1(0);
	DIE(epollfd < 0, "epoll_create1");

	rc = epoll_add(epollfd, tcp_listenfd, EPOLLIN);
	DIE(rc < 0, "epoll add tcp");

	rc = epoll_add(epollfd, udp_listenfd, EPOLLIN);
	DIE(rc < 0, "epoll add udp");

	rc = epoll_add(epollfd, STDIN_FILENO, EPOLLIN);
	DIE(rc < 0, "epoll add stdin");
}

void close_fds() {
	close(tcp_listenfd);
	close(udp_listenfd);
	close(epollfd);
}

void handle_stdin() {
	string cmd;
	cin >> cmd;

	if (cmd == "exit") {
		for (auto it = sockfd_to_id.begin(); it != sockfd_to_id.end(); it++) {
			epoll_remove(epollfd, it->first);
			close(it->first);
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

	char id_array[11];
	recv_msg(sockfd, id_array);

	string id(id_array);

	if (connected_clients.find(id) != connected_clients.end()) {
		close(sockfd);
		cout << "Client " << id << " already connected.\n";

		return;
	}

	rc = epoll_add(epollfd, sockfd, EPOLLIN);
	DIE(rc < 0, "epoll add");

	connected_clients.insert(id);
	sockfd_to_id[sockfd] = id;
	id_to_sockfd[id] = sockfd;

	cout << "New client " << id << " connected from "
		 << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << "."
		 << endl;
}

void handle_udp() {
	message buf;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	int rc;

	rc = recv_udp(udp_listenfd, (char *)&buf, sizeof(message), &addr, addrlen);
	DIE(rc < 0, "recv_udp");

	string topic(buf.topic);

	unordered_set<string> sent_to;
	for (unsigned int i = 0; i < topics.size(); i++) {
		if (match_topic(topics[i].first, topic)) {
			for (auto it = topics[i].second.begin();
				 it != topics[i].second.end(); it++) {
				if (connected_clients.find(*it) == connected_clients.end() ||
					sent_to.find(*it) != sent_to.end()) {
					continue;
				}

				int sockfd = id_to_sockfd[*it];
				send_msg(sockfd, (char *)&addr, sizeof(struct sockaddr_in));
				send_msg(sockfd, (char *)&buf, sizeof(message));

				sent_to.insert(*it);
			}
		}
	}
}

void subscribe_client(string id, char *topic_array) {
	string topic(topic_array);

	for (unsigned int i = 0; i < topics.size(); i++) {
		if (topics[i].first == topic) {
			topics[i].second.insert(id);
			return;
		}
	}

	topics.push_back({topic, {id}});
}

void unsubscribe_client(string id, char *topic_array) {
	string topic(topic_array);

	for (unsigned int i = 0; i < topics.size(); i++) {
		if (match_topic(topic, topics[i].first)) {
			topics[i].second.erase(id);
		}
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

		connected_clients.erase(sockfd_to_id[sockfd]);
		id_to_sockfd.erase(sockfd_to_id[sockfd]);
		sockfd_to_id.erase(sockfd);

		return;
	}

	if (buf[0] == 's') {
		subscribe_client(sockfd_to_id[sockfd], buf + 10);
	} else if (buf[0] == 'u') {
		unsubscribe_client(sockfd_to_id[sockfd], buf + 12);
	}
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
