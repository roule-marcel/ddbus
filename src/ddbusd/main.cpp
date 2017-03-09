/*
 * main.c
 *
 *  Created on: 22 févr. 2017
 *      Author: jfellus
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/un.h>
#include "../udp.h"
#include <errno.h>


char (*get_my_ip_addresses())[32] {
	static char _ips[15][32];
	char ip[128];
	FILE *fp = popen("my-ip-addr", "r");
	if (fp == NULL) return NULL;

	int i=0;
	for (i=0; (fgets(ip, 128, fp) != NULL) && i < 14; i++) {
		if(ip[strlen(ip)-1] == '\n') ip[strlen(ip)-1] = 0;
		strcpy(_ips[i], ip);
	}
	_ips[i][0] = 0;


	int status = pclose(fp);
	return _ips;
}

static char (*my_ips)[32] = 0;
bool is_my_ip(const char* ip) {
	if(my_ips == 0) my_ips = get_my_ip_addresses();
	for(int i=0; my_ips[i][0]!=0; i++) {
		if(!strcmp(my_ips[i], ip)) return true;
	}
	return false;
}


///////


void broadcast_local(const char* s, size_t len);
void broadcast_remote(const char* s, size_t len);


void process_network_message(const char* buf, const char* ip) {
	if(is_my_ip(ip)) return;
	char s[1024];
	sprintf(s,"[%s] %s", ip, buf);
	printf("%s", s);
	broadcast_local(s, strlen(s));
}

void process_local_message(const char* buf) {
	char s[1024];
	sprintf(s,"[localhost] %s", buf);
	printf("%s", s);
	broadcast_local(s, strlen(s));
	broadcast_remote(buf, strlen(buf));
}


pthread_t th_local_server;

typedef struct connection {
	pthread_t th;
	int fd;
} connection_t;
connection_t connections[1024];
int nbConnections = 0;



void* thread_local_connection(void* _c) {
	int n;
	char* buf = (char*)malloc(1024);
	char* line = (char*)malloc(1024);
	int i=0;
	connection_t* c = (connection_t*) _c;
	while ( (n=read(c->fd,buf,sizeof(buf))) > 0) {
		for(int j=0; j<n; j++) {
			if(buf[j]=='\n') {
				line[i] = '\n';
				line[i+1] = 0;
				process_local_message(line);
				i = 0;
			}
			else line[i++] = buf[j];
		}
	}
	c->fd = -1;
	nbConnections--;
	return 0;
}

void broadcast_local(const char* s, size_t n) {
	int j=0;
	for(int i=0; j<nbConnections; i++) {
		if(connections[i].fd==-1) continue;
		write(connections[i].fd, s, n);
		j++;
	}
}

int broadcast_fd;
struct sockaddr_in broadcast_addr;
void broadcast_remote(const char* s, size_t n) {
	int nn = sendto(broadcast_fd, s, n, 0, (const sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
}


void* thread_local_server (void*) {
	struct sockaddr_un addr;
	char buf[100];
	int cl,rc;

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, "/run/lock/ddbusd.sock");

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) ERROR("Can't bind to /run/lock/ddbusd.sock");
	listen(fd, 20);
	int i=0;
	for(i=0; i<1024; i++) connections[i].fd = -1;
	while(1) {
		for(i=0; i<1024; i++) if(connections[i].fd==-1) break;
		connections[i].fd = accept(fd, NULL, NULL);
		nbConnections++;
		pthread_create(&connections[i].th, NULL, thread_local_connection, &connections[i]);
	}

	return 0;
}


int main(int argc, char **argv) {
	char buf[1024];
	char ip[128];

	if(argc <= 1) ERROR("USAGE : %s <broadcast_ip>\n", argv[0]);

	unlink("/run/lock/ddbusd.sock");

	// Create Local server
	pthread_create(&th_local_server, NULL, thread_local_server, NULL);

	// Create UDP client
	int yes = 1;
	broadcast_fd = socket (AF_INET, SOCK_DGRAM, 0);
	setsockopt(broadcast_fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );
    bzero((char *) &broadcast_addr, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    inet_aton(argv[1],&broadcast_addr.sin_addr);
    broadcast_addr.sin_port = htons(16522);

	// Create UDP server
	int fd = udp_server_open(16522);
	int n;
	while((n = udp_read(buf, sizeof(buf), fd, ip)) > 0) {
		process_network_message(buf, ip);
	}
	udp_server_close(fd);
	return 0;
}
