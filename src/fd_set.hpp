#ifndef FD_SET_HPP
#define FD_SET_HPP

#include <vector>

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

using namespace std;

namespace TCP_Ping
{

	enum FdSetType{ READ = 0x01, WRITE = 0x02, EXCEPTION = 0x04,
					  READ_WRITE = 0x03, READ_EXCP = 0x05, WRITE_EXCP = 0x06,
					  ALL = 0x07 };

	class Fd_Set
	{
		private:
			int max_fd_;

			fd_set *read_set_;
			fd_set *working_read_set_;

			fd_set *write_set_;
			fd_set *working_write_set_;
			
			fd_set *exception_set_;
			fd_set *working_exception_set_;

			timeval *timeout_;

		public:
			Fd_Set( const vector< int > &read_list = vector< int >( 0 ), const vector< int > &write_list = vector< int >( 0 ), const vector< int > &exception_set = vector< int >( 0 ) );
			Fd_Set( const timeval &timeout, const vector< int > &read_list = vector< int >( 0 ), const vector< int > &write_list = vector< int >( 0 ), const vector< int > &exception_set = vector< int >( 0 ) );
			Fd_Set( const pair< int, int > &timeout, const vector< int > &read_list = vector< int >( 0 ), const vector< int > &write_list = vector< int >( 0 ), const vector< int > &exception_set = vector< int >( 0 ) );

			~Fd_Set( void );

			void Clear( const FdSetType type = ALL );
			void Set_Zero( const FdSetType type = ALL );

			void Erase( const int erase_fd, const FdSetType type = READ );
			void Erase( const vector< int > &erase_list, const FdSetType type = READ );

			void Set( const int set_fd, const FdSetType type = READ );
			void Set( const vector< int > &set_list, const FdSetType type = READ );

			bool Is_Set( const int check_fd, const FdSetType type = READ ) const;
			vector< bool > Is_Set( const vector< int > &check_list, const FdSetType type = READ ) const;

			void Select( void );

			bool Is_Selected( const int check_fd, const FdSetType type = READ ) const;
			vector< bool > Is_Selected( const vector< int > &check_list, const FdSetType type = READ ) const;

			void Timeout( const timeval &timeout, bool force_set = false );
			void Timeout( const pair< int, int > &timeout, bool force_set = false );
			timeval Timeout( void ) const;

			void Blocking( bool blocking );
			bool Blocking( void ) const;

			int Max_Fd( void ) const;
	};

}
#endif
