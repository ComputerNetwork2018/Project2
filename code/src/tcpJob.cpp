#include <cassert>

#include "tcpJob.hpp"

using namespace std;
using namespace chrono;

namespace Client
{
	void TCPJob::_TryTCP( )
	{
		if( not isHandshaking )
		{
			int errorNumber;

			if( not connect_to( host, port, serverFd, errorNumber, socketInfo ) )
			{
				if( serverFd == -1 )
				{
					return;
				}

				if( errorNumber != EINPROGRESS )
				{
					close( serverFd );
					nothingToDo = true;
					return;
				}

				isHandshaking = true;
				return;
			}

			assert( serverFd != -1 );
			isHandshaking = isConnected = true;
			return;
		}

		if( not isConnected )
		{
		tcpJob__TryTCP_failed:;
			const auto &readFdSet = select_read( { serverFd } );
			const auto &writeFdSet = select_write( { serverFd } );

			if( ( not readFdSet.empty( ) ) or ( not writeFdSet.empty( ) ) )
			{
				int errorNumber;

				if( try_connect( serverFd, errorNumber, socketInfo ) )
				{
					goto tcpJob__TryTCP_failed;
				}

				if( errorNumber != EISCONN )
				{
					close( serverFd );
					nothingToDo = true;
					return;
				}

				isConnected = true;
			}

			return;
		}

		if( not isSent )
		{
			bool error;
			
			if( send_string( serverFd, command, error ) )
			{
				isSent = true;
			}

			if( error )
			{
				close( serverFd );
				nothingToDo = true;
				return;
			}

			return;
		}

		{
			bool error;
			string msg;

			if( receive_string( serverFd, msg, error ) )
			{
				close( serverFd );
				nothingToDo = true;
				return;
			}

			if( error )
			{
				close( serverFd );
				nothingToDo = true;
				return;
			}

			return;
		}
	}

	TCPJob::TCPJob( const string &_command, const string &_host, const int _port, const int _timeout, const int _id )
		:command( _command ), host( _host ), port( _port ), timeout( _timeout ), id( _id ), startClock( steady_clock::now( ) ), result( "timeout when connect to " + host )
	{
	}

	void TCPJob::TryTCP( string &info, bool &isTimeout )
	{
		if( not nothingToDo )
		{
			_TryTCP( );
		}

		isTimeout = ( timeout != 0 and ( currentDelay( ) > timeout ) );

		if( not infoGiven and ( nothingToDo or isTimeout ) )
		{
			info = result;
			infoGiven = true;
		}

		if( not nothingToDo and isTimeout and serverFd != -1 )
		{
			close( serverFd );
		}
	}

	int TCPJob::currentDelay( )
	{
		return static_cast<int>( duration_cast<microseconds>( steady_clock::now( ) - startClock ).count( ) / 1000 );
	}
}
