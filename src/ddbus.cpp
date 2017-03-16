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
#include <vector>

typedef struct ddbus_t {
	pthread_t th;
	int fd;
	void (*callback)(const char* from, const char* msg);
	char channel[512];
} ddbus_t;

std::vector<ddbus_t*> buses;


void* thread_read(void* _p) {
	ddbus_t* p  = (ddbus_t*)_p;
	int n;
	char buf[1024];
	char line[1024];
	int i=0;
	while( (n = read(p->fd, buf, 1024)) > 0) {
		for(int j=0; j<n; j++) {
			if(buf[j]=='\n') {
				line[i] = 0;
				char* from = &line[1];
				char* channel = strchr(from, ']'); *channel = 0; channel+=2;
				if(*channel) {
					char* msg = strchr(channel, ':'); *msg = 0; msg++;
					if(!strcmp(channel, p->channel)) p->callback(from, msg);
				}
				i = 0;
			}
			else line[i++] = buf[j];
		}
	}
	return 0;
}

static int ddbusd_start() {
	system("ddbusd start >/dev/null 2>&1");
}

int ddbus_write(int fd, const char* msg) {
	char buf[strlen(msg)+5+strlen(buses[fd]->channel)];
	sprintf(buf, "%s:%s", buses[fd]->channel, msg);
	::write(fd, buf, strlen(buf));
}


int ddbus_open(const char* channel, void (*callback) (const char* from, const char* msg)) {
	ddbusd_start();
	ddbus_t* p = new ddbus_t;
	struct sockaddr_un addr;

	p->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	p->callback = callback;
	strcpy(p->channel,channel);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/run/lock/ddbusd.sock");

	if(connect (p->fd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) ERROR("Couldn't connect to /run/lock/ddbusd.sock\n");

	while(buses.size()<p->fd+1) buses.push_back(0);
	buses[p->fd] = p;

	if(callback) pthread_create(&p->th, NULL, thread_read, p);
	return p->fd;
}

