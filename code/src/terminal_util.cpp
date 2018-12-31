#include "terminal_util.hpp"

using namespace std;

Terminal_Util::Clear( )
{
	__output << "\e[2J";
}

Terminal_Util::Fill( const Position &from, const Position &to, const char character, const Format &format )
{
	Format( format );

	for( int i = from.Row( ); i <= to.Row( ); ++i )
	{
		Position( class Position( i, from.Col( ) ) );

		for( int j = from.Col( ); j <= to.Col( ); ++j )
		{
			__output << character;
		}
	}
}

Terminal_Util::MsgPos( const string &msg, const Position &pos, const Format &format )
{
	Position( pos );
	Format( format );
	__output << msg;
}