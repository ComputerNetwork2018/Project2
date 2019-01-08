#include<bits/stdc++.h>
#include<thread>
#include<mutex>
#include<chrono>
#include"common.hpp"
using namespace std;
using namespace chrono;
namespace DataBase
{
	namespace // private members
	{
		class Message
		{
		private:
			string from, to;
			string text;
			string prev = "", next = "";
		public:
			Message( )
			{
			}
			Message( const string &_from, const string &_to, const string &_text ) :from( _from ), to( _to ), text( _text )
			{
			}
			string to_json( )const
			{
				return "{from:" + from + ",to:" + to + ",text:" + text + "}";
			}
			bool belong_to( const string &name )const
			{
				return name == from || name == to;
			}
			void SetPrev( const string &message_id )
			{
				assert( prev == "" ); prev = message_id;
			}
			void SetNext( const string &message_id )
			{
				assert( next == "" ); next = message_id;
			}
			string GetPrev( )const
			{
				return prev;
			}
			string GetNext( )const
			{
				return next;
			}
		};
		mt19937 Rand( time( NULL ) );
		string generate_token( )
		{
			const int len = 16;
			string ans = "";
			for( int i = 0; i < len; i++ )
			{
				const unsigned v = (unsigned) Rand( ) & 0xf;
				if( v <= 9 )ans.push_back( (char) ( '0' + v ) );
				else ans.push_back( (char) ( 'a' + ( v - 10 ) ) );
			}
			cerr << ans << endl;
			return ans;
		}
		map<string, string>usernames;// username, password
		map<string, pair<string, time_point<steady_clock>>>session_tokens;// session_token, (username, last_online)
		map<string, set<string>>friends;// username, friends
		map<string, Message>messages;// message_id, message_content
		map<pair<string, string>, string>last_message;// <user1, user2>, message_id
		void ConnectMessage( const string &message_id1, const string &message_id2 )
		{
			auto it1 = messages.find( message_id1 ),
				it2 = messages.find( message_id2 );
			assert( it1 != messages.end( ) && it2 != messages.end( ) );
			it1->second.SetNext( message_id2 );
			it2->second.SetPrev( message_id1 );
		}
		map<pair<string, string>, string>::iterator GetLastMessage( const string &user1, const string &user2 )
		{
			if( user1 > user2 )return GetLastMessage( user2, user1 );
			const auto &key = make_pair( user1, user2 );
			return last_message.find( key );
		}
		void UpdateLastMessage( const string &user1, const string &user2, const string &message_id )
		{
			const auto it = GetLastMessage( user1, user2 );
			if( it != last_message.end( ) )
			{
				ConnectMessage( it->second, message_id );
				it->second = message_id;
			}
			else
			{
				const auto &key = make_pair( min( user1, user2 ), max( user1, user2 ) );
				last_message[ key ] = message_id;
			}
		}
		string InsertMessage( const string &from, const string &to, const string &text )
		{
			const auto msg = Message( from, to, text );
			const auto message_id = generate_token( );
			messages[ message_id ] = msg;
			UpdateLastMessage( from, to, message_id );
			return message_id;
		}
		bool IsUserExists( const string &username )
		{
			return usernames.find( username ) != usernames.end( );
		}
		bool TryGetUsername( const string &session_token, string &username )
		{
			const auto it = session_tokens.find( session_token );
			if( it == session_tokens.end( ) )return false;
			it->second.second = steady_clock::now( );
			return username = it->second.first, true;
		}
		double TimeBetween( const time_point<steady_clock>&time_sooner, const time_point<steady_clock>&time_latter = steady_clock::now( ) )
		{
			return (double) ( duration_cast<microseconds>( time_latter - time_sooner ).count( ) / 1000 );
		}
	}
	mutex mutex_global;
	bool TrySignup( const string &username, const string &password, string &msg )
	{
		lock_guard<mutex>guard( mutex_global );
		if( IsUserExists( username ) )return msg = "user \"" + username + "\" is already signed up", false;
		usernames[ username ] = password;
		return true;
	}
	bool TryLogin( const string &username, const string &password, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		const auto it = usernames.find( username );
		if( it == usernames.end( ) )return response = "user \"" + username + "\" not existed", false;
		if( it->second != password )return response = "wrong password", false;
		const string &token = generate_token( );
		session_tokens[ token ] = make_pair( username, steady_clock::now( ) );
		return response = token, true;
	}
	bool TryGetOnlineUsers( const string &session_token, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		vector<string>ans;
		for( const auto p : session_tokens )if( p.second.first != username )ans.push_back( p.second.first );
		response = to_string( ans.size( ) );
		for( const string &name : ans )response += " " + name;
		return true;
	}
	bool TryGetFriends( const string &session_token, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		const set<string>&ans = friends[ username ];
		response = to_string( ans.size( ) );
		for( const string &name : ans )response += " " + name;
		return true;
	}
	bool TryAddFriend( const string &session_token, const string &friend_name, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		if( !IsUserExists( friend_name ) )return response = "friend \"" + friend_name + "\" not exists", false;
		set<string>&fs = friends[ username ];
		if( fs.find( friend_name ) != fs.end( ) )return response = "you're already friends", false;
		return fs.insert( friend_name ), true;
	}
	bool TryRemoveFriend( const string &session_token, const string &friend_name, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		set<string>&fs = friends[ username ];
		const auto it = fs.find( friend_name );
		if( it == fs.end( ) )return response = "you're not friends", false;
		return fs.erase( it ), true;
	}
	bool TryGetMessage( const string &session_token, const string &message_id, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		const auto &it = messages.find( message_id );
		if( it == messages.end( ) )return response = "non-existing message_id", false;
		const Message &msg = it->second;
		if( !msg.belong_to( username ) )return response = "permission denied", false;
		return response = msg.to_json( ), true;
	}
	bool TrySendMessage( const string &session_token, const string &partner_name, const string &text, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		if( !IsUserExists( partner_name ) )return response = "partner \"" + partner_name + "\" not existed", false;
		const string &message_id = InsertMessage( username, partner_name, text );
		return response = message_id, true;
	}
	bool TryGetLastMessage( const string &session_token, const string &partner_name, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		const auto it = GetLastMessage( username, partner_name );
		if( it == last_message.end( ) )return response = "", true;
		else return response = it->second, true;
	}
	namespace
	{
		bool TryGetNextMessages( const string &username, const string &message_id, const int desired_count, string &response )
		{
			const auto it = messages.find( message_id );
			if( it == messages.end( ) )return response = "non-existing message_id", false;
			auto msg = it->second;
			if( !msg.belong_to( username ) )return response = "permission denied", false;
			vector<string>ans;
			for( int i = 0; i < desired_count; i++ )
			{
				assert( msg.belong_to( username ) );
				const string &next_id = msg.GetNext( );
				if( next_id == "" )break;
				const auto iu = messages.find( next_id );
				assert( iu != messages.end( ) );
				msg = iu->second;
				ans.push_back(next_id);
			}
			response = to_string( ans.size( ) );
			for( const string &id : ans )response += " " + id;
			return true;
		}
		bool TryGetPreviousMessages( const string &username, const string &message_id, const int desired_count, string &response )
		{
			const auto it = messages.find( message_id );
			if( it == messages.end( ) )return response = "non-existing message_id", false;
			auto msg = it->second;
			if( !msg.belong_to( username ) )return response = "permission denied", false;
			vector<string>ans;
			for( int i = 0; i < desired_count; i++ )
			{
				assert( msg.belong_to( username ) );
				const string &prev_id = msg.GetPrev( );
				if( prev_id == "" )break;
				const auto iu = messages.find( prev_id );
				assert( iu != messages.end( ) );
				msg = iu->second;
				ans.push_back(prev_id);
			}
			response = to_string( ans.size( ) );
			for( const string &id : ans )response += " " + id;
			return true;
		}
	}
	bool TryGetNextMessages( const string &session_token, const string &message_id, const string &desired_count, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		int n;
		try
		{
			n = stoi( desired_count );
		}
		catch( ... )
		{
			return response = "desired_count isn't a number", false;
		}
		if( n < 1 || n>100 )return response = "supported range of desired_count is 1~100, got " + to_string( n ), false;
		return TryGetNextMessages( username, message_id, n, response );
	}
	bool TryGetPreviousMessages( const string &session_token, const string &message_id, const string &desired_count, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		int n;
		try
		{
			n = stoi( desired_count );
		}
		catch( ... )
		{
			return response = "desired_count isn't a number", false;
		}
		if( n < 1 || n>100 )return response = "supported range of desired_count is 1~100, got " + to_string( n ), false;
		return TryGetPreviousMessages( username, message_id, n, response );
	}
	bool TryOnlineCheck( const string &session_token, const string &name, string &response )
	{
		lock_guard<mutex>guard( mutex_global );
		string username;
		if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		if( !IsUserExists( name ) )return response = "user \"" + name + "\" not exists", false;
		for( const auto p : session_tokens )if( p.second.first == name && TimeBetween( p.second.second ) < 1000 * 60 )return response = "online", true;
		return response = "offline", true;
	}
	namespace
	{
		map<string, string>file_request_info; // receiver_name, (sender_name+filesize)
		map<string, string>file_request_status; // receiver_name, status
		map<string, int>file_ids;
		map<string, string>file_contents;
		// must implement thread-safety
		bool TryFileRequest( const string &username, const string &receiver_name, const int file_size,const string &file_name, string &response )
		{
			{
				///////////////////////// cerr<<"1. sender's info has been put up"<<endl;
				lock_guard<mutex>guard( mutex_global );
				if( file_request_info.find( receiver_name ) != file_request_info.end( ) )return response = "receiver busy, please try again later", false;
				file_request_info[ receiver_name ] = username + " " + to_string( file_size )+" "+file_name;
				file_request_status[ receiver_name ] = "=====requested=====";
			}
			//////////////////////////// cerr<<"2. sender waits for receiver to admit the request. HOW: watch file_request_status"<<endl;
			for( const auto start_clock = steady_clock::now( ); TimeBetween( start_clock ) < 1000 * 5;) // wait for 5 seconds
			{
				usleep( 1 );
				lock_guard<mutex>guard( mutex_global );
				if( file_request_status[ receiver_name ] == "=====responded=====" )goto index_got_responded;
			}
			{
				lock_guard<mutex>guard( mutex_global );
				file_request_info.erase( receiver_name );
				file_request_status.erase( receiver_name );
			}
			return response = "no response from receiver, timed out after 5 seconds", false;
		index_got_responded:;
			//////////////////////////// cerr<<"4. sender found the request has been admitted"<<endl;
			const string &file_id = generate_token( );
			{
				//////////////////////// cerr<<"5. file_id has been generated, associated with file_size, and file_id has given to sender"<<endl;
				lock_guard<mutex>guard( mutex_global );
				file_request_status[ receiver_name ] = file_id;
				file_ids[ file_id ] = file_size;
			}
			return response = file_id, true;
		}
	}
	bool TryFileRequest( const string &session_token, const string &receiver_name, const string &file_size, const string &file_name, string &response )
	{
		string username;
		{
			lock_guard<mutex>guard( mutex_global );
			if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
		}
		int n;
		try
		{
			n = stoi( file_size );
		}
		catch( ... )
		{
			return response = "file_size isn't a number", false;
		}
		if( n < 0 || n>1000000000 )return response = "supported range of file_size is 0~1000000000, got " + to_string( n ), false;
		// below function must implement thread-safety
		return TryFileRequest( username, receiver_name, n,file_name, response );
	}
	bool TryCheckFileRequest( const string &session_token, string &response )
	{
		// cerr<<"TryCheckFileRequest"<<endl;
		string username;
		string sender_info;
		{
			// cerr<<"try locking...";
			lock_guard<mutex>guard( mutex_global );
			// cerr<<" OK"<<endl;
			if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
			// cerr<<"username="<<username<<endl;
			const auto it = file_request_info.find( username );
			if( it == file_request_info.end( ) )return response = "", true;
			/////////////////////// cerr<<"3. receiver admitted the file request and discard sender's info"<<endl;
			sender_info = it->second;
			file_request_info.erase( it );
			assert( file_request_status[ username ] == "=====requested=====" );
			file_request_status[ username ] = "=====responded=====";
		}
		string file_id;
		//////////////////////////// cerr<<"6. receiver waits for file_id to be generated. HOW: watch file_request_status until changed into file_id"<<endl;
		for( const auto start_clock = steady_clock::now( ); TimeBetween( start_clock ) < 1000 * 5;) // wait for 5 seconds
		{
			usleep( 1 );
			lock_guard<mutex>guard( mutex_global );
			file_id = file_request_status[ username ];
			if( file_id != "=====responded=====" )
			{
				//////////////////// cerr<<"7. receiver gets file_id, now sender has been moved to next stage, so receiver close the communication channel: file_request_status.erase()"<<endl;
				file_request_status.erase( username );
				return response = file_id+" "+sender_info, true;
			}
		}
		return response = "internal error: sender not work, timed out after 5 seconds", false;
	}
	bool TrySendFile( const string &session_token, const string &file_id, const string &file_content, string &response )
	{
		{
			lock_guard<mutex>guard( mutex_global );
			string username;
			if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
			const auto it = file_ids.find( file_id );
			if( it == file_ids.end( ) )return response = "invalid file_id", false;
			if( (int) file_content.size( ) != it->second )return response = "expect file_size to be " + to_string( it->second ) + ", received " + to_string( file_content.size( ) ) + " bytes.", false;
			assert( file_contents.find( file_id ) == file_contents.end( ) );
			/////////////////// cerr<<"8. sender puts the file on server with a valid file_id"<<endl;
			file_contents[ file_id ] = file_content;
		}

		////////////////////////////////// 9. sender waits for the file to be admitted. HOW: watch file_id to be removed
		for( const auto start_clock = steady_clock::now( ); TimeBetween( start_clock ) < 1000 * 5;) // wait for 5 seconds
		{
			usleep( 1 );
			lock_guard<mutex>guard( mutex_global );
			//////////////////////////////// 12. sender knows the file has been received, quit with peace
			if( file_ids.find( file_id ) == file_ids.end( ) )return response = "", true;
		}
		{
			lock_guard<mutex>guard( mutex_global );
			file_ids.erase( file_id );
			file_contents.erase( file_id );
			return response = "receiver did not want to take the file, your file_id has been invalidated, please restart the entire transmission again.", false;
		}
	}
	bool TryReceiveFile( const string &session_token, const string &file_id, string &response )
	{
		{
			lock_guard<mutex>guard( mutex_global );
			string username;
			if( !TryGetUsername( session_token, username ) )return response = "invalid session_token", false;
			const auto it = file_ids.find( file_id );
			if( it == file_ids.end( ) )return response = "invalid file_id", false;
		}
		string file_content;
		//////////////////////////////// 10. receiver waits for the file associated with its file_id to be put up. HOW: watch file_contents to appear 
		for( const auto start_clock = steady_clock::now( ); TimeBetween( start_clock ) < 1000 * 5;) // wait for 5 seconds
		{
			usleep( 1 );
			lock_guard<mutex>guard( mutex_global );
			const auto it = file_contents.find( file_id );
			if( it != file_contents.end( ) )
			{
				//////////////////////////// 11. receiver erase file_id to notify sender that it has found the file
				file_ids.erase( file_id );
				file_content = it->second;
				file_contents.erase( it );
				goto index_got_file;// release the lock for sender first, so did not return immediately
			}
		}
		return response = "sender did not put up the file, timed out after 5 seconds", false;
	index_got_file:;
		//////////////////////////// 13. receiver is enjoying the file, transmission completed
		return response = file_content, true;
	}
}
bool ProcessMessage( const string &msg, string &response )
{
	const vector<string>&args = split( msg, ' ' );
	const string &title = args[ 0 ];
	// “signup <username> <password>”
	if( title == "signup" )
	{
		if( args.size( ) != 3 )return response = "signup <username> <password> expect 3 params, got " + to_string( args.size( ) ), false;
		return DataBase::TrySignup( args[ 1 ], args[ 2 ], response );
	}
	else
	// “login <username> <password>”
	if( title == "login" )
	{
		if( args.size( ) != 3 )return response = "login <username> <password> expect 3 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryLogin( args[ 1 ], args[ 2 ], response );
	}
	else
	// “online_users <session_token>”
	if( title == "online_users" )
	{
		if( args.size( ) != 2 )return response = "online_users <session_token> expect 2 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryGetOnlineUsers( args[ 1 ], response );
	}
	else
	// “friends <session_token>”
	if( title == "friends" )
	{
		if( args.size( ) != 2 )return response = "friends <session_token> expect 2 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryGetFriends( args[ 1 ], response );
	}
	else
	// “add_friend <session_token> <friend username>”
	if( title == "add_friend" )
	{
		if( args.size( ) != 3 )return response = "add_friend <session_token> <friend username> expect 3 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryAddFriend( args[ 1 ], args[ 2 ], response );
	}
	else
	// “remove_friend <session_token> <friend username>”
	if( title == "remove_friend" )
	{
		if( args.size( ) != 3 )return response = "remove_friend <session_token> <friend username> expect 3 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryRemoveFriend( args[ 1 ], args[ 2 ], response );
	}
	else
	// “get_message <session_token> <message_id>”
	if( title == "get_message" )
	{
		if( args.size( ) != 3 )return response = "get_message <session_token> <message_id> expect 3 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryGetMessage( args[ 1 ], args[ 2 ], response );
	}
	else
	// “send_message <session_token> <partner username> <message>”
	if( title == "send_message" )
	{
		if( args.size( ) != 4 )return response = "send_message <session_token> <partner username> <message> expect 4 params, got " + to_string( args.size( ) ), false;
		return DataBase::TrySendMessage( args[ 1 ], args[ 2 ], args[ 3 ], response );
	}
	else
	// “last_message <session_token> <partner username>”
	if( title == "last_message" )
	{
		if( args.size( ) != 3 )return response = "last_message <session_token> <partner username> expect 3 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryGetLastMessage( args[ 1 ], args[ 2 ], response );
	}
	else
	// “next_messages <session_token> <message_id> <desired number of messages to get>”
	if( title == "next_messages" )
	{
		if( args.size( ) != 4 )return response = "next_messages <session_token> <message_id> <desired number of messages to get> expect 4 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryGetNextMessages( args[ 1 ], args[ 2 ], args[ 3 ], response );
	}
	else
	// “prev_messages <session_token> <message_id> <desired number of messages to get>”
	if( title == "prev_messages" )
	{
		if( args.size( ) != 4 )return response = "prev_messages <session_token> <message_id> <desired number of messages to get> expect 4 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryGetPreviousMessages( args[ 1 ], args[ 2 ], args[ 3 ], response );
	}
	else
	// “file_request <session_id> <receiver username> <file_size> <file_name>”
	if( title == "file_request" )
	{
		if( args.size( ) < 5 )return response = "file_request <session_id> <receiver username> <file_size> <file_name> expect 5 params, got " + to_string( args.size( ) ), false;
		string file_name = args[ 4 ];
		for( int i = 5; i < (int) args.size( ); i++ )file_name += " " + args[ i ];
		return DataBase::TryFileRequest( args[ 1 ], args[ 2 ], args[ 3 ],file_name, response );
	}
	else
	// “check_file_request <session_id>”
	if( title == "check_file_request" )
	{
		if( args.size( ) != 2 )return response = "check_file_request <session_id> expect 2 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryCheckFileRequest( args[ 1 ], response );
	}
	else
	// “send_file <session_id> <file_id> <entire file content>”
	if( title == "send_file" )
	{
		if( args.size( ) < 4 )return response = "send_file <session_id> <file_id> <entire file content> expect 4 params, got " + to_string( args.size( ) ), false;
		string file_content = args[ 3 ];
		for( int i = 4; i < (int) args.size( ); i++ )file_content += " " + args[ i ];
		return DataBase::TrySendFile( args[ 1 ], args[ 2 ], file_content, response );
	}
	else
	// “receive_file <session_id> <file_id>”
	if( title == "receive_file" )
	{
		if( args.size( ) != 3 )return response = "receive_file <session_id> <file_id> expect 3 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryReceiveFile( args[ 1 ], args[ 2 ], response );
	}
	else
	// "online_check <session token> <username>"
	if( title == "online_check" )
	{
		if( args.size( ) != 3 )return response = "online_check <session token> <username> expect 3 params, got " + to_string( args.size( ) ), false;
		return DataBase::TryOnlineCheck( args[ 1 ], args[ 2 ], response );
	}
	else
	{
		return response = "unrecognized command: " + title, false;
	}
}
vector<int>SelectRead( const map<int, string>&fds )
{
	vector<int>s;
	for( const auto fd : fds )s.push_back( fd.first );
	return select_read( s );
}
int main( int argc, char *argv[ ] )
{
	if( argc != 2 )
	{
		cerr << "argc = " << argc << endl;
		cerr << "Usage: ./server listen_port" << endl;
		exit( 0 );
	}
	const int listen_port = stoi( argv[ 1 ] );
	int server_fd;
	if( !init_server( listen_port, 10, server_fd ) )
	{
		cerr << "Error: init_server()" << endl;
		exit( 0 );
	}
	cerr << "Server fd = " << server_fd << ". Listening..." << endl;
	map<int, string>client_fds;
	mutex mutex_client_fds;
	while( true )
	{
		usleep( 1 );
		//cerr<<"loop";
		{
			int client_fd;
			uint32_t client_ip;
			int client_port;
			while( accept( server_fd, client_fd, client_ip, client_port ) )
			{
				//clog<<"accept new client: fd = "<<client_fd<<endl;
				ostringstream oss;
				oss << ip_to_string( client_ip ) << ':' << client_port;
				oss.flush( );
				//cerr<<"accept client: "<<s<<'('<<ip_to_string(client_ip)<<endl;
				lock_guard<mutex>guard( mutex_client_fds );
				client_fds[ client_fd ] = oss.str( );
			}
		}
		//cerr<<" ac"<<endl;
		const auto &readable_client_fds = [ & ] ( ) {lock_guard<mutex>guard( mutex_client_fds ); return SelectRead( client_fds ); }( );
		for( const int fd : readable_client_fds )
		{
			string msg;
			bool unexpected_error;
			if( receive_string( fd, msg, unexpected_error ) )
			{
				{
					lock_guard<mutex>guard( mutex_client_fds );
#ifdef DEBUG
					cout << "recv from " << client_fds[ fd ] << " : \"" << msg << "\"" << endl;
#else
					cout << "recv from " << client_fds[ fd ] << endl;
#endif			
				}
				// call thread constructor to execute in background
				// call thread.detach() to allow safe destruction while still executing in background
				thread( [ &mutex_client_fds, &client_fds ] ( const int _fd, const string _msg )
				{
					string response = "";
					string to_send = ProcessMessage( _msg, response ) ? "AC" : "WA";
					if( response != "" )to_send += " " + response;
					bool err;
					if( !send_string( _fd, to_send, err ) )cerr << "send error" << endl;
#ifdef DEBUG
					cout << "send back \"" << to_send << "\"" << endl;
#endif
					close( _fd );
					{
						lock_guard<mutex>lock( mutex_client_fds );
						client_fds.erase( _fd );
					}
				}, fd, msg ).detach( );
			}
		}
	}
	return 0;
}
