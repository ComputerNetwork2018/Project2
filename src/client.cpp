#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <exception>
#include <chrono>

#include <cstring>
#include <time.h>

extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
}

#include "client.hpp"
#include "socket.hpp"

using namespace std;
using namespace std::chrono;

#ifdef DEBUG

// #define DEBUG_PARSEARG
#define DEBUG_ERROR

#endif

namespace TCP_Ping
{
	bool Client::ParseArgs( vector< string > args )
	{
#ifdef DEBUG_PARSEARG
		for( auto arg : args )
		{
			clog << "ListArgs: " << arg << endl;
		}
#endif

		for( int i = 1; i < args.size( ); ++i )
		{
			if( args[ i ] == "-n" )
			{
				if( i + 1 >= args.size( ) )
				{
					clog << "Missing argument after -n." << endl;
					return false;
				}

				try
				{
					ping_count_ = stoul( args[ i + 1 ], nullptr, 10 );
				}
				catch( invalid_argument e )
				{
					clog << "Invalid argument after -n : \"" << args[ i + 1 ] << '\"' << endl;
					return false;
				}
				catch( out_of_range e )
				{
					clog << "Ping count " << args[ i + 1 ] << " out of range." << endl;
					return false;
				}

				++i;
			}
			else if( args[ i ] == "-t" )
			{
				if( i + 1 >= args.size( ) )
				{
					clog << "Missing argument after -t." << endl;
					return false;
				}

				try
				{
					timeout_ = stoul( args[ i + 1 ], nullptr, 10 );
				}
				catch( invalid_argument e )
				{
					clog << "Invalid argument after -t : \"" << args[ i + 1 ] << '\"' << endl;
					return false;
				}
				catch( out_of_range e )
				{
					clog << "Timeout " << args[ i + 1 ] << " out of range." << endl;
					return false;
				}

				++i;
			}
			else
			{
				size_t pos = args[ i ].find_first_of( ':' );
				if( pos == args[ i ].npos )
				{
					clog << "Invalid port number : port number DNE." << endl;
					return false;
				}
				else
				{
					try
					{
						unsigned int port = stoul( args[ i ].substr( pos + 1 ), nullptr, 10 );

						if( port >= 65536 )
						{
							throw out_of_range( "Port number " + args[ i ].substr( pos + 1 ) + "is too large." );
						}

						connectionList_.push_back( ClientSocketConnection( args[ i ].substr( 0, pos ), port ) );
					}
					catch( invalid_argument e )
					{
						clog << "Invalid port number : \"" << args[ i ].substr( pos + 1 ) << '\"' << endl;
						return false;
					}
					catch( out_of_range e )
					{
						clog << "Port number " << args[ i ].substr( pos + 1 ) << " out of range." << endl;
						return false;
					}
				}
			}
		}

		if( connectionList_.size( ) == 0 )
		{
			clog << "Invalid host list: no connection in list." << endl;
			return false;
		}

#ifdef DEBUG_PARSEARG
		clog << "ping_count_ = " << ping_count_ << endl
			 << "timeout_ = " << timeout_ << endl
			 << "connectionList_ :" << endl;
		for( auto i : connectionList_ )
		{
			clog << "( " << i.address << " , " << i.port << " , " << i.socketFd << " )" << endl;
		}
#endif
		
		return true;
	}

	void Client::Usage( )
	{
		clog << "\n\nUsage: ./client [-n N] [-t T] host1:port1 host2:port2 ...\n"
			 << "\n"
			 << "    N   : Number of packets sent to each host.\n"
			 << "          If N = 0, ping until process ends.\n"
			 << "          Default value: 0.\n"
			 << "\n"
			 << "    T   : Waiting timeout in milliseconds.\n"
			 << "          Default value: 1000.\n"
			 << endl;
	}

	Client::Client( ) : connectionList_( 0 ) {}

	bool Client::TryConnect( ClientSocketConnection &connection )
	{
#ifdef DEBUG
		clog << "TryConnect( " << connection << " )" << endl;
#endif
		int socketFd;
		addrinfo hints, *info;

		memset( &hints, 0, sizeof( hints ) );
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		int result = getaddrinfo( connection.address.c_str( ), to_string( connection.port ).c_str( ), &hints, &info );
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
			clog << "Try connect with fd " << socketFd << endl;
#endif
			if( connect( socketFd, curr -> ai_addr, curr -> ai_addrlen ) == -1 )
			{
#ifdef DEBUG_ERROR
				clog << "\e[1;31m[ERROR]\e[m Unable to connect" << endl;
#endif
				close( socketFd );
				continue;
			}

			connection.socket.Fd( socketFd );
			freeaddrinfo( info );
#ifdef DEBUG
			clog << "Connected: socketFd = " << socketFd << endl;
#endif
			return true;
		}

		freeaddrinfo( info );
		return false;
	}

	void Client::FailToConnect( ClientSocketConnection &connection )
	{
		cout << "timeout when connect to " << connection.address << endl;
	}

	int Client::Main( vector< string > args )
	{
		if( not ParseArgs( args ) )
		{
			Usage( );
			return 0;
		}

		for( int i = 0; ping_count_ == 0 or i < ping_count_; ++i )
		{
			for( auto &connection : connectionList_ )
			{
				if( not TryConnect( connection ) )
				{
#ifdef DEBUG_ERROR
					clog << "\e[1;31m[ERROR]\e[m Failed to connect to " << connection.address << " : " << connection.port << " , abort." << endl;
#endif
					FailToConnect( connection );
					continue;
				}
				else
				{
#ifdef DEBUG
					clog << "Done connection to " << connection << endl;
#endif
					if( connection.socket.Fd( ) != -1 )
					{
						steady_clock::time_point startTime = steady_clock::now( );

						if( connection.socket.Send( DEFAULT_MESSAGE ) )
						{
#ifdef DEBUG
							clog << "Message sent to server." << endl;
#endif
						}
						else
						{
#ifdef DEBUG_ERROR
							clog << "\e[1;31m[ERROR]\e[m Message sending failed." << endl;
#endif
							FailToConnect( connection );
							close( connection.socket.Fd( ) );
							connection.socket.Fd( -1 );

							continue;
						}

						string message = connection.socket.Recv( );
						if( message.length( ) == 0 )
						{
#ifdef DEBUG_ERROR
							clog << "\e[1;31m[ERROR]\e[m Message recieving failed." << endl;
#endif
							FailToConnect( connection );
							close( connection.socket.Fd( ) );
							connection.socket.Fd( -1 );

							continue;
						}
						else
						{
#ifdef DEBUG
							clog << "Server message recieved : " << message << endl;
#endif
							steady_clock::time_point endTime = steady_clock::now( );
							duration< int, milli > timeSpan = duration_cast< duration< int, milli > >( endTime - startTime );

							if( timeSpan.count( ) > timeout_ )
							{
								FailToConnect( connection );
							}
							else
							{
								cout << "recv from " << connection.address << ", RTT = " << timeSpan.count( ) << " msec" << endl;

								usleep( ( timeout_ - timeSpan.count( ) ) * 1000 / connectionList_.size( ) );
							}

							close( connection.socket.Fd( ) );
							connection.socket.Fd( -1 );
						}
					}
				}
			}
		}
#ifdef DEBUG
		clog << "Done, close sessions." << endl;
#endif
		for( auto &connection : connectionList_ )
		{
			if( connection.socket.Fd( ) != -1 )
			{
				close( connection.socket.Fd( ) );
				connection.socket.Fd( -1 );
			}
		}

		return 0;
	}

	unsigned int Client::ping_count_ = Client::DEFAULT_PING_COUNT;
	unsigned int Client::timeout_ = Client::DEFAULT_TIMEOUT;
}

