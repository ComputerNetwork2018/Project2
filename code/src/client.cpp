#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <mutex>

#include <cstdint>

#include <unistd.h>

#include "common.hpp"
#include "tcpJob.hpp"
#include "terminal_util.hpp"

#define JOB(C) (TCPJob(C,serverName,serverPort))

using namespace std;

namespace Client
{
	string serverName;
	int serverPort;
	int serverFd;

	Terminal_Util term = Terminal_Util( );

	// thread & inter-thread communication
	void tcpSender( );
	bool ready = false;

	mutex sendMutex;
	queue<TCPJob> sendQueue;

	mutex resultMutex;
	queue<string> resultQueue;

	// login variables
	bool login = false;
	string sessionToken;

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

	bool Login( string &sessionToken )
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

		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ), Format( ) );
		term.MsgPos( "Login: ", Position( 3, 5 ), Format( ) );
		term.MsgPos( "Logging in...", Position( 5, 5 ), Format( ) );
		cout << term;

		string command = "login " + account + " " + password;

		unique_lock<mutex> sendLock( sendMutex );
		sendQueue.push( JOB( command ) );
		sendLock.unlock( );
		usleep( 50000 );
		
		bool loginPending = true;
		bool loginSuccess = false;

		while( loginPending )
		{
			term.MsgPos( "(loginPending)", Position( 6, 5 ), Format( ) );
			cout << term;

			unique_lock<mutex> resultLock( resultMutex );

			if( not resultQueue.empty( ) )
			{
				string &result = resultQueue.front( );
				
				if( result.substr( 2 ) == "WA" )
				{
					loginSuccess = false;
					term.MsgPos( " failed", Position( 5, 19 ), Format( ) );
					cout << term;
				}
				else if( result.substr( 2 ) == "AC" )
				{
					loginSuccess = true;
					sessionToken = result.substr( 3, 16 );
					term.MsgPos( " success!", Position( 5, 19 ), Format( ) );
					cout << term;
				}
				else
				{
					term.MsgPos( "WTF?", Position( ), Format( FORMAT_BOLD, COLOR_RED, COLOR_BLACK ) );
				}

				loginPending = false;
			}

			resultLock.unlock( );

			int ret = usleep( 50000 );
			term.MsgPos( "usleep ret = " + to_string( ret ), Position( 7, 5 ), Format( ) );
			cout << term;
		}

		return loginSuccess;
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
						bool success = Login( sessionToken );
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
			string result = "";
			
			unique_lock<mutex> sendLock( sendMutex );

			term.MsgPos( "sender: mutex get", Position( 20, 5 ), Format( ) );
			cout << term;
			
			if( not sendQueue.empty( ) )
			{
				auto job = sendQueue.front( );
				bool isTimeout;

				job.TryTCP( result, isTimeout );

				if( isTimeout )
				{
					const auto newJob = TCPJob( job.command, job.host, job.port, job.timeout, job.id + 1 );
					sendQueue.pop( );
					sendQueue.push( newJob );
				}
			}

			sendLock.unlock( );
			usleep( 65537 );

			if( result != "" )
			{
				unique_lock<mutex> resultLock( resultMutex );

				resultQueue.push( result );
			}
		}
	}
}

int main( int argc, char **argv )
{
	return Client::main( argc, argv );
}
