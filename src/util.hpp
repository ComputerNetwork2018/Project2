#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <string>
#include <vector>

extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}

using namespace std;

namespace TCP_Ping
{
	struct Address_Port
	{
		string address;
		unsigned short port;

		sockaddr_in To_sockaddr_in( );
		Address_Port From_sockaddr_in( sockaddr_in sockaddrIn );
	}
}

#endif
