#include <iostream>
#include <string>
#include <vector>
#include <exception>

#include <cstring>

extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
}

#include "server.hpp"
#include "socket.hpp"

using namespace std;

#ifdef DEBUG

#define DEBUG_ERROR

#endif

namespace TCP_Ping
{
	bool Server::ParseArgs( vector< string >args )
	{
		if( args.size( ) != 2 )
		{
			return false;
		}

		try
		{
			unsigned int port = stoul( args[ 1 ], nullptr, 10 );

			if( port >= 65536 )
			{
				throw out_of_range( "Port number " + args[ 1 ] + " is too large." );
			}

			listen_port_ = port;
		}
		catch( invalid_argument e )
		{
			clog << "Invalid port number : \"" << args[ 1 ] << '\"' << endl;
			return false;
		}
		catch( out_of_range e )
		{
			clog << "Port number " << args[ 1 ] << " out of range." << endl;
			return false;
		}

		return true;
	}

	void Server::Usage( )
	{
		clog << "\n\nUsage: ./server listenPort\n"
			 << "\n"
			 << "    listenPort: Listening port of the server.\n"
			 << endl;
	}
	
	Server::Server( )
	{
	}

	bool Server::StartListen( )
	{
		int socketFd;
		addrinfo hints, *info;

		memset( &hints, 0, sizeof( hints ) );
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		int result = getaddrinfo( nullptr, to_string( listen_port_ ).c_str( ), &hints, &info );
		if( result != 0 )
		{
#ifdef DEBUG_ERROR
			clog << "\e[1;31m[ERROR]\e[m getaddrinfo: " << gai_strerror( result ) << endl;
#endif
			return false;
		}

		for( auto curr = info; curr != nullptr; curr = curr -> ai_next )
		{
#ifdef DEBUG
			clog << "Next." << endl;
#endif
			socketFd = socket( curr -> ai_family, curr -> ai_socktype, curr -> ai_protocol );
#ifdef DEBUG
			clog << "socketFd = " << socketFd << endl;
#endif
			if( socketFd == -1 )
			{
#ifdef DEBUG_ERROR
				clog << "\e[1;31m[ERROR]\e[m Unable to create socket." << endl;
#endif
				continue;
			}
#ifdef DEBUG
			clog << "Try bind with fd " << socketFd << endl;
#endif
			if( bind( socketFd, curr -> ai_addr, curr -> ai_addrlen ) == -1 )
			{
#ifdef DEBUG_ERROR
				clog << "\e[1;31m[ERROR]\e[m Unable to bind." << endl;
#endif
				continue;
			}
#ifdef DEBUG
			clog << "Bound: socketFd = " << socketFd << endl;
#endif
			listen_socket_ = Socket( socketFd );
			freeaddrinfo( info );
			listen( listen_socket_.Fd( ), DEFAULT_BACKLOG );
			fcntl( listen_socket_.Fd( ), F_SETFL, O_NONBLOCK );

			return true;
		}

		freeaddrinfo( info );
		return false;
	}

	int Server::Main( vector< string > args )
	{
		if( not ParseArgs( args ) )
		{
			Usage( );
			return 0;
		}

		if( not StartListen( ) )
		{
#ifdef DEBUG_ERROR
			clog << "\e[1;31m[ERROR]\e[m Failed to bind socket." << endl;
#endif
			return 0;
		}

		while( true )
		{
			struct sockaddr_in clientInfo;
			socklen_t addrlen;

			int clientFd = accept( listen_socket_.Fd( ), reinterpret_cast<sockaddr*>( &clientInfo ), &addrlen );
			if( clientFd == -1 )
			{
				if( errno == EWOULDBLOCK )
				{
					continue;
				}
				else
				{
#ifdef DEBUG_ERROR
					clog << "\e[1;31m[ERROR]\e[m Accept error: " << strerror( errno ) << endl;
#endif
					return -1;
				}
			}
			else
			{
#ifdef DEBUG
				clog << "Client accepted, fd = " << clientFd << endl;
#endif				
				Socket clientSocket = Socket( clientFd );

				string message = clientSocket.Recv( );
				if( message.length( ) == 0 )
				{
#ifdef DEBUG
					clog << "Clinet session end." << endl;
#endif
					close( clientSocket.Fd( ) );
					continue;
				}
				else
				{
#ifdef DEBUG
					clog << "Client message received: " << message << endl;
#endif
					struct sockaddr_in addr;
					socklen_t addrlen;
					getpeername( clientSocket.Fd( ), reinterpret_cast<sockaddr*>( &addr ), &addrlen );

					cout << "recv from " << inet_ntoa( addr.sin_addr ) << ":" << addr.sin_port << endl;
				}

				if( clientSocket.Send( DEFAULT_MESSAGE ) )
				{
#ifdef DEBUG
					clog << "Message sent to client."  << endl;
#endif
				}
				else
				{
#ifdef DEBUG_ERROR
					clog << "\e[1;31m[ERROR]\e[m Message sending failed." << endl;
#endif
				}

				close( clientSocket.Fd( ) );
			}
		}

		return 0;
	}
}
