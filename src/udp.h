/*
 * udp.h
 *
 *  Created on: 22 févr. 2017
 *      Author: jfellus
 */

#ifndef UDP_H_
#define UDP_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>

#define ERROR(...) do {	printf( __VA_ARGS__ ); exit(1); } while(0)


int udp_server_open(int port) {
	int sock, status, buflen;
	struct sockaddr_in sock_in;

	sock = socket (AF_INET, SOCK_DGRAM, 0);

	memset(&sock_in, 0, sizeof(struct sockaddr_in));
	sock_in.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_in.sin_port = htons(port);
	sock_in.sin_family = AF_INET;

	if(bind(sock, (struct sockaddr *)&sock_in, sizeof(struct sockaddr_in)) != 0) {
		ERROR("Can't bind to port %u\n", port);
	}

	return sock;
}

int udp_read(char* buf, size_t len, int sock, char* addr) {
	struct sockaddr_in sock_in;
	unsigned sinlen = sizeof(struct sockaddr_in);
	int n = recvfrom(sock, buf, len, 0, (struct sockaddr *)&sock_in, &sinlen);
	if(n<0) return 0;
	buf[n] = 0;
	if(addr) strcpy(addr, inet_ntoa(sock_in.sin_addr));
	return n;
}

void udp_server_close(int sock) {
	shutdown(sock, 2);
	close(sock);
}


#endif /* UDP_H_ */
