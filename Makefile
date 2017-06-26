SOURCES:=$(shell find src/ -maxdepth 1 -type f -name '*.cpp')
OBJECTS:=$(SOURCES:src/%.cpp=bin/%.o)

all: libddbus.so ddbusdd ddbus

install:
	service ddbusd stop || echo
	cp my-ip-addr /usr/bin
	cp ddbusdd /usr/bin
	cp ddbusd /usr/bin
	cp libddbus.so /usr/lib
	cp ddbus /usr/bin
	cp src/ddbus.h /usr/include
	cp ddbusd.service /etc/init.d/ddbusd
	service ddbusd start
	update-rc.d ddbusd defaults

libddbus.so: $(OBJECTS)
	g++ -shared -o $@ $^

ddbus: bin/test/test.o
	gcc -g  -o $@ $^ -pthread -lddbus -L.

ddbusdd: bin/ddbusd/main.o
	g++ -g  -o $@ $^ -pthread -lddbus -L.


bin/test/%.o: src/test/%.c
	mkdir -p `dirname $@`
	gcc -g -fPIC -o $@ -c $< -I./src


bin/%.o: src/%.cpp
	mkdir -p `dirname $@`
	g++ -g -fPIC -o $@ -c $< -I./src

clean:
	rm -rf bin
	rm -f *.so
	rm -f ddbusdd
	rm -f ddbus
	rm -f ddbus-test
