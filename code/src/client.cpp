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

using namespace std;

namespace Client
{
	string serverName;
	int serverPort;
	int serverFd;

	Terminal_Util term;

	// thread & inter-thread communication
	void tcpSender( );
	bool exit;

	mutex sendMutex;
	queue<TCPJob> sendQueue;

	mutex resultMutex;
	queue<string> resultQueue;

	// login variables
	bool login = false;
	string sessionToken;

	void Usage( int argc, char **argv )
	{
		clog << "Usage: " << argv[ 0 ] << " address:port" << endl;
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

	void WaitEnter( const Position &pos = Position( ), const Format &format = Format( ), const string &msg = "press ENTER to contunue." )
	{
		term.MsgPos( msg, pos, format );
		cout << term;

		char temp = getchar( );

		do
		{
			temp = getchar( );
		} while( temp != '\n' );
	}

	int Main_Logout( )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "1. Login", Position( 3, 5 ) );
		term.MsgPos( "2. Register", Position( 4, 5 ) );
		term.MsgPos( "3. Exit", Position( 5, 5 ) );
		term.MsgPos( "Put your choice here: ", Position( 7, 1 ) );
		clog << term;

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
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "Login: ", Position( 3, 5 ) );
		term.MsgPos( "Account: ", Position( 5, 5 ) );
		cout << term;
		
		string account;
		cin >> account;

		term.MsgPos( "Password: ", Position( 6, 5 ) );
		cout << term;

		string password;
		cin >> password;

		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "Login: ", Position( 3, 5 ) );
		term.MsgPos( "Logging in...", Position( 5, 5 ) );
		cout << term;

		string command = "login " + account + " " + password;

		unique_lock<mutex> sendLock( sendMutex );
		auto job = TCPJob( command, serverName, serverPort );
		clog << "\e[1;33mPush  \e[m" << job << flush;
		sendQueue.push( job );
		clog << "\e[1;33mPushed\e[m" << flush;
		sendLock.unlock( );
		usleep( 50000 );
		
		bool loginPending = true;
		bool loginSuccess = false;

		while( loginPending )
		{
			unique_lock<mutex> resultLock( resultMutex );

			if( not resultQueue.empty( ) )
			{
				string &result = resultQueue.front( );
				
				if( result == "WA" or result == "timeout" )
				{
					loginSuccess = false;
					term.MsgPos( " failed", Position( 5, 19 ) );
					cout << term;

					WaitEnter( Position( 6, 19 ) );
				}
				else if( result.substr( 2 ) == "AC" )
				{
					loginSuccess = true;
					sessionToken = result.substr( 3, 16 );
					term.MsgPos( " success!", Position( 5, 19 ) );
					cout << term;

					WaitEnter( Position( 6, 19 ) );
				}
				else
				{
					term.MsgPos( "WTF?", Position( 5, 19 ), Format( FORMAT_BOLD, COLOR_RED, COLOR_BLACK ) );
					clog << term;

					WaitEnter( Position( 6, 19 ) );
				}

				loginPending = false;
			}

			resultLock.unlock( );

			usleep( 50000 );
		}

		return loginSuccess;
	}

	void Register( ) { }

	int main( int argc, char **argv )
	{
		term.Clear( );
		cout << term;

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

		exit = false;

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

		term.Clear( );
		clog << term;

		return 0;
	}

	void tcpSender( )
	{
		term.MsgPos( "sender: thread begin.", Position( 20, 5 ) );
		clog << term;

		int count = 0;

		while( not exit )
		{

			string result = "";

			term.MsgPos( "sender: getting mutex...", Position( 21, 5 ) );
			clog << term;

			unique_lock<mutex> sendLock( sendMutex );

			term.MsgPos( "sender: mutex get.", Position( 22, 5 ) );
			clog << term;
			
			if( not sendQueue.empty( ) )
			{
				++count;

				auto job = sendQueue.front( );
				bool isTimeout;

				term.MsgPos( "sender: job.TryTCP( ) : \"" + job.command + "\" # " + to_string( count ), Position( 23, 5 ) );
				clog << term << job;
				job.TryTCP( result, isTimeout );

				if( isTimeout )
				{
					term.MsgPos( "sender: timeout # " + to_string( count ), Position( 24, 5 ) );
					clog << term;

					sendQueue.pop( );
				}
			}

			sendLock.unlock( );
			usleep( 618033 );

			if( result != "" )
			{
				term.MsgPos( "sender: result = \"" + result + "\"", Position( 24, 5 ) );
				clog << term;

				unique_lock<mutex> resultLock( resultMutex );

				resultQueue.push( result );
			}
		}

		term.MsgPos( "sender: dude.", Position( 20, 50 ) );
	}
}

int main( int argc, char **argv )
{
	return Client::main( argc, argv );
}
