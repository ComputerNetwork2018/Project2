#include <vector>
#include <iostream>

#include <cstring>

extern "C"
{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}

#include "fd_set.hpp"

using namespace std;

namespace TCP_Ping
{

	Fd_Set::Fd_Set( const vector< int > &read_set, const vector< int > &write_set, const vector< int > &exception_set ) : max_fd_( -1 )
	{
		read_set_ = new fd_set;
		working_read_set_ = new fd_set;

		write_set_ = new fd_set;
		working_write_set_ = new fd_set;

		exception_set_ = new fd_set;
		working_exception_set_ = new fd_set;

		timeout_ = new timeval;

		FD_ZERO( read_set_ );
		for( auto &i : read_set )
		{
			FD_SET( i, read_set_ );
			
			if( i >= max_fd_ )
			{
				max_fd_ = i + 1;
			}
		}

		FD_ZERO( write_set_ );
		for( auto &i : write_set )
		{
			FD_SET( i, write_set_ );
			
			if( i >= max_fd_ )
			{
				max_fd_ = i + 1;
			}
		}

		FD_ZERO( exception_set_ );
		for( auto &i : exception_set )
		{
			FD_SET( i, exception_set_ );
			
			if( i >= max_fd_ )
			{
				max_fd_ = i + 1;
			}
		}

		memset( timeout_, 0, sizeof( timeval ) );
	}

	Fd_Set::Fd_Set( const timeval &timeout, const vector< int > &read_set, const vector< int > &write_set, const vector< int > &exception_set ) : max_fd_( -1 )
	{
		read_set_ = new fd_set;
		working_read_set_ = new fd_set;

		write_set_ = new fd_set;
		working_write_set_ = new fd_set;

		exception_set_ = new fd_set;
		working_exception_set_ = new fd_set;

		timeout_ = new timeval;

		FD_ZERO( read_set_ );
		for( auto &i : read_set )
		{
			FD_SET( i, read_set_ );
			
			if( i >= max_fd_ )
			{
				max_fd_ = i + 1;
			}
		}

		FD_ZERO( write_set_ );
		for( auto &i : write_set )
		{
			FD_SET( i, write_set_ );
			
			if( i >= max_fd_ )
			{
				max_fd_ = i + 1;
			}
		}

		FD_ZERO( exception_set_ );
		for( auto &i : exception_set )
		{
			FD_SET( i, exception_set_ );
			
			if( i >= max_fd_ )
			{
				max_fd_ = i + 1;
			}
		}

		*timeout_ = timeout;
	}

	Fd_Set::Fd_Set( const pair< int, int > &timeout, const vector< int > &read_set, const vector< int > &write_set, const vector< int > &exception_set ) : max_fd_( -1 )
	{
		read_set_ = new fd_set;
		working_read_set_ = new fd_set;

		write_set_ = new fd_set;
		working_write_set_ = new fd_set;

		exception_set_ = new fd_set;
		working_exception_set_ = new fd_set;

		timeout_ = new timeval;

		FD_ZERO( read_set_ );
		for( auto &i : read_set )
		{
			FD_SET( i, read_set_ );
			
			if( i >= max_fd_ )
			{
				max_fd_ = i + 1;
			}
		}

		FD_ZERO( write_set_ );
		for( auto &i : write_set )
		{
			FD_SET( i, write_set_ );
			
			if( i >= max_fd_ )
			{
				max_fd_ = i + 1;
			}
		}

		FD_ZERO( exception_set_ );
		for( auto &i : exception_set )
		{
			FD_SET( i, exception_set_ );
			
			if( i >= max_fd_ )
			{
				max_fd_ = i + 1;
			}
		}

		memset( timeout_, 0, sizeof( timeval ) );
		timeout_ -> tv_sec = timeout.first;
		timeout_ -> tv_usec = timeout.second;
	}

	Fd_Set::~Fd_Set( void )
	{
		delete read_set_;
		delete working_read_set_;

		delete write_set_;
		delete working_write_set_;

		delete exception_set_;
		delete working_exception_set_;

		delete timeout_;
	}

	void Fd_Set::Clear( const FdSetType type )
	{
		if( ( type & READ ) != 0 )
		{
			FD_ZERO( read_set_ );
		}

		if( ( type & WRITE ) != 0 )
		{
			FD_ZERO( write_set_ );
		}

		if( ( type & EXCEPTION ) != 0 )
		{
			FD_ZERO( exception_set_ );
		}

		max_fd_ = -1;
	}

	void Fd_Set::Set_Zero( const FdSetType type )
	{
		Clear( type );
	}

	void Fd_Set::Erase( const int erase_fd, const FdSetType type )
	{
		if( ( type & READ ) != 0 )
		{
			FD_CLR( erase_fd, read_set_ );
		}
		
		if( ( type & WRITE ) != 0 )
		{
			FD_CLR( erase_fd, write_set_ );
		}

		if( ( type & EXCEPTION ) != 0 )
		{
			FD_CLR( erase_fd, exception_set_ );
		}
	}

	void Fd_Set::Erase( const vector< int > &erase_list, const FdSetType type )
	{
		if( ( type & READ ) != 0 )
		{
			for( auto &i : erase_list )
			{
				FD_CLR( i, read_set_ );
			}
		}

		if( ( type & WRITE ) != 0 )
		{
			for( auto &i : erase_list )
			{
				FD_CLR( erase_list[ i ], write_set_ );
			}
		}

		if( ( type & EXCEPTION ) != 0 )
		{
			for( auto &i : erase_list )
			{
				FD_CLR( erase_list[ i ], exception_set_ );
			}
		}

	}

	void Fd_Set::Set( const int set_fd, const FdSetType type )
	{
		if( ( type & READ ) != 0 )
		{
			FD_SET( set_fd, read_set_ );
			if( set_fd >= max_fd_ )
			{
				max_fd_ = set_fd + 1;
			}
		}

		if( ( type & WRITE ) != 0 )
		{
			FD_SET( set_fd, write_set_ );
			
			if( set_fd >= max_fd_ )
			{
				max_fd_ = set_fd + 1;
			}
		}

		if( ( type & EXCEPTION ) != 0 )
		{
			FD_SET( set_fd, exception_set_ );
			
			if( set_fd >= max_fd_ )
			{
				max_fd_ = set_fd + 1;
			}
		}
	}

	void Fd_Set::Set( const vector< int > &set_list, const FdSetType type )
	{
		if( ( type & READ ) != 0 )
		{
			for( auto &i : set_list )
			{
				FD_SET( i, read_set_ );

				if( i >= max_fd_ )
				{
					max_fd_ = i + 1;
				}
			}
		}

		if( ( type & WRITE ) != 0 )
		{
			for( auto &i : set_list )
			{
				FD_SET( i, write_set_ );

				if( i >= max_fd_ )
				{
					max_fd_ = i + 1;
				}
			}
		}

		if( ( type & EXCEPTION ) != 0 )
		{
			for( auto &i : set_list )
			{
				FD_SET( i, exception_set_ );

				if( i >= max_fd_ )
				{
					max_fd_ = i + 1;
				}
			}
		}
	}

	bool Fd_Set::Is_Set( const int check_fd, const FdSetType type ) const
	{
		if( ( type & READ ) != 0 )
		{
			if( not FD_ISSET( check_fd, read_set_ ) )
			{
				return false;
			}
		}

		if( ( type & WRITE ) != 0 )
		{
			if( not FD_ISSET( check_fd, write_set_ ) )
			{
				return false;
			}
		}

		if( ( type & EXCEPTION ) != 0 )
		{
			if( not FD_ISSET( check_fd, exception_set_ ) )
			{
				return false;
			}
		}

		return true;
	}

	vector< bool > Fd_Set::Is_Set( const vector< int > &check_list, const FdSetType type ) const
	{
		vector< bool > result;
		result.reserve( check_list.size( ) );

		for( auto &i : check_list )
		{
			result.push_back( Is_Set( i, type ) );
		}

		return result;
	}

	void Fd_Set::Select( void )
	{
		if( max_fd_ < 0 )
		{
			clog << "Unable to select, not any fd is set." << endl;
			return;
		}

		*working_read_set_ = *read_set_;
		*working_write_set_ = *write_set_;
		*working_exception_set_ = *exception_set_;

		select( max_fd_, working_read_set_, working_write_set_, working_exception_set_, timeout_ );
	}

	bool Fd_Set::Is_Selected( const int check_fd, const FdSetType type ) const
	{
		if( ( type & READ ) != 0 )
		{
			if( not FD_ISSET( check_fd, working_read_set_ ) )
			{
				return false;
			}
		}

		if( ( type & WRITE ) != 0 )
		{
			if( not FD_ISSET( check_fd, working_write_set_ ) )
			{
				return false;
			}
		}

		if( ( type & EXCEPTION ) != 0 )
		{
			if( not FD_ISSET( check_fd, working_exception_set_ ) )
			{
				return false;
			}
		}

		return true;
	}

	vector< bool > Fd_Set::Is_Selected( const vector< int > &check_list, const FdSetType type ) const
	{
		vector< bool > result;
		result.reserve( check_list.size( ) );

		for( auto &i : check_list )
		{
			result.push_back( Is_Set( i, type ) );
		}

		return result;
	}

	void Fd_Set::Timeout( const timeval &timeout, bool force_set )
	{
		if( force_set )
		{
			*timeout_ = timeout;
		}
		else if( timeout_ -> tv_sec >= 0 and timeout_ -> tv_usec >= 0 and timeout.tv_sec >= 0 and timeout.tv_usec >= 0 )
		{
			*timeout_ = timeout;
		}
		else
		{
			clog << "Trying to change blocking attribite, or is originally non-blocking." << endl;
		}
	}

	void Fd_Set::Timeout( const pair< int, int > &timeout, bool force_set )
	{
		if( force_set )
		{
			timeout_ -> tv_sec = timeout.first;
			timeout_ -> tv_usec = timeout.second;
		}
		else if( timeout_ -> tv_sec >= 0 and timeout_ -> tv_usec >= 0 and timeout.first >= 0 and timeout.second >= 0 )
		{
			timeout_ -> tv_sec = timeout.first;
			timeout_ -> tv_usec = timeout.second;
		}
		else
		{
			clog << "Trying to change blocking attribite, or is originally non-blocking." << endl;
		}
	}

	timeval Fd_Set::Timeout( void ) const
	{
		return *timeout_;
	}

	void Fd_Set::Blocking( bool blocking )
	{
		if( ( timeout_ -> tv_sec < 0 or timeout_ -> tv_usec < 0 ) and ( not blocking ) )
		{
			timeout_ -> tv_sec = 0;
			timeout_ -> tv_usec = 0;
		}
		else if( timeout_ -> tv_sec >= 0 and timeout_ -> tv_usec >= 0 and blocking )
		{
			timeout_ -> tv_sec = -1;
			timeout_ -> tv_usec = -1;
		}
	}

	bool Fd_Set::Blocking( void ) const
	{
		return ( timeout_ -> tv_sec < 0 or timeout_ -> tv_usec < 0 );
	}

	int Fd_Set::Max_Fd( void ) const
	{
		return max_fd_;
	}

}
