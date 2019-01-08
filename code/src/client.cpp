#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <mutex>

#include <cstdint>

#include <unistd.h>
#include <sys/stat.h>

#include "common.hpp"
#include "tcpJob.hpp"
#include "terminal_util.hpp"

#ifdef DEBUG

// #define DEBUG_MAIN
// #define DEBUG_LOGIN
// #define DEBUG_SENDER
// #define DEBUG_CHAT
// #define DEBUG_LIST_MENU
// #define DEBUG_FILE

#endif

#define CACHE_HEIGHT 12
#define EXIT_BAR_HEIGHT 20
#define OPTION_HEIGHT 22
#define SCREEN_HEIGHT 24
#define TAB_COL 5

using namespace std;

namespace Client
{
	string serverName;
	int serverPort;
	int serverFd;

	Terminal_Util term;

	// sender thread & main-sender communication
	void tcpSender( const int selector );
	bool exit;

	mutex sendMutex[ 2 ];
	queue<TCPJob> sendQueue[ 2 ];

	mutex resultMutex[ 2 ];
	queue<string> resultQueue[ 2 ];

	// chat manager thread & main-chatMgr communication
	void chatManager( );
	bool chatting;
	string partnerUsername;

	mutex msgMutex;
	queue<string> msgQueue;

	// file sender thread & main-fileSender communication
	void fileHandler( );

	mutex fileMutex;
	queue<string> fileQueue;

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

	bool CharacterValid( const char &c )
	{
		return ( ( c >= 'a' and c <= 'z' ) or ( c >= 'A' and c <= 'Z' ) or ( c >= '0' and c <= '9' ) or c == '_' );
	}

	bool UsernameValid( const string &username )
	{
		for( auto i : username )
		{
			if( not CharacterValid( i ) )
			{
				return false;
			}
		}

		return true;
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

	void ConnectionTimeout( int bottomColumn = SCREEN_HEIGHT )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Main Menu >", Position( 3, TAB_COL ) );
		term.MsgPos( "Connection timeout.", Position( 5, TAB_COL ), Format( FORMAT_BOLD, COLOR_YELLOW ) );
		cout << term;

		WaitEnter( Position( bottomColumn, 5 ) );
	}

	void RecvString( string &msg, int selector = 0 )
	{
		bool pending = true;
		while( pending )
		{
			lock_guard<mutex> resultLock( resultMutex[ selector ] );

			if( not resultQueue[ selector ].empty( ) )
			{
				msg = resultQueue[ selector ].front( );
				resultQueue[ selector ] .pop( );
				pending = false;
			}
		}
	}

	void SendJobToSender( const TCPJob &job, int selector = 0 )
	{
		lock_guard<mutex> sendLock( sendMutex[ selector ] );

		sendQueue[ selector ].push( job );
	}

	bool ReadFile( const std::string& filename, string &buffer )
	{
		struct stat buf;
		if( stat( filename.c_str( ), &buf ) != -1 )
		{
			ifstream file ( filename, ios::in | ios::binary );

			char charTemp = 0;
			while( file.get( charTemp ) )
			{
				buffer.push_back( charTemp );
			}
			file.close( );

			return true;
		}
		return false;
	}

	int ListMenu( const vector< string > &listToShow, const string &listType = "" )
	{
		int choiceInt = -1;

		string title, emptyMsg[ 2 ];
		if( listType == "friends" )
		{
			title = "< Friend List >";
			emptyMsg[ 0 ] = "Welp... It looks like you got no friends at all.";
			emptyMsg[ 1 ] = "SO SAD.";
		}
		else if( listType == "online_users" )
		{
			title = "< Online User List >";
			emptyMsg[ 0 ] = "Welp... It look like there is nobody online.";
			emptyMsg[ 1 ] = "WHOOPS.";
		}
		else
		{
			title = "< Santa's Gift List >";
			emptyMsg[ 0 ] = "You";
			emptyMsg[ 1 ] = "MERRY MAS AND A HAPPY NEW YEAR! HO, HO, HO!";
		}

		while( choiceInt == -1 )
		{
			term.Clear( );
			term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
			term.MsgPos( title, Position( 3, TAB_COL ) );
			cout << term;

			if( listToShow.size( ) > CACHE_HEIGHT )
			{
				// ListMenu_MultiPage( listToShow, 0 );
			}
			else if( listToShow.size( ) == 0 )
			{
				term.MsgPos( emptyMsg[ 0 ], Position( 5, TAB_COL ) );
				term.MsgPos( emptyMsg[ 1 ], Position( 6, TAB_COL ), Format( FORMAT_BOLD, COLOR_RED ) );

				term.MsgPos( "Oh, maybe you're just offline.", Position( 8, TAB_COL ) );

				WaitEnter( Position( SCREEN_HEIGHT, TAB_COL ) );
				return -1;
			}
			else
			{
				for( size_t i = 0; i < listToShow.size( ); ++i )
				{
					term.MsgPos( to_string( i + 1 ) + ( i < 10 ? ".  " : ". " ) + listToShow[ i ], Position( 5 + static_cast<int>( i ), TAB_COL ) );
				}

				cout << term;
			}

			term.MsgPos( "User to chat:     ( q to exit )", Position( 6 + static_cast<int>( listToShow.size( ) ), TAB_COL ) );
			term.MsgPos( "", Position( 6 + static_cast<int>( listToShow.size( ) ), TAB_COL + 15 ) );
			cout << term;

			string userChoice;
			cin >> userChoice;

			if( userChoice == "q" )
			{
				return -1;
			}
			else if( stoiRange( userChoice, choiceInt, 1, static_cast<int>( listToShow.size( ) ) ) )
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
		term.MsgPos( "< Main Menu >", Position( 3, TAB_COL ) );
		term.MsgPos( "1. Login", Position( 4, TAB_COL ) );
		term.MsgPos( "2. Register", Position( 5, TAB_COL ) );
		term.MsgPos( "q. Exit", Position( EXIT_BAR_HEIGHT, TAB_COL ) );
		term.MsgPos( "<Option> <ENTER> to choose an option: ", Position( OPTION_HEIGHT, 1 ) );
		cout << term;

		string userChoice;
		cin >> userChoice;

		int choiceInt;

		if( userChoice == "q" )
		{
			return 10;
		}
		else if( stoiList( userChoice, choiceInt, { 1, 2 } ) )
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
		term.MsgPos( "< Login >", Position( 3, TAB_COL ) );
		term.MsgPos( "Username: ", Position( 5, TAB_COL ) );
		cout << term;

		string username;
		cin >> username;

		term.MsgPos( "Password: ", Position( 6, TAB_COL ) );
		cout << term;

		string password;
		cin >> password;

		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Login >", Position( 3, TAB_COL ) );
		term.MsgPos( "Logging in...", Position( 5, TAB_COL ) );
		cout << term;

		string command = "login " + username + " " + Encode( password );

		SendJobToSender( TCPJob( command, serverName, serverPort ) );

		bool loginSuccess = true;
		string result;
		RecvString( result );

		if( result == "timeout" )
		{
			loginSuccess = false;
			term.MsgPos( " failed : connection timeout", Position( 5, TAB_COL + 13 ) );
		}
		else if( result.substr( 0, 2 ) == "WA" )
		{
			loginSuccess = false;
			term.MsgPos( " failed : " + result.substr( 2 ), Position( 5, TAB_COL + 13 ) );
		}
		else if( result.substr( 0, 2 ) == "AC" )
		{
			loginSuccess = true;
			sessionTokenTobe = result.substr( 3, TAB_COL + 11 );
			term.MsgPos( " success!", Position( 5, TAB_COL + 13 ) );
		}
		else
		{
			loginSuccess = false;
			term.MsgPos( " failed : unknown error [ " + result + " ]" + result.substr( 2 ), Position( 5, TAB_COL + 13 ) );
		}

		cout << term;
		WaitEnter( Position( 7, TAB_COL ) );

		return loginSuccess;
	}

	void Register( )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Register >", Position( 3, TAB_COL ) );
		term.MsgPos( "Format: [0-9a-zA-Z_]*, length is not limited.", Position( 6, TAB_COL ) );
		term.MsgPos( "Username: ", Position( 5, TAB_COL ) );
		cout << term;

		string username;
		cin >> username;

		while( not UsernameValid( username ) )
		{
			term.MsgPos( "Format: [0-9a-zA-Z_]*, length is not limited.", Position( 6, TAB_COL ) );
			term.MsgPos( "Username: ", Position( 5, TAB_COL ) );
			cout << term;

			cin >> username;
		}

		term.MsgPos( "Password: ", Position( 7, TAB_COL ) );
		cout << term;

		string password;
		cin >> password;

		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Register >", Position( 3, TAB_COL ) );
		term.MsgPos( "Registering...", Position( 5, TAB_COL ) );
		cout << term;

		string command = "signup " + username + " " + Encode( password );

		SendJobToSender( TCPJob( command, serverName, serverPort ) );

		string result;
		RecvString( result );

		if( result == "timeout" )
		{
			term.MsgPos( " failed : connection timeout", Position( 5, TAB_COL + 13 ) );
		}
		else if( result.substr( 0, 2 ) == "WA" )
		{
			term.MsgPos( " failed : " + result.substr( 2 ), Position( 5, TAB_COL + 13 ) );
		}
		else if( result.substr( 0, 2 ) == "AC" )
		{
			term.MsgPos( " success!", Position( 5, TAB_COL + 13 ) );
		}
		else
		{
			term.MsgPos( " failed : unknown error [" + result + " ]" + result.substr( 2 ), Position( 5, TAB_COL + 13 ) );
		}

		cout << term;
		WaitEnter( Position( 7, TAB_COL ) );
	}

	int Menu_Login( )
	{
		vector< bool > randomizer( 100, false );
		randomizer[ 50 ] = true;
		random_shuffle( randomizer.begin( ), randomizer.end( ) );

		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Main Menu >", Position( 3, TAB_COL ) );
		term.MsgPos( "1. Friend List", Position( 4, TAB_COL ) );
		term.MsgPos( "2. Online User List", Position( 5, TAB_COL ) );
		if( randomizer[ 0 ] )
		{
			term.MsgPos( "5. Santa's Gift List", Position( 8, TAB_COL ), Format( FORMAT_BOLD, COLOR_RED ) );
		}
		term.MsgPos( "q. Logout", Position( EXIT_BAR_HEIGHT, TAB_COL ) );
		term.MsgPos( "<Option> <Enter> to choose an option: ", Position( OPTION_HEIGHT, 1 ) );
		cout << term;

		string userChoice;
		cin >> userChoice;

		int choiceInt;

		if( userChoice == "q" )
		{
			return 10;
		}
		else if( randomizer[ 0 ] )
		{
			if( stoiList( userChoice, choiceInt, { 1, 2, 5 } ) )
			{
				return choiceInt;
			}
			else
			{
				return -1;
			}
		}
		else if( stoiList( userChoice, choiceInt, { 1, 2 } ) )
		{
			return choiceInt;
		}
		else
		{
			return -1;
		}
	}

	void main_chat( )
	{
		term.Clear( );
		term.MsgPos( "CNline: An Online Messenger", Position( 1, 1 ) );
		term.MsgPos( "< Main Menu >", Position( 3, TAB_COL ) );
		cout << term;

		chatting = true;

		thread( chatManager ).detach( );
		thread( fileHandler ).detach( );

		string msg = "";
		getchar( );

		while( chatting )
		{
			if( msg != "" )
			{
				lock_guard<mutex> msgLock( msgMutex );
				msgQueue.push( msg );
			}

			term.Fill( Position( EXIT_BAR_HEIGHT, 0 ), Position( SCREEN_HEIGHT + 1, 300 ), Format( ), ' ' );
			term.MsgPos( "/a /r : add friend / remove friend", Position( SCREEN_HEIGHT - 2, TAB_COL ) );
			term.MsgPos( "/f <filename> : file transfer", Position( SCREEN_HEIGHT - 1, TAB_COL ) );
			term.MsgPos( "/q : quit.", Position( SCREEN_HEIGHT, TAB_COL ) );
			term.MsgPos( "> ", Position( EXIT_BAR_HEIGHT, TAB_COL ) );
			cout << term;
			getline( cin, msg );

			stringstream msgStream( msg );
			string temp;
			msgStream >> temp;
#ifdef DEBUG_FILE
			clog << "msg = " << msg << " temp = " << temp << endl;
#endif
			if( temp[ 0 ] == '/' )
			{
				if( temp.length( ) == 2 )
				{
					switch( temp[ 1 ] )
					{
						case 'q':
						{
							chatting = false;
							break;
						}
						case 'f':
						{
							lock_guard<mutex> fileLock( fileMutex );

							temp = msgStream.str( ).substr( static_cast<size_t>( msgStream.tellg( ) ) + 1 );
							fileQueue.push( temp );
#ifdef DEBUG_FILE
							clog << "pushed filename " << temp << endl;
#endif
							break;
						}
						case 'a':
						{
							break;
						}
						case 'r':
						{
							break;
						}
						default:
						{
							break;
						}
					}
				}

				msg = "";
			}
		}
	}

	void ShowList( const string &listType = "" )
	{
		string command = ( listType == "" ? "friends" : listType ) + " " + sessionToken;

		SendJobToSender( TCPJob( command, serverName, serverPort ) );

		string result;
		RecvString( result );

		stringstream resultStream( result );
		vector< string > list( 0 );

		resultStream >> result; // get rid of the "AC"/"timeout" msg

		if( result == "timeout" )
		{
			ConnectionTimeout( 14 );
		}
		else
		{
			resultStream >> result; // get rid of N.

			while( not resultStream.eof( ) )
			{
				resultStream >> result;
				list.push_back( result );
			}
		}

		int choiceInt = ListMenu( list, listType );
		while( choiceInt != -1 )
		{
			partnerUsername = list[ choiceInt - 1 ];
			main_chat( );
			choiceInt = ListMenu( list, listType );
		}
	}

	void main_login( )
	{
		term.Clear( );
		cout << term;

		while( login )
		{
			int userChoice = Menu_Login( ); // 1 = friend list, 2 = online list, 5 = santa's gift lift, 9 = logout

			switch( userChoice )
			{
				case 1:
				{
					ShowList( "friends" );
					break;
				}
				case 2:
				{
					ShowList( "online_users" );
					break;
				}
				case 5:
				{
					ShowList( );
				}
				case 10:
				{
					login = false;
					break;
				}
				default:
				{
					term.MsgPos( "Unknown Option.", Position( OPTION_HEIGHT, TAB_COL + 33 ), Format( FORMAT_BOLD ) );
					cout << term;

					WaitEnter( Position( SCREEN_HEIGHT, 1 ) );
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

		thread( tcpSender, 0 ).detach( );
		thread( tcpSender, 1 ).detach( );

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
				case 10:
				{
					exit = true;
					break;
				}
				default:
				{
					term.MsgPos( "Unknown Option.", Position( OPTION_HEIGHT, TAB_COL + 13 ), Format( FORMAT_BOLD ) );
					cout << term;

					WaitEnter( Position( SCREEN_HEIGHT, 1 ) );
					break;
				}
			}
		}

		term.Clear( );
		cout << term;

		return 0;
	}

	void tcpSender( int selector )
	{
#ifdef DEBUG_SENDER
		term.MsgPos( "sender: thread begin.", Position( 20, TAB_COL ) );
		clog << term;
#endif
		while( not exit )
		{
			unique_lock<mutex> sendLock( sendMutex[ selector ], defer_lock );
			unique_lock<mutex> resultLock( resultMutex[ selector ], defer_lock );
			lock( sendLock, resultLock );

			if( not sendQueue[ selector ].empty( ) )
			{
				auto &job = sendQueue[ selector ].front( );
				string result = "";
				bool isTimeout;
#ifdef DEBUG_SENDER
				term.MsgPos( "sender: job.TryTCP( ) : \"" + job.command + "\" # " + to_string( job.id ), Position( 30, TAB_COL ) );
				clog << term << job;
#endif
				job.TryTCP( result, isTimeout );

				if( result != "" )
				{
#ifdef DEBUG_SENDER
					term.MsgPos( "sender: result = \"" + result + "\"", Position( 34, TAB_COL ) );
					clog << term;
#endif
					resultQueue[ selector ].push( result );

					sendQueue[ selector ].pop( );
				}
				else if( isTimeout )
				{
#ifdef DEBUG_SENDER
					term.MsgPos( "sender: timeout # " + to_string( job.id ), Position( 31, TAB_COL ) );
					clog << term;
#endif
					sendQueue[ selector ].pop( );
				}
			}

			sendLock.unlock( );
			resultLock.unlock( );
		}
#ifdef DEBUG_SENDER
		term.MsgPos( "sender: dude.", Position( 30, TAB_COL + 45 ) );
#endif
	}

	string ParseMsg( const string &rawMsg )
	{
		string msg = "";

		auto begin = rawMsg.find( "from:" ) + 5;
		auto end = rawMsg.find_first_of( ',', begin );

		msg += "[ " + rawMsg.substr( begin, end - begin ) + " ] : ";

		begin = rawMsg.find( "text:", end ) + 5;
		end = rawMsg.find_first_of( '}', begin );

		msg += Decode( rawMsg.substr( begin, end - begin ) );

		return msg;
	}

	void getMsgFromId( const string &msgId, string &msg )
	{
		string command = "get_message " + sessionToken + " " + msgId;
		SendJobToSender( TCPJob( command, serverName, serverPort ) );

		string result;
		RecvString( result );

		if( result == "timeout" )
		{
			msg = "";
		}
		else
		{
			msg = result.substr( 3 );
		}
	}

	void getMsgs( const string &rootMsgId, const int count, const bool newer, vector<string> &resultList )
	{
		string command = ( newer ? "next_messages " : "prev_messages " ) + sessionToken + " " + rootMsgId + " " + to_string( count );
		SendJobToSender( TCPJob( command, serverName, serverPort ) );

		string result;
		RecvString( result );

		stringstream resultStream( result );

		resultStream >> result;

		if( result == "timeout" )
		{
			resultList.clear( );
		}
		else
		{
			resultStream >> result; // get rid of N.

			while( not resultStream.eof( ) )
			{
				resultStream >> result;
				resultList.push_back( result );
			}
		}
	}

	void getMsgs( const string &rootMsgId, vector<string> &resultList )
	{
		vector<string> temp( 0 );

		getMsgs( rootMsgId, CACHE_HEIGHT - 1, false, temp );
		for( auto i = temp.rbegin( ); i != temp.rend( ); ++i )
		{
			resultList.push_back( *i );
		}

		resultList.push_back( rootMsgId );

		if( resultList.size( ) < CACHE_HEIGHT )
		{
			temp.clear( );
			getMsgs( rootMsgId, CACHE_HEIGHT - static_cast<int>( resultList.size( ) ), true, temp );
			for( auto i = temp.begin( ); i != temp.end( ); ++i )
			{
				resultList.push_back( *i );
			}
		}
	}

	void chatManager( )
	{
		string rootMsgId = "";
		deque< pair< string, string > > msgCache( 0 );

		while( chatting )
		{
			string msg = "", command;
			{
				lock_guard<mutex> msgLock( msgMutex );

				if( not msgQueue.empty( ) )
				{
					msg = msgQueue.front( );
					msgQueue.pop( );
				}
			}

			if( msg == "" )
			{
				command = "last_message " + sessionToken + " " + partnerUsername;
			}
			else
			{
				command = "send_message " + sessionToken + " " + partnerUsername + " " + Encode( msg );
			}

			SendJobToSender( TCPJob( command, serverName, serverPort ) );

			string result;
			RecvString( result );

			if( result == "timeout" )
			{
				// empty
			}
			else if( result == "AC" )
			{
				rootMsgId = "";
			}
			else
			{
				rootMsgId = result.substr( 3, TAB_COL + 11 );
			}

			if( rootMsgId != "" )
			{
				vector<string> resultList( 0 );
				getMsgs( rootMsgId, resultList );
#ifdef DEBUG_CHAT
				clog << "size = " << resultList.size( ) << " root = " << rootMsgId << endl;
#endif
				bool push = msgCache.empty( );
				for( auto &i : resultList )
				{
#ifdef DEBUG_CHAT
					clog << i + " ";
#endif
					if( push )
					{
#ifdef DEBUG_CHAT
						clog << "<-p ";
#endif
						msgCache.push_back( pair<string, string>( i, "" ) );
						getMsgFromId( msgCache.back( ).first, msgCache.back( ).second );

						if( msgCache.size( ) > CACHE_HEIGHT )
						{
							msgCache.pop_front( );
						}
					}

					if( i == msgCache.back( ).first )
					{
						push = true;
					}
#ifdef DEBUG_CHAT
					clog << endl;
#endif
				}

				term.Fill( Position( EXIT_BAR_HEIGHT - CACHE_HEIGHT, 1 ), Position( EXIT_BAR_HEIGHT - 2, 300 ), Format( ), ' ' );
				for( size_t i = 0; i < msgCache.size( ); ++i )
				{
					term.MsgPos( ParseMsg( msgCache[ i ].second ), Position( EXIT_BAR_HEIGHT - 1 - static_cast<int>( msgCache.size( ) - i ), 5 ) );
				}

				cout << "\e[s" << flush;
				cout << term;
				cout << "\e[u" << flush;

				if( msgCache.empty( ) )
				{
					rootMsgId = "";
				}
				else
				{
					rootMsgId = msgCache.back( ).first;
				}
				// rootMsgId = msgCache.back( ).second.substr( msgCache.back( ).second.find_last_of( ',' ) + 1 );
			}

			sleep( 1 );
		}
	}

	void fileHandler( )
	{
		while( chatting )
		{
			string filename = "";
			string buffer = "";
			bool fileExists;

			{
				lock_guard<mutex> fileLock( fileMutex );

				if( not fileQueue.empty( ) )
				{
					filename = fileQueue.front( );
					fileQueue.pop( );
#ifdef DEBUG_FILE
					clog << "file: get filename: " << filename << endl;
#endif
				}
			}

			fileExists = ReadFile( filename, buffer );

			if( fileExists )
			{
				string command = "file_request " + sessionToken + " " + partnerUsername + " " + to_string( buffer.length( ) ) + " " + filename;
				SendJobToSender( TCPJob( command, serverName, serverPort ), 1 );

				string result;
				RecvString( result, 1 );

				string fileID = "";
				if( result.substr( 0, 2 ) == "AC" )
				{
					fileID = result.substr( 3, 16 );
				}

				if( fileID != "" )
				{
					command = "send_file " + sessionToken + " " + fileID + " " + buffer;
					SendJobToSender( TCPJob( command, serverName, serverPort, 300000 ), 1 );

					RecvString( result, 1 );
				}
			}
			// send file above, receive file below
			{
				string command = "check_file_request " + sessionToken;
				SendJobToSender( TCPJob( command, serverName, serverPort ), 1 );

				string result;
				RecvString( result, 1 );

				stringstream resultStream( result );

				string fileID = "";
				resultStream >> result; // get rid of "AC" msg.

				if( not resultStream.eof( ) )
				{
					resultStream >> fileID;
					resultStream >> result; // get rid of the sender username.
					resultStream >> result; // get rid of the buffer length.
					filename = resultStream.str( ).substr( static_cast<size_t>( resultStream.tellg( ) ) + 1 );
				}

				if( fileID != "" )
				{
					command = "receive_file " + sessionToken + " " + fileID;
					SendJobToSender( TCPJob( command, serverName, serverPort, 300000 ), 1 );

					RecvString( result, 1 );

					ofstream file( filename, ios::out | ios::trunc | ios::binary );
					file.write( result.substr( 3 ).c_str( ), result.length( ) - 3 );
					file.close( );	
				}
			}

			sleep( 1 );
		}
	}
}

int main( int argc, char **argv )
{
	return Client::main( argc, argv );
}
