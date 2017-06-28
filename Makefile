SOURCES:=$(shell find src/ -maxdepth 1 -type f -name '*.cpp')
OBJECTS:=$(SOURCES:src/%.cpp=bin/%.o)

# Target build repository 
BUILD=build
LIB=$(BUILD)/usr/lib
BIN=$(BUILD)/usr/bin
INSTALL_DIR?=/

all: $(LIB)/libddbus.so $(BIN)/ddbusdd  $(BIN)/ddbus

install:
	./install_tmp.sh $(INSTALL_DIR)

$(LIB)/libddbus.so: $(OBJECTS)
	@mkdir -p $(LIB)
	g++ -shared -o $@ $^

$(BIN)/ddbus: bin/test/test.o
	@mkdir -p $(BIN)
	gcc -g  -o $@ $^ -pthread -lddbus -L.

$(BIN)/ddbusdd: bin/ddbusd/main.o
	@mkdir -p $(BIN)
	g++ -g  -o $@ $^ -pthread -lddbus -L.


bin/test/%.o: src/test/%.c
	mkdir -p `dirname $@`
	gcc -g -fPIC -o $@ -c $< -I./src


bin/%.o: src/%.cpp
	mkdir -p `dirname $@`
	g++ -g -fPIC -o $@ -c $< -I./src

clean:
	rm -rf bin
	rm -rf $(BUILD)
