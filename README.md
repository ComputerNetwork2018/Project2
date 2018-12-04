# README


## Usage

### Compile:

- make: make all needed executables in directory 'bin/'.

- make DFLAGS=-DDEBUG: enable debug mode.

- make obj/[ object name ]: compile individual objects.
	- server.o server_main.o: server
	- client.o client_main.o: client
	- socket.o: socket

- make bin/[ client | server ]: make client or server executable only.

- make clean: clean up all objective and binary files.


### Client:

./client [-n N] [-t T] host1:port1 host2:port2 ...

- N 
	- Number of packets sent to each host.
	- If N = 0, ping until process ends.
	- Default value: 0.

- T
	- Waiting timeout in milliseconds.
	- Default value: 1000.

- The order of parameters does not matter.


### Server:

./server listenPort

- listenPort: Listening port of the server.



## Implementation Detail


### Socket:

- A C++ Class in charge of sending and recieving messages.

- Saves file descriptor.


### Client:

- A C++ Class TCP-ping-ing server.

- Using a vector< ClientSocketConnection > to maintain connections with servers.
	- struct ClientSocketConnection: saves address, port and socket to a server.

- Using a for-loop running through ClientSocketConnection list, ping one server at a time.

- At most block by send() or recv() by T milliseconds.

- Intentionally yield a ( T / # of servers ) millisecond gap between pings among servers for smooth performance.
	- At line 305, client.cpp.
	- Removal of this line is totally fine, if we need fast test-runs.


### Server:

- A C++ Class being TCP-ping-ed.

- Using simple bind-listen and accept-recv-send flow.

- A while-loop checking backlogs so that server won't be blocked by accept( ),
