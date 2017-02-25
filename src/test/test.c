/*
 * test.c
 *
 *  Created on: 24 févr. 2017
 *      Author: jfellus
 */

#include <pubsub.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void cb(const char* msg) {
	printf("%s\n", msg);
}

int main(int argc, char **argv) {
	char buf[1024];

	int fd = ddbus_open(cb);

	while( fgets(buf, 1024 , stdin) ) {
		write(fd, buf, strlen(buf));
	}

	return 0;
}

