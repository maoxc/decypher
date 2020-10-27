CC=g++
OBJECT_DIR=obj
LIB_DIR=lib
INCLUDE_DIR=include
BIN_DIR=bin
TEST_DIR=test
CFLAGS=-I$(INCLUDE_DIR)
BUILD_DIRS=$(OBJECT_DIR) $(BIN_DIR)

_DEPS=pattern-trie.hpp pattern-trie-node.hpp
DEPS=$(patsubst %,$(INCLUDE_DIR)/%,$(_DEPS))

_OBJECTS=decypher.o pattern-trie.o pattern-trie-node.o
OBJECTS=$(patsubst %,$(OBJECT_DIR)/%,$(_OBJECTS))

all: directories decypher

$(OBJECT_DIR)/%.o: $(LIB_DIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

decypher: $(OBJECTS)
	$(CC) -o $(BIN_DIR)/$@ $^

.PHONY: clean directories

directories:
	@for DIR in $(BUILD_DIRS) ; do \
    	mkdir -p $$DIR ; \
	done

clean:
	@rm -rf $(OBJECT_DIR)/*.o $(BIN_DIR)/decypher && rmdir $(BUILD_DIRS)

obj/test.o: lib/test.cpp catch2/single_include/catch2/catch.hpp
	$(CC) -c -o $@ $< -Icatch2/single_include/catch2 $(CFLAGS)

test: obj/test.o $(OBJECTS)
	$(CC) -o $@.out $^