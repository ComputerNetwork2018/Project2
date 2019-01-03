.PHONY: clean

# Compiler & options
CC=g++
CFLAGS=-std=c++11 -O2 -pthread -Wall -Wextra -Wshadow -Wconversion -fsanitize=address -fsanitize=undefined
CFLAGS+=$(DFLAGS)

# File directory
BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

# Files
_TEST=test_server
_SERVER=server
_CLIENT=client
_TEST_OBJ=common.o test_server.o
_SERVER_OBJ=common.o server.o
_CLIENT_OBJ=terminal.o terminal_util.o codec.o common.o tcpJob.o client.o

# Directed Files
TEST=$(patsubst %, $(BIN_DIR)/%, $(_TEST))
SERVER=$(patsubst %, $(BIN_DIR)/%, $(_SERVER))
CLIENT=$(patsubst %, $(BIN_DIR)/%, $(_CLIENT))
TEST_OBJ=$(patsubst %, $(OBJ_DIR)/%, $(_TEST_OBJ))
SERVER_OBJ=$(patsubst %, $(OBJ_DIR)/%, $(_SERVER_OBJ))
CLIENT_OBJ=$(patsubst %, $(OBJ_DIR)/%, $(_CLIENT_OBJ))

# Default target: all
default: all

# Make all
all: $(TEST) $(SERVER) $(CLIENT)

# Make test
test: $(TEST)

$(TEST): $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Make server
server: $(SERVER)

$(SERVER): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Make client
client: $(CLIENT)

$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Objective file dependency
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean middle files
clean:
	rm -f $(OBJ_DIR)/*
	rm -f $(BIN_DIR)/*
