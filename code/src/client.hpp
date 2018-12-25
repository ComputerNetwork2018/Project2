#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <thread>

extern "C"
{
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
}

#include "socket.hpp"

using namespace std;

namespace TCP_Ping
{

	class Client
	{
		private:
			struct ClientSocketConnection
			{
				string address;
				unsigned short port;
				Socket socket;

				ClientSocketConnection( const string &address = "", const unsigned short port = 0, int socketFd = -1 )
				{
					this -> address = address;
					this -> port = port;
					socket = Socket( socketFd, timeout_ );
				}

				friend ostream& operator<<( ostream &output, ClientSocketConnection &connection )
				{
					output << "( " << connection.address << " , " << connection.port << " , " << connection.socket.Fd( ) << " )";
					return output;
				}
			};

			static const unsigned int DEFAULT_PING_COUNT = 0;
			static const unsigned int DEFAULT_TIMEOUT = 1000;

			static unsigned int ping_count_;
			static unsigned int timeout_;

			vector< ClientSocketConnection > connectionList_;

			bool ParseArgs( vector< string > args );
			void Usage( );

			bool TryConnect( ClientSocketConnection &connection );
			void FailToConnect( ClientSocketConnection &connection );

		public:
			Client( );
			int Main( vector< string > args );
	};
}

#endif
