SOURCES:=$(shell find src/ -maxdepth 1 -type f -name '*.c')
OBJECTS:=$(SOURCES:src/%.c=bin/%.o)

all: libpubsub.so pubsubdd pubsub-test

install:
	cp pubsubdd /usr/bin
	cp pubsubd /usr/bin
	cp libpubsub.so /usr/lib
	cp pubsub-test /usr/bin

libpubsub.so: $(OBJECTS)
	g++ -shared -o $@ $^ -pthread
	
pubsub-test: bin/test/test.o
	g++ -o $@ $^ -pthread -lpubsub -L.

pubsubdd: bin/pubsubd/main.o
	g++ -o $@ $^ -pthread -lpubsub -L.

bin/%.o: src/%.c
	mkdir -p `dirname $@`
	g++ -fPIC -o $@ -c $< -I./src
	
clean:
	rm -rf bin
	rm -f *.so
	rm -f pubsubdd
	rm -f pubsub-test