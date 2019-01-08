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
		int serverFd;
		bool nothingToDo = false;
		bool isHandshaking = false;
		bool isConnected = false;
		bool isSent = false;
		bool infoGiven = false;
		
		steady_clock::time_point startClock;

		string result;
		sockaddr_in socketInfo;

		void _TryTCP( );

	public:
		TCPJob( const string &_command, const string &_host, const int _port, const int _timeout = 6000, const int _id = 0 );
		~TCPJob( );

		string command;
		string host;
		int port, timeout, id;

		void TryTCP( string &info, bool &isTimeout );
		int currentDelay( );

		friend ostream& operator<< ( ostream &output, TCPJob &tcpJob );
	};
}

#endif