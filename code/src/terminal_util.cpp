#include "terminal_util.hpp"

using namespace std;

Terminal_Util::Terminal_Util( ) : outputMutex( )
{
	
}

void Terminal_Util::Clear( )
{
	lock_guard<mutex> outputLock( outputMutex );

	__output << "\e[2J";
}

void Terminal_Util::Fill( const class Position &from, const class Position &to, const class Format &format, const char character )
{
	lock_guard<mutex> outputLock( outputMutex );

	Format( format );

	for( int i = from.Row( ); i <= to.Row( ); ++i )
	{
		Position( ::Position( i, from.Col( ) ) );

		for( int j = from.Col( ); j <= to.Col( ); ++j )
		{
			__output << character;
		}
	}
}

void Terminal_Util::MsgPos( const string &msg, const class Position &pos, const class Format &format )
{
	lock_guard<mutex> outputLock( outputMutex );

	//Position( pos );
	Format( format );
	__output << msg;
}

stringstream& Terminal_Util::OSStream( )
{
	return __output;
}

ostream& operator<< ( ostream &output, Terminal_Util &terminalUtil )
{
	output << terminalUtil.OSStream( ).rdbuf( ) << flush;
	terminalUtil.OSStream( ) = stringstream( );

	return output;
}
