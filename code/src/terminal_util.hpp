#ifndef TERMINAL_UTIL_HPP
#define TERMINAL_UTIL_HPP

#include "terminal.hpp"

using namespace std;

class Terminal_Util : public Terminal
{
	public:
		Clear( );
		Fill( const Position &from, const Position &to, const char character = '@', const Format &format = Format( ) );
		MsgPos( const string &msg, const Position &pos = Position( ), const Foramt &format = Format( ) );
};

#endif