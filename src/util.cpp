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

#include "util.hpp"

using namespace std;

namespace TCP_Ping
{
	sockaddr_in Address_Port::To_sockaddr_in( )
	{
		sockaddr_in address_info;
		addrinfo hint;
		memset( &hint, 0, sizeof( hint ) );

		hint.ai_family = AF_UNSPEC;
		hint.ai_socktype = SOCK_STREAM;

		int result = getaddrinfo( address.c_str( ), to_string( port ).c_str( ), hint, &result );

		if( result = -1 )
		{
			clog << "getaddrinfo( ) error: " << gai_strerror( result ) << endl;
		}

		return address_info;
	}

	Address_Port Address_Port::From_sockaddr_in( sockaddr_in sockaddrIn )
	{
		address = inet_ntoa( sockaddrIn.sin_addr );
		port = sockaddrIn.sin_port;
	}
}
