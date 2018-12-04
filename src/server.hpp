#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>

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
	class Server
	{
		private:
			static const unsigned int DEFAULT_BACKLOG = 20;

			unsigned short listen_port_;
			Socket listen_socket_;

			bool ParseArgs( vector< string > args );
			void Usage( );

			bool StartListen( );

		public:
			Server( );
			int Main( vector< string > args );
	};
}

#endif
