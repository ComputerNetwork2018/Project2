#ifndef TCPJOB_HPP
#define TCPJOB_HPP

#include <vector>
#include <chrono>

#include <ctime>

#include "common.hpp"

using namespace std;
using namespace chrono;

namespace Client
{
	class TCPJob
	{
	private:
		steady_clock::time_point startClock;

		int serverFd;
		bool nothingToDo;
		bool isHandshaking, isConnected, isSent, infoGiven;

		string result;
		sockaddr_in socketInfo;

		void _TryTCP( );

	public:
		TCPJob( const string &_command, const string &_host, const int _port, const int _timeout = 0, const int _id = 0 );

		string command;
		string host;
		int port, timeout, id;

		void TryTCP( string &info, bool &isTimeout );
		int currentDelay( );
	};
}

#endif