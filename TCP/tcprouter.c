#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>

#define PENDING 1
#define MAXCONN 32
#define PORT 81

void write_all(int fd, const char * c, size_t n) {
	do {
		ssize_t written = write(fd, c, n);
		n -= written;
		if (written == -1) {
			perror("write()");
			exit(-1);
		}
	} while (n > 0);
}

void read_all(int fd, const char *c, size_t n) {
	do {
		ssize_t readin = read(fd, c, n);
		n -= readin;
		if (readin == -1) {
			perror("read()");
			exit(-1)
		}
	} while (n > 0);
}

struct client {
	int active;
	pthread_t thread;
	pthread_mutex_t lock;
	int fd;
	int connect_to;
};

struct client connections * [MAXCONN];

int thread_start(int id) {
	struct pollfd pollfds[2];
	struct client this_connection [2];
	this_connection[0] = connections[id];
	pollfds[0].fd = this_connection[0].fd;
	pollfds[0].events = POLLIN | POLLOUT | POLLERR;
	for(;;) {
		char text [13];
		read_all(this_connection[0]->fd, text, 13);
		if (sscanf(text, "CONNECT TO %d\n", &this_connection[0]->connect_to) > 0)
			break;
	}

	for(;;) {

	}
}

void add_connection(int fd) {
	int id = -1;
	for (int i = 0; i < MAXCONN; i++) {
		if (connections[i] == NULL){
			id = i;
			break;
		} else if (connections[i]->active == 0) {
			pthread_join(connections[i]->thread, NULL);
			free(connections[i])
			id = i;
			break;
		}
	}
	if (id == -1) {
		char msg[] = "The maximum number of clients are connected!\n";
		write_all(fd, msg, strlen(msg));
		return;
	}
	struct client * c = (struct client *) malloc(sizeof(struct client));
	if (c == -1) {
		perror("malloc()");
		exit(-1);
	}

	c->active = 1;
	c->fd = fd;
	c->connect_to = -1;
	c->lock = PTHREAD_MUTEX_INITIALIZER;

	connections[id] = c;

	pthread_create(&c->thread, NULL, &thread_start, &id);
}

int main(int argc, char** argv) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket()");
		exit(-1);
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(81);
	inet_aton("127.0.0.1", &addr.sin_addr, sizeof(struct sockaddr_in));

	if (bind(sockfd, &addr) == -1) {
		perror("bind()");
		exit(-1);
	}
	if (listen(sockfd, PENDING) == -1) {
		perror("listen()");
		exit(-1);
	}

	memset(connections, NULL, sizeof(struct client *) * MAXCONN);

	for(;;) {
		newfd = accept(sockfd, NULL, NULL);
		add_connection(newfd)
	}


}