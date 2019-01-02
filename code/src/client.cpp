#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

#include <cstdint>

#include "common.hpp"
#include "tcpJob.hpp"
#include "msgQueues.hpp"
#include "terminal_util.hpp"

typedef uint64_t Id;

using namespace std;

namespace Client
{
	string serverName;
	int serverPort;
	int serverFd;

	Terminal_Util term = Terminal_Util( );
	bool login = false;

	vector< ::Id > onlineList;
	vector< ::Id > friendList;

	// thread & inter-thread communication
	void tcpSender( );
	bool ready = false;
	
	mutex sendMutex;
	MsgSendQueue msgSendQueue;

	mutex resultMutex;
	MsgCache msgCache;

	void Usage( int argc, char **argv )
	{
		cout << "Usage: " << argv[ 0 ] << " address:port" << endl;
	}

	bool stoiRange( const string &msg, int &result, const int min, const int max )
	{
		try
		{
			result = stoi( msg );
		}
		catch( exception &e )
		{
			return false;
		}

		if( result < min or result > max )
		{
			return false;
		}

		return true;
	}

	int Main_Logout( )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ), Format( ) );
		term.MsgPos( "1. Login", Position( 3, 5 ), Format( ) );
		term.MsgPos( "2. Register", Position( 4, 5 ), Format( ) );
		term.MsgPos( "3. Exit", Position( 5, 5 ), Format( ) );
		term.MsgPos( "Put your choice here: ", Position( 7, 1 ), Format( ) );
		cout << term;

		string userChoice;
		cin >> userChoice;

		int choiceInt;
		
		if( stoiRange( userChoice, choiceInt, 1, 3 ) )
		{
			return choiceInt;
		}
		else
		{
			return -1;
		}
	}

	bool Login( string &sessionId )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ), Format( ) );
		term.MsgPos( "Login: ", Position( 3, 5 ), Format( ) );
		term.MsgPos( "Account: ", Position( 5, 5 ), Format( ) );
		cout << term;
		
		string account;
		cin >> account;

		term.MsgPos( "Password: ", Position( 6, 5 ), Format( ) );
		cout << term;

		string password;
		cin >> password;

		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ), Format( ) );
		term.MsgPos( "Login: ", Position( 3, 5 ), Format( ) );
		term.MsgPos( "Logging in...", Position( 5, 5 ), Format( ) );
		cout << term;

		string command = "login " + account + " " + password;
	}

	void Register( ) { }

	int main( int argc, char **argv )
	{
		if( argc != 2 )
		{
			Usage( argc, argv );
			return 0;
		}
		else
		{
			string server = string( argv[ 1 ] );
			serverName = server.substr( 0, server.find_first_of( ':' ) );
			if( not stoiRange( server.substr( server.find_first_of( ':' ) + 1 ), serverPort, 0, 65535 ) )
			{
				Usage( argc, argv );
				return 0;
			}
		}

		bool exit = false;

		thread tcpThread( tcpSender );

		while( not exit )
		{
			int userChoice = Main_Logout( ); // 1 = login, 2 = reg, 3 = exit
			while( userChoice == -1 )
			{
				userChoice = Main_Logout( );
			}

			switch( userChoice )
			{
				case 1:
					{
						string sessionId;
						bool success = Login( sessionId );
						break;
					}
				case 2:
					Register( );
					break;
				case 3:
					exit = true;
					break;
			}
		}

		tcpThread.join( );
	}

	void tcpSender( )
	{
		while( not exit )
		{
			if( ( not mainQueue.empty( ) ) and sendQueue.empty( ) and ( not ready ) )
			{
				ready = true;
			}
			else if( not ( ( sendQueue.empty( ) or ready ) and ( sendQueue.empty( ) or mainQueue.empty( ) ) ) )
			{
				auto job = sendQueue.front( );
				
				string result = "";
				bool isTimeout;

				job.TryTCP( result, isTimeout );

				if( result != "" )
				{

				}

				if( isTimeout )
				{
					const auto newJob = TCPJob( job.command, job.host, job.timeout, job.id + 1 );
					sendQueue.pop( );
					sendQueue.push( newJob );
				}
			}
		}
	}
}

int main( int argc, char **argv )
{
	return Client::main( argc, argv );
}
