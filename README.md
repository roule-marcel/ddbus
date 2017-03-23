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
#### Setting the broadcast ip
Optionally, you can set the broadcast address a specific bus will be forwarded to :
````c
...
int fd = ddbus_open(bus_name, cb);
ddbus_set_broadcast_ip(fd, "172.17.255.255"); // All data put on bus 'bus_name' will be forwarded to broadcast address "172.17.255.255"
...
````
