/*
 * pubsub-talk.c
 *
 *  Created on: 23 févr. 2017
 *      Author: jfellus
 */


#include "ddbus.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <pthread.h>
#include "udp.h"

typedef struct pubsub_t {
	pthread_t th;
	int fd;
	void (*callback)(const char*);
} pubsub_t;

void* thread_read(void* _p) {
	pubsub_t* p  = (pubsub_t*)_p;
	int n;
	char buf[1024];
	char line[1024];
	int i=0;
	while( (n = read(p->fd, buf, 1024)) > 0) {
		for(int j=0; j<n; j++) {
			if(buf[j]=='\n') {
				line[i] = 0;
				p->callback(line);
				i = 0;
			}
			else line[i++] = buf[j];
		}
	}
	return 0;
}

extern "C" {

int ddbus_open(void (*callback) (const char*)) {

	system("ddbusd start");

	pubsub_t* p = (pubsub_t*)malloc(sizeof(pubsub_t));
	struct sockaddr_un addr;

	p->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	p->callback = callback;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/run/lock/ddbusd.sock");

	if(connect (p->fd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) ERROR("Couldn't connect to /run/lock/ddbusd.sock\n");

	pthread_create(&p->th, NULL, thread_read, p);
	return p->fd;
}

}
