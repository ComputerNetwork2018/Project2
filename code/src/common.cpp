#include<bits/stdc++.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<netdb.h>
#include<fcntl.h>

using namespace std;
template<class T>inline bool getmax( T&a, const T&b )
{
	return a < b ? ( a = b, true ) : false;
}

template<class T>inline bool getmin( T&a, const T&b )
{
	return b < a ? ( a = b, true ) : false;
}

/*bool set_nonblock(const int fd)
{
	int flags=fcntl(fd,F_GETFL);
	if(flags==-1)return false;
	return fcntl(fd,F_SETFL,flags|O_NONBLOCK)!=-1;
}*/

vector<int> select_write( const vector<int> &fds )
{
	fd_set wfds;
	FD_ZERO( &wfds );
	int max_fd = 0;
	for( const int fd : fds )
	{
		FD_SET( fd, &wfds );
		getmax( max_fd, fd + 1 );
	}
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	const int retval = select( max_fd, NULL, &wfds, NULL, &tv );
	if( retval <= 0 )return vector<int>( );
	//cerr<<"SelectRead "<<retval<<" fds."<<endl;
	vector<int>ans;
	for( const int fd : fds )
	{
		if( FD_ISSET( fd, &wfds ) )ans.push_back( fd );
	}
	assert( (int) ans.size( ) == retval );
	return ans;
}

vector<int> select_read( const vector<int> &fds )
{
	fd_set rfds;
	FD_ZERO( &rfds );
	int max_fd = 0;
	for( const int fd : fds )
	{
		FD_SET( fd, &rfds );
		getmax( max_fd, fd + 1 );
	}
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	const int retval = select( max_fd, &rfds, NULL, NULL, &tv );
	if( retval <= 0 )return vector<int>( );
	//cerr<<"SelectRead "<<retval<<" fds."<<endl;
	vector<int>ans;
	for( const int fd : fds )
	{
		if( FD_ISSET( fd, &rfds ) )ans.push_back( fd );
	}
	assert( (int) ans.size( ) == retval );
	return ans;
}

bool init_server( const int port, const int queueLen, int &server_fd )
{
	server_fd = socket( AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0 );
	if( server_fd < 0 )
	{
		fprintf( stderr, "socket open error\n" );
		return false;
	}
	/*if(!set_nonblock(server_fd))
	{
		cerr<<"Error: set_nonblock"<<endl;
		return false;
	}*/
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_ANY );
	addr.sin_port = htons( (uint16_t) port );
	if( bind( server_fd, ( struct sockaddr* )&addr, sizeof( addr ) ) < 0 )
	{
		close( server_fd );
		fprintf( stderr, "port bind error\n" );
		return false;
	}
	if( listen( server_fd, queueLen ) < 0 )
	{
		close( server_fd );
		fprintf( stderr, "port listen error\n" );
		return false;
	}
	return true;
}

bool send( const int fd, const void *buf, size_t len, bool &unexpected_error )
{
	unexpected_error = false;
	const bool ret = ( send( fd, buf, len, 0 ) == (int) len );
	if( !ret )
	{
		if( errno != EAGAIN && errno != EWOULDBLOCK )unexpected_error = true;
	}
	return ret;
}

bool send_int( const int fd, const int v, bool &unexpected_error )
{
	return send( fd, &v, sizeof( int ), unexpected_error );
}

bool send_string( const int fd, const string &s, bool &unexpected_error )
{
	const size_t length = sizeof( int ) + s.size( ) * sizeof( char );
	uint8_t *buf = new uint8_t[ length ];
	const int size = (int) s.size( );
	memcpy( buf, &size, sizeof( int ) );
	memcpy( buf + sizeof( int ), s.c_str( ), s.size( ) * sizeof( char ) );
	const auto ret = send( fd, buf, length, unexpected_error );
	delete[ ]buf;
	return ret;
}

bool receive( const int fd, void *buf, size_t len, bool &unexpected_error )
{
	unexpected_error = false;
	const bool ret = ( recv( fd, buf, len, MSG_DONTWAIT ) == (int) len );
	if( !ret )
	{
		if( errno != EAGAIN && errno != EWOULDBLOCK )unexpected_error = true;
	}
	return ret;
}

bool receive_int( const int fd, int &msg, bool &unexpected_error )
{
	return receive( fd, &msg, sizeof( int ), unexpected_error );
}

bool receive_string( const int fd, string &msg, bool &unexpected_error )
{
	int length;
	if( !receive_int( fd, length, unexpected_error ) )return false;
	char *buf = new char[ length ];
	if( receive( fd, buf, length * sizeof( char ), unexpected_error ) )
	{
		msg = "";
		for( int i = 0; i < length; i++ )msg += buf[ i ];
		delete[ ]buf;
		return true;
	}
	else
	{
		delete[ ]buf;
		return false;
	}
}

bool accept( const int sockfd, int &client_fd, uint32_t &client_ip, int &client_port )
{
	// 呼叫 accept() 會 block 住程式直到有新連線進來，回傳值代表新連線的描述子
	struct sockaddr_in client_addr;
	socklen_t addrlen = sizeof( client_addr );
	client_fd = accept4( sockfd, ( struct sockaddr* ) &client_addr, &addrlen, SOCK_NONBLOCK );
	//    assert(client_fd >= 0);

	/* 注意實作上細節，呼叫 accept() 時 addrlen 必須是 client_addr 結構的長度
	   accept() 回傳之後會將客戶端的 IP 位置填到 client_addr
	   並將新的 client_addr 結構長度填寫到 addrlen                        */
	if( client_fd == -1 )return false;
	client_ip = client_addr.sin_addr.s_addr;
	client_port = (int) client_addr.sin_port;
	return true;
}

bool try_connect( const int server_fd, int &error_number, const struct sockaddr_in &addr )
{
	if( connect( server_fd, ( struct sockaddr* )&addr, sizeof( addr ) ) < 0 )
	{
		//https://blog.csdn.net/nphyez/article/details/10268723
		error_number = errno;
		return false;
	}
	return true;
}

bool connect_to( const string &servername, const int port, int &server_fd, int &error_number, struct sockaddr_in &addr )
{
	struct hostent* server = NULL;
	server = gethostbyname( servername.c_str( ) );
	if( server == NULL )
	{
		fprintf( stderr, "bad server address error\n" );
		return false;
	}
	server_fd = socket( AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0 );
	if( server_fd < 0 )
	{
		fprintf( stderr, "socket open error\n" );
		return false;
	}
	memset( &addr, 0, sizeof( addr ) );
	addr.sin_family = AF_INET;
	addr.sin_port = htons( (uint16_t) port );
	memcpy( &addr.sin_addr.s_addr,
			server->h_addr_list[ 0 ],
			server->h_length );
	if( connect( server_fd, ( struct sockaddr* )&addr, sizeof( addr ) ) < 0 )
	{
		//https://blog.csdn.net/nphyez/article/details/10268723
		error_number = errno;
		if( errno == EINPROGRESS )//three-way-hand-shaking
		{
			return false;
		}
		fprintf( stderr, "connect server error\n" );
		close( server_fd );
		return false;
	}
	return true;
}

/*bool connect_to(const string &servername_port,int &server_fd)
{
	const auto i=servername_port.find(':');
	if(i==string::npos)return false;
	int port=stoi(servername_port.substr(i+1));
	return connect_to(servername_port.substr(0,i),port,server_fd);
}*/

string ip_to_string( const uint32_t ip )
{
	ostringstream oss;
	oss << ( ip & 0xff ) << '.';
	oss << ( ( ip >> 8 ) & 0xff ) << '.';
	oss << ( ( ip >> 16 ) & 0xff ) << '.';
	oss << ( ip >> 24 );
	oss.flush( );
	return oss.str( );
}