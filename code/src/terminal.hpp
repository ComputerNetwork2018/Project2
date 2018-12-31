#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

using namespace std;

#define FORMAT_COUNT 16

#define ESC string( "\033[" )

class Position
{
	private:
		int __row;
		int __col;

	public:
		Position( int row = 1, int col = 1 );

		int Row( void ) const;
		bool Row( int row );

		int Col( void ) const;
		bool Col( int col );

		string String( void ) const;
};

enum Colors{ COLOR_BLACK = 0, COLOR_RED = 1, COLOR_GREEN = 2, COLOR_YELLOW = 3, COLOR_BLUE = 4, COLOR_MAGENTA = 5, COLOR_CYAN = 6, COLOR_WHITE = 7, COLOR_RANDOM = 8 };
enum Formats{ FORMAT_NONE = 0x0, FORMAT_BOLD = 0x1, FORMAT_UNDERLINE = 0x10, FORMAT_FLASH = 0x20, FORMAT_HIGHLIGHT = 0x80, FORMAT_INVISIBLE = 0x100 };

class Format
{
	private:
		uint32_t __formats;
		Colors __foreground;
		Colors __background;
		static const uint32_t Mask[ FORMAT_COUNT ];

	public:
		Format( uint32_t formats = FORMAT_NONE, Colors foreground = COLOR_WHITE, Colors background = COLOR_BLACK );

		uint32_t Formats( void ) const;
		bool Formats( uint32_t formats );

		Colors Foreground( void ) const;
		bool Foreground( Colors color );

		Colors Background( void ) const;
		bool Background( Colors color );

		string String( void ) const;
};


class Terminal
{
	private:
		class Position __position;
		class Format __format;
		stringstream __output;
		
	public:
		Terminal( void );

		class Position& Position( void );
		const class Position& Position( void ) const;
		bool Position( const class Position &position );

		class Format& Format( void );
		const class Format& Format( void ) const;
		bool Format( const class Format &format );

		friend ostream& operator<< ( ostream &output, Terminal &terminal );
	
		template <typename T>
		Terminal& operator<< ( const T &output_thing )
		{
			__output << output_thing;
			return *this;
		}
};
#endif
