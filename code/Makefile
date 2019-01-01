.PHONY: clean

# Compiler & options
CC=g++
CFLAGS=-std=c++11 -O2
CFLAGS+=$(DFLAGS)

# File directory
BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src

# Files
_SERVER=server
_CLIENT=client
_SERVER_OBJ=terminal.o terminal_util.o msgQueues.o msg.o common.o server.o
_CLIENT_OBJ=terminal.o terminal_util.o msgQueues.o msg.o common.o client.o

# Directed Files
SERVER=$(patsubst %, $(BIN_DIR)/%, $(_SERVER))
CLIENT=$(patsubst %, $(BIN_DIR)/%, $(_CLIENT))
SERVER_OBJ=$(patsubst %, $(OBJ_DIR)/%, $(_SERVER_OBJ))
CLIENT_OBJ=$(patsubst %, $(OBJ_DIR)/%, $(_CLIENT_OBJ))

# Default target: all
default: all

# Make all
all: $(SERVER) $(CLIENT)

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