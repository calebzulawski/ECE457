#include <pthread.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PENDING  1
#define MAXCONN  10
#define PORT     1234
#define BUFFSIZE 4096

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

void read_all(int fd, char *c, size_t n) {
	do {
		ssize_t readin = read(fd, c, n);
		n -= readin;
		if (readin == -1) {
			perror("read()");
			exit(-1);
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

struct client * connections [MAXCONN];

static void * thread_start(void * arg) {
	struct client * this_connection [2];
	this_connection[0] = arg;


	for(;;) {
		char text [13];
		read_all(this_connection[0]->fd, text, 13);
		if (sscanf(text, "CONNECT TO %d\n", &this_connection[0]->connect_to) > 0)
			break;
	}

	this_connection[1] = connections[this_connection[0]->connect_to];

	struct pollfd pollfds[2];
	pollfds[0].fd = this_connection[0]->fd;
	pollfds[0].events = POLLIN | POLLERR;
	pollfds[1].fd = this_connection[1]->fd;
	pollfds[1].events = POLLOUT | POLLERR;

	printf("Routing to %d\n", this_connection[0]->connect_to);

	char buffer [BUFFSIZE];
	for(;;) {
		int ret = poll(pollfds, 2, 1000);
		if (ret == -1) {
			perror("poll()");
			this_connection[0]->active = 0;
			pthread_exit(NULL);
		} else if (ret == 0) {
			continue;
		}
		
		if (pollfds[0].revents & POLLERR || pollfds[1].revents & POLLERR) {
			printf("Dropping\n");
			this_connection[0]->active = 0;
			pthread_exit(NULL);
		} else if (pollfds[0].revents && pollfds[1].revents) {
			pollfds[0].revents = 0;
			pollfds[0].revents = 0;
			ssize_t len = recv(pollfds[0].fd, buffer, BUFFSIZE, MSG_DONTWAIT);
			if (len == -1) {
				perror("recv()");
				this_connection[0]->active = 0;
				pthread_exit(NULL);
			} else if (len > 0) {
				while (pthread_mutex_lock(&this_connection[1]->lock));
				write_all(pollfds[1].fd, buffer, len);
				while (pthread_mutex_unlock(&this_connection[1]->lock));
			}
		}
	}
}

void add_connection(int fd) {
	int id = -1;
	for (int i = 0; i < MAXCONN; i++) {
		if (connections[i] == NULL) {
			id = i;
			break;
		} else if (connections[i]->active == 0) {
			pthread_join(connections[i]->thread, NULL);
			free(connections[i]);
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
	if (c == (struct client *)-1) {
		perror("malloc()");
		exit(-1);
	}
	char msg [20];
	sprintf(msg, "CONNECTED, CLIENT %d\n", id);
	write_all(fd, msg, strlen(msg));

	c->active = 1;
	c->fd = fd;
	c->connect_to = -1;
	pthread_mutex_init(&c->lock, NULL);

	connections[id] = c;

	pthread_create(&c->thread, NULL, &thread_start, c);
}

int main(int argc, char** argv) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket()");
		exit(-1);
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
		perror("bind()");
		exit(-1);
	}
	if (listen(sockfd, PENDING) == -1) {
		perror("listen()");
		exit(-1);
	}

	memset(connections, 0, sizeof(struct client *) * MAXCONN);

	for(;;) {
		int newfd = accept(sockfd, NULL, NULL);
		add_connection(newfd);
	}
}