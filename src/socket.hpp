#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <string>

extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
}

using namespace std;

namespace TCP_Ping
{
	static const string DEFAULT_MESSAGE = "OuOb";
	
	class Socket
	{
		private:
			int fd_;

		public:
			Socket( const int socketFd = -1, const unsigned int timeout = 0 );

			bool Send( const string &message ) const;
			string Recv( ) const;

			int Fd( ) const { return fd_; }
			void Fd( const int fd ) { fd_ = fd; }
	};
}
#endif
