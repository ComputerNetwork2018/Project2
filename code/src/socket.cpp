#include <iostream>
#include <string>
#include <cstring>

extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
}

#include "socket.hpp"

using namespace std;

namespace TCP_Ping
{
	Socket::Socket( const int socketFd, const unsigned int timeout ) : fd_( socketFd )
	{
		setsockopt( fd_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof( timeout ) );
		setsockopt( fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) );
	}

	bool Socket::Send( const string &message ) const
	{
		int size = send( fd_, message.c_str( ), message.length( ), 0 );
		if( size == -1 )
		{
			clog << "Socket #" << fd_ << " send error : " << strerror( errno ) << endl;
			return false;
		}
		else if( size != message.length( ) )
		{
			clog << "\e[1;33m[Warning] Message not fully sent.\e[m" << endl;
		}
		return true;
	}

	string Socket::Recv( ) const
	{
		string message( 16, '\0' );
		
		int size = recv( fd_, &message[ 0 ], DEFAULT_MESSAGE.length( ), 0 );
		if( size == -1 )
		{
			clog << "Socket #" << fd_ << " recv error : " << strerror( errno ) << endl;
			return "";
		}

		message.resize( size );
		return message;
	}
}
