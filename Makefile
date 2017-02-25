SOURCES:=$(shell find src/ -maxdepth 1 -type f -name '*.c')
OBJECTS:=$(SOURCES:src/%.c=bin/%.o)

all: libddbus.so ddbusdd ddbus-test

install:
	cp ddbusdd /usr/bin
	cp ddbusd /usr/bin
	cp libddbus.so /usr/lib
	cp ddbus-test /usr/bin

libddbus.so: $(OBJECTS)
	g++ -shared -o $@ $^ -pthread
	
ddbus-test: bin/test/test.o
	g++ -o $@ $^ -pthread -lddbus -L.

ddbusdd: bin/ddbusd/main.o
	g++ -o $@ $^ -pthread -lddbus -L.

bin/%.o: src/%.c
	mkdir -p `dirname $@`
	g++ -fPIC -o $@ -c $< -I./src
	
clean:
	rm -rf bin
	rm -f *.so
	rm -f ddbusdd
	rm -f ddbus-test