# ddbus
bus-like IPC with broadcast networking

# How to build
````bash
make
sudo make install
````

# Example

````c
#include <ddbus.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void cb(const char* from, const char* msg) {
	printf("%s : %s\n", from, msg);
}

int main(int argc, char **argv) {
	char buf[1024];
	if(argc<2) {fprintf(stderr, "Usage : %s <channel>", argv[0]); return -1; }

	int fd = ddbus_open(argv[1], cb);

	while( fgets(buf, 1024 , stdin) ) {
		ddbus_write(fd, buf);
	}

	return 0;
}

````
