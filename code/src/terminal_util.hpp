#ifndef TERMINAL_UTIL_HPP
#define TERMINAL_UTIL_HPP

#include "terminal.hpp"

using namespace std;

class Terminal_Util : public Terminal
{
	public:
		void Clear( );
		void Fill( const class Position &from, const class Position &to, const class Format &format, const char character = '@' );
		void MsgPos( const string &msg, const class Position &pos, const class Format &format );
};

#endif
