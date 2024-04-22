#include "tcp_client.hpp"

#include "debug.hpp"
#include "utils.hpp"

int connect_to_server(const char* server_ip, const char* server_port) {
	int rc;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(server_port));

	rc = inet_aton(server_ip, &addr.sin_addr);
	DIE(rc == 0, "inet_aton");

	rc = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	DIE(rc < 0, "connect");

	return sockfd;
}

int main(int argc, char* argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <client_id> <server_ip> <server_port>\n",
				argv[0]);
		return 1;
	}

	char buf[1500];

	while (true) {
		fgets(buf, sizeof(buf), stdin);

		if (!strncmp(buf, "exit", 4)) {
			break;
		}

		if (true) {
		} else if (true) {
		} else {
			fprintf(stderr, "Invalid command.\n");
			continue;
		}

	
	}
}
