/*
 * test.c
 *
 *  Created on: 24 févr. 2017
 *      Author: jfellus
 */

#include <ddbus.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void cb(const char* from, const char* msg) {
	printf("%s : %s\n", from, msg);
}


int usage(const char* argv0) {
	fprintf(stderr, "Usage : %s [-b broadcast_ip] <channel>\n", argv0); return -1;
}

int main(int argc, char **argv) {
	char buf[1024];
	if(argc<2 || argc==3 || argc>4) return usage(argv[0]);
	if(argc==4) {
		if(strcmp(argv[1], "-b")) return usage(argv[0]);
	}
	int fd = ddbus_open(argv[argc-1], cb);

	if(argc==4) ddbus_set_broadcast_ip(fd, argv[2]);

	while( fgets(buf, 1024 , stdin) ) {
		ddbus_write(fd, buf);
	}

	return 0;
}

