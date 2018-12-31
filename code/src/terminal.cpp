#ifdef MAKE
#include "terminal.hpp"
#else
#include "../include/terminal.hpp"
#endif

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

extern "C"
{
#include <sys/ioctl.h>
#include <unistd.h>
}

using namespace std;

Position::Position( int row, int col )
{
	Row( row );
	Col( col );
}

int Position::Row( void ) const
{
	return __row;
}

bool Position::Row( int row )
{
	if( row <= 0 )
	{
		__row = 1;
		return false;
	}
	
	winsize terminal_size;
	ioctl( STDOUT_FILENO, TIOCGWINSZ, &terminal_size );
	if( row > terminal_size.ws_row )
	{
		__row = terminal_size.ws_row;
		return false;
	}

	__row = row;
	return true;
}

int Position::Col( void ) const
{
	return __col;
}

bool Position::Col( int col )
{
	if( col <= 0 )
	{
		__col = 1;
		return false;
	}

	winsize terminal_size;
	ioctl( STDOUT_FILENO, TIOCGWINSZ, &terminal_size );
	if( col > terminal_size.ws_col )
	{
		__col = terminal_size.ws_col;
		return false;
	}

	__col = col;
	return true;
}

string Position::String( void ) const
{
	return ( "\033[" + to_string( __row ) + ';' + to_string( __col ) + 'H' );
}





Format::Format( uint32_t formats, Colors foreground, Colors background ): __formats( formats ), __foreground( foreground ), __background( background )
{
	//	empty
}

uint32_t Format::Formats( void ) const
{
	return __formats;
}

bool Format::Formats( uint32_t formats )
{
	__formats = formats;
	return true;
}

Colors Format::Foreground( void ) const
{
	return __foreground;
}

bool Format::Foreground( Colors foreground )
{
	if( foreground == COLOR_RANDOM )
	{
		__foreground = ( Colors )( rand( ) % 8 );
	}
	else
	{
		__foreground = foreground;
	}

	return true;
}

Colors Format::Background( void ) const
{
	return __background;
}

bool Format::Background( Colors background )
{
	if( background == COLOR_RANDOM )
	{
		__background = ( Colors )( rand( ) % 8 );
	}
	else
	{
		__background = background;
	}
	return true;
}

string Format::String( void ) const
{
	string format_string = ESC;
	bool format_set = false;

	for( int i = 0; i < FORMAT_COUNT; ++i )
	{
		if( __formats & Mask[ i ] )
		{
			format_string += to_string( i ) + ';';
			format_set = true;
		}
	}
	if( not format_set )
	{
		format_string += "0;";
	}

	format_string += '4' + to_string( __background ) + ';';
	format_string += '3' + to_string( __foreground ) + 'm';
	return format_string;
}

const uint32_t Format::Mask[ 16 ] = { 0x0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000 };

Terminal::Terminal( void ) : __position( ), __format( )
{
	//	empty
}

class Position& Terminal::Position( void )
{
	return __position;
}

const class Position& Terminal::Position( void ) const
{
	return __position;
}

bool Terminal::Position( const class Position &position )
{
	__position = position;

	__output << __position.String( );

	return true;
}

class Format& Terminal::Format( void )
{
	return __format;
}

const class Format& Terminal::Format( void ) const
{
	return __format;
}

bool Terminal::Format( const class Format &format )
{
	__format = format;

	__output << __format.String( );

	return true;
}

ostream& operator<< ( ostream &output, Terminal &terminal )
{
	output << terminal.__output.rdbuf( ) << flush;
	terminal.__output = stringstream( );

	return output;
}
