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
#include <sys/stat.h>


pthread_t th_local_server;


typedef struct {
	char ip[32];
	int broadcast_fd;
	struct sockaddr_in broadcast_addr;
} broadcast_t;

typedef struct connection {
	pthread_t th;
	int fd;
	broadcast_t* broadcast;
} connection_t;
connection_t connections[1024];
int nbConnections = 0;

/////////////

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

bool str_starts_with(const char* s, const char* prefix) {
	return strncmp(s, prefix, strlen(prefix))==0;
}


///////

static broadcast_t* create_udp_client(const char* broadcast_addr);

void broadcast_local(const char* s, size_t len);
void broadcast_remote(broadcast_t* broadcast, const char* s, size_t len);


void process_network_message(const char* buf, const char* ip) {
	if(is_my_ip(ip)) return;
	char s[1024];
	sprintf(s,"[%s] %s", ip, buf);
	printf("%s", s);
	broadcast_local(s, strlen(s));
}

void process_local_message(connection_t* c, const char* buf) {
	char s[1024];
	sprintf(s,"[localhost] %s", buf);
	printf("%s", s);
	broadcast_local(s, strlen(s));
	broadcast_remote(c->broadcast, buf, strlen(buf));
}


void set_connection_broadcast_ip(connection_t* c, const char* broadcast_ip) {
	c->broadcast = create_udp_client(broadcast_ip);
}



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
				if(str_starts_with(line, "__BroadcastNetwork:")) set_connection_broadcast_ip(c, strchr(line,':')+1);
				else process_local_message(c, line);
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

broadcast_t* default_broadcast = NULL;
void broadcast_remote(broadcast_t* c, const char* s, size_t n) {
	int nn = sendto(c->broadcast_fd, s, n, 0, (const sockaddr*)&c->broadcast_addr, sizeof(struct sockaddr_in));
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

	chmod(addr.sun_path, 0666);

	while(1) {
		for(i=0; i<1024; i++) if(connections[i].fd==-1) break;
		connections[i].fd = accept(fd, NULL, NULL);
		connections[i].broadcast = default_broadcast;
		nbConnections++;
		pthread_create(&connections[i].th, NULL, thread_local_connection, &connections[i]);
	}

	return 0;
}

static broadcast_t* get_udp_client(const char* broadcast_addr) {
	int j=0;
	for(int i=0; j<nbConnections; i++) {
		if(connections[i].fd==-1) continue;
		if(!strcmp(connections[i].broadcast->ip, broadcast_addr)) return connections[i].broadcast;
		j++;
	}
	if(default_broadcast && !strcmp(default_broadcast->ip, broadcast_addr)) {
		return default_broadcast;
	}
	return 0;
}

static broadcast_t* create_udp_client(const char* broadcast_addr) {
	broadcast_t *b = get_udp_client(broadcast_addr);
	if(!b) b = (broadcast_t*) malloc(sizeof(broadcast_t));
	else return b;
	strcpy(b->ip, broadcast_addr);
	int yes = 1;
	b->broadcast_fd = socket (AF_INET, SOCK_DGRAM, 0);
	setsockopt(b->broadcast_fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes) );
	bzero((char *) &b->broadcast_addr, sizeof(b->broadcast_addr));
	b->broadcast_addr.sin_family = AF_INET;
	inet_aton(broadcast_addr,&b->broadcast_addr.sin_addr);
	b->broadcast_addr.sin_port = htons(16522);
	return b;
}


int main(int argc, char **argv) {
	char buf[1024];
	char ip[128];

	if(argc <= 1) ERROR("USAGE : %s <broadcast_ip>\n", argv[0]);

	unlink("/run/lock/ddbusd.sock");

	// Create Local server
	pthread_create(&th_local_server, NULL, thread_local_server, NULL);

	// Create UDP client
	default_broadcast = create_udp_client(argv[1]);

	// Create UDP server
	int fd = udp_server_open(16522);
	int n;
	while((n = udp_read(buf, sizeof(buf), fd, ip)) > 0) {
		process_network_message(buf, ip);
	}
	udp_server_close(fd);
	return 0;
}
