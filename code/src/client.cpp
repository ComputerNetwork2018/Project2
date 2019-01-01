#include <iostream>
#include <vector>
#include <string>

#include <cstdint>

#include "common.hpp"
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

	void Usage( int argc, char **argv )
	{
		cout << "Usage: " << argv[ 0 ] << " address:port" << endl;
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

		int choiceInt = -1;
		try
		{
			choiceInt = stoi( userChoice );
		}
		catch( exception &e )
		{
			choiceInt = -1;
		}

		if( 1 <= choiceInt and choiceInt <= 3 )
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
		term.MsgPos( "Connecting to server...", Position( 3, 5 ), Format( ) );
		cout << term;

		int errorNumber;
		sockaddr_in addr;
		bool success = connect_to( serverName, serverPort, serverFd, errorNumber, addr );

		if( success )
		{
			term.Clear( );
			term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ), Format( ) );
			term.MsgPos( "Login: ", Position( 3, 5 ), Format( ) );
			term.MsgPos( "Account: ", Position( 5, 5 ), Format( ) );
		
			string account;
			cin >> account;

			term.MsgPos( "Password: ", Position( 6, 5 ), Format( ) );

			string password;
			cin >> password;

			term.MsgPos( "Logging in...", Position( 8, 5 ), Format( ) );
		}
		else
		{
			term.MsgPos( "failed", Position( ), Format( ) );
			term.MsgPos( "Error: " + strerror( errorNumber ), Position( 4, 5 ), Format( FORMAT_BOLD, COLOR_RED, COLOR_BLACK ) );
			term.MsgPos( "Press ENTER to continue...", Position( 6, 5 ), Format( ) );
			cin >> errorNumber;

			sessionId = "";
			return false;
		}
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
			serverName = argv[ 1 ];
		}

		bool exit = false;

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
					Login( );
					break;
				case 2:
					Register( );
					break;
				case 3:
					exit = true;
					break;
			}
		}
	}
}

int main( int argc, char **argv )
{
	return Client::main( argc, argv );
}
