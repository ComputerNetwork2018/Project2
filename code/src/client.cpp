#include <iostream>
#include <vector>
#include <string>

#include "terminal_util.hpp"

using namespace std;

namespace Client
{
	Terminal_Util term( );
	bool login = false;

	vector< Id > onlineList;
	vector< Id > friendList;

	int Main_Logout( )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger" );
		term.MsgPos( "1. Login", Position( 3, 5 ) );
		term.MsgPos( "2. Register", Position( 4, 5 ) );
		term.MsgPos( "3. Exit", Position( 5, 5 ) );
		term.MsgPos( "Put your choice here: ", Position( 6, 1 ) );

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

	int main( int argc, char **argv )
	{
		bool exit = false;

		while( not exit )
		{
			int userChoice = Main_Logout( ); // 1 = login, 2 = reg, 3 = exit
			while( userChoice == -1 )
			{
				Main_Logout( );
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