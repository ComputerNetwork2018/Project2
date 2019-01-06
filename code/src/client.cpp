#include <iostream>
#include <sstream>
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

#ifdef DEBUG

#define DEBUG_MAIN
#define DEBUG_LOGIN
#define DEBUG_SENDER

#endif

#define MENU_PAGE_SIZE 20

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
	vector<TCPJob> sendList;

	mutex resultMutex;
	queue<string> resultQueue;

	// login variables
	bool login = false;
	string sessionToken;

	void Usage( char **argv )
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

	bool stoiList( const string &msg, int &result, const vector< int > &range )
	{
		try
		{
			result = stoi( msg );
		}
		catch( exception &e )
		{
			return false;
		}

		for( auto i : range )
		{
			if( result == i )
			{
				return true;
			}
		}

		return false;
	}

	void WaitEnter( const Position &pos = Position( ), const Format &format = Format( ), const string &msg = "press ENTER to contunue." )
	{
		term.MsgPos( msg, pos, format );
		cout << term;

		int temp = getchar( );

		do
		{
			temp = getchar( );
		} while( temp != '\n' );
	}

	int ListMenu( const vector< string > &listToShow, const string &title )
	{
		int choiceInt = -1;

		while( choiceInt == -1 )
		{
			term.Clear( );
			term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
			term.MsgPos( title, Position( 3, 5 ) );
			cout << term;

			if( listToShow.size( ) > MENU_PAGE_SIZE )
			{
				// ListMenu_MultiPage( listToShow, 0 );
			}
			else
			{
				for( size_t i = 0; i < listToShow.size( ); ++i )
				{
					term.MsgPos( to_string( i + 1 ) + ( i < 10 ? ".  " : ". " ) + listToShow[ i ], Position( 5 + static_cast<int>( i ), 5 ) );
				}

				cout << term;
			}

			term.MsgPos( "User to chat:     ( e to exit )", Position( 6 + static_cast<int>( listToShow.size( ) ), 5 ) );
			term.MsgPos( "", Position( 6 + static_cast<int>( listToShow.size( ) ), 20 ) );
			cout << term;

			string userChoice;
			cin >> userChoice;

			if( userChoice == "e" )
			{
				return -1;
			}
			else if( stoiRange( userChoice, choiceInt, 1, static_cast< int >( listToShow.size( ) ) ) )
			{
				return choiceInt;
			}
			else
			{
				choiceInt = -1;
			}
		}

		return choiceInt;
	}

	int Menu_Logout( )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Main Menu >", Position( 3, 5 ) );
		term.MsgPos( "1. Login", Position( 4, 5 ) );
		term.MsgPos( "2. Register", Position( 5, 5 ) );
		term.MsgPos( "9. Exit", Position( 12, 5 ) );
		term.MsgPos( "<Num> <ENTER> to choose an option: ", Position( 14, 1 ) );
		cout << term;

		string userChoice;
		cin >> userChoice;

		int choiceInt;
		
		if( stoiList( userChoice, choiceInt, { 1, 2, 9 } ) )
		{
			return choiceInt;
		}
		else
		{
			return -1;
		}
	}

	bool Login( string &sessionTokenTobe )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Login >", Position( 3, 5 ) );
		term.MsgPos( "Username: ", Position( 5, 5 ) );
		cout << term;
		
		string username;
		cin >> username;

		term.MsgPos( "Password: ", Position( 6, 5 ) );
		cout << term;

		string password;
		cin >> password;

		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Login >", Position( 3, 5 ) );
		term.MsgPos( "Logging in...", Position( 5, 5 ) );
		cout << term;

		string command = "login " + username + " " + password;

		unique_lock<mutex> sendLock( sendMutex );
		TCPJob job = TCPJob( command, serverName, serverPort );
		sendList.push_back( job );
		sendLock.unlock( );
		
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Login >", Position( 3, 5 ) );
		term.MsgPos( "Logging in...", Position( 5, 5 ) );
		cout << term;

		usleep( 50000 );
		
		bool loginPending = true;
		bool loginSuccess = false;

		while( loginPending )
		{
			unique_lock<mutex> resultLock( resultMutex );

			if( not resultQueue.empty( ) )
			{
				string &result = resultQueue.front( );
				
				if( result == "timeout" )
				{
					loginSuccess = false;
					term.MsgPos( " failed : connection timeout", Position( 5, 18 ) );
				}
				else if( result.substr( 0, 2 ) == "WA" )
				{
					loginSuccess = false;
					term.MsgPos( " failed : " + result.substr( 2 ), Position( 5, 18 ) );
				}
				else if( result.substr( 0, 2 ) == "AC" )
				{
					loginSuccess = true;
					sessionTokenTobe = result.substr( 3, 16 );
					term.MsgPos( " success!", Position( 5, 18 ) );
				}
				else
				{
					loginSuccess = false;
					term.MsgPos( " failed : unknown error [ " +  result +  " ]" + result.substr( 2 ), Position( 5, 18 ) );
				}

				cout << term;

				loginPending = false;
				resultQueue.pop( );

				WaitEnter( Position( 7, 5 ) );
			}

			resultLock.unlock( );
			usleep( 50000 );
		}

		return loginSuccess;
	}

	void Register( )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Register >", Position( 3, 5 ) );
		term.MsgPos( "Username: ", Position( 5, 5 ) );
		cout << term;

		string username;
		cin >> username;

		term.MsgPos( "Password: ", Position( 6, 5 ) );
		cout << term;

		string password;
		cin >> password;

		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Register >", Position( 3, 5 ) );
		term.MsgPos( "Registering...", Position( 5, 5 ) );
		cout << term;

		string command = "signup " + username + " " + password;

		unique_lock<mutex> sendLock( sendMutex );
		sendList.push_back( TCPJob( command, serverName, serverPort ) );
		sendLock.unlock( );

		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Register >", Position( 3, 5 ) );
		term.MsgPos( "Registering...", Position( 5, 5 ) );
		cout << term;

		usleep( 50000 );

		bool registerPending = true;
		while( registerPending )
		{
			unique_lock< mutex > resultLock( resultMutex );

			if( not resultQueue.empty( ) )
			{
				string &result = resultQueue.front( );

				if( result == "timeout" )
				{
					term.MsgPos( " failed : connection timeout", Position( 5, 18 ) );
				}
				else if( result.substr( 0, 2 ) == "WA" )
				{
					term.MsgPos( " failed : " + result.substr( 2 ), Position( 5, 18 ) );
				}
				else if( result.substr( 0, 2 ) == "AC" )
				{
					term.MsgPos( " success!", Position( 5, 18 ) );
				}
				else
				{
					term.MsgPos( " failed : unknown error [ " + result + " ]" + result.substr( 2 ), Position( 5, 18 ) );
				}

				cout << term;

				registerPending = false;
				resultQueue.pop( );

				WaitEnter( Position( 7, 5 ) );
			}

			resultLock.unlock( );
			usleep( 50000 );
		}
	}

	int Menu_Login( )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Main Menu >", Position( 3, 5 ) );
		term.MsgPos( "1. Friend List", Position( 4, 5 ) );
		term.MsgPos( "2. Online User List", Position( 5, 5 ) );
		term.MsgPos( "9. Logout", Position( 12, 5 ) );
		term.MsgPos( "<Num> <Enter> to choose an option: ", Position( 14, 1 ) );
		cout << term;

		string userChoice;
		cin >> userChoice;

		int choiceInt;

		if( stoiList( userChoice, choiceInt, { 1, 2, 9 } ) )
		{
			return choiceInt;
		}
		else
		{
			return -1;
		}
	}

	void FriendList( )
	{
		string command = "friends " + sessionToken;
		
		unique_lock<mutex> sendLock( sendMutex );

		sendList.push_back( TCPJob( command, serverName, serverPort ) );

		sendLock.unlock( );

		bool friendListPending = true;
		vector< string > friendList( 0 );

		while( friendListPending )
		{
			unique_lock<mutex> resultLock( resultMutex );

			if( not resultQueue.empty( ) )
			{
				stringstream resultStream( resultQueue.front( ) );
				string result;

				resultStream >> result; // get rid of the "AC" msg.

				while( not resultStream.eof( ) )
				{
					resultStream >> result;
					friendList.push_back( result );
				}

				friendListPending = false;
			}

			resultLock.unlock( );
		}


	}

	void main_login( )
	{
		term.Clear( );
		cout << term;

		while( login )
		{
			int userChoice = Menu_Login( ); // 1 = friend list, 2 = online list, 9 = logout

			switch( userChoice )
			{
				case 1:
				{
					FriendList( );
					break;
				}
				case 2:
				{
					// OnlineList( );
					break;
				}
				case 9:
				{
					login = false;
					break;
				}
				default:
				{
					term.MsgPos( "Unknown Option.", Position( 14, 36 ), Format( FORMAT_BOLD ) );
					cout << term;

					WaitEnter( Position( 15, 1 ) );
					break;
				}
			}
		}
	}

	int main( int argc, char **argv )
	{
		term.Clear( );
		cout << term;

		if( argc != 2 )
		{
			Usage( argv );
			return 0;
		}
		else
		{
			string server = string( argv[ 1 ] );
			serverName = server.substr( 0, server.find_first_of( ':' ) );
			if( not stoiRange( server.substr( server.find_first_of( ':' ) + 1 ), serverPort, 0, 65535 ) )
			{
				Usage( argv );
				return 0;
			}
		}

		exit = false;

		thread tcpThread( tcpSender );

		while( not exit )
		{
			int userChoice = Menu_Logout( ); // 1 = login, 2 = reg, 9 = exit

			switch( userChoice )
			{
				case 1:
				{
					bool loginSuccess = Login( sessionToken );

					if( loginSuccess )
					{
						login = true;
						main_login( );
					}

					break;
				}
				case 2:
				{
					Register( );
					break;
				}
				case 9:
				{
					exit = true;
					break;
				}
				default:
				{
					term.MsgPos( "Unknown Option.", Position( 14, 36 ), Format( FORMAT_BOLD ) );
					cout << term;

					WaitEnter( Position( 15, 1 ) );
					break;
				}
			}
		}

		tcpThread.join( );

		term.Clear( );
		clog << term;

		return 0;
	}

	void tcpSender( )
	{
#ifdef DEBUG_SENDER
		term.MsgPos( "sender: thread begin.", Position( 20, 5 ) );
		clog << term;
#endif

		while( not exit )
		{
			unique_lock<mutex> sendLock( sendMutex );
			
			while( not sendList.empty( ) )
			{
				for( auto it = sendList.begin( ); it != sendList.end( ); ++it )
				{
					auto &job = *it;
					string result = "";
					bool isTimeout;

#ifdef DEBUG_SENDER
					term.MsgPos( "sender: job.TryTCP( ) : \"" + job.command + "\" # " + to_string( job.id ), Position( 20, 5 ) );
					clog << term << job;
#endif
					
					job.TryTCP( result, isTimeout );

					if( result != "" )
					{
#ifdef DEBUG_SENDER
						term.MsgPos( "sender: result = \"" + result + "\"", Position( 24, 5 ) );
						clog << term;
#endif

						unique_lock<mutex> resultLock( resultMutex );

						resultQueue.push( result );
						
						sendList.erase( it );
						break;
					}

					if( isTimeout )
					{
#ifdef DEBUG_SENDER
						term.MsgPos( "sender: timeout # " + to_string( job.id ), Position( 21, 5 ) );
						clog << term;
#endif

						sendList.erase( it );
						break;
					}
				}
			}

			sendLock.unlock( );
			usleep( 618033 );
		}
#ifdef DEBUG_SENDER
		term.MsgPos( "sender: dude.", Position( 20, 50 ) );
#endif
	}
}

int main( int argc, char **argv )
{
	return Client::main( argc, argv );
}
