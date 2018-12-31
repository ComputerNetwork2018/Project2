#include "terminal_util.hpp"

using namespace std;

void Terminal_Util::Clear( )
{
	__output << "\e[2J";
}

void Terminal_Util::Fill( const class Position &from, const class Position &to, const class Format &format, const char character )
{
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
	Position( pos );
	Format( format );
	__output << msg;
}
