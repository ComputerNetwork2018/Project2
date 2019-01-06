#ifndef TERMINAL_UTIL_HPP
#define TERMINAL_UTIL_HPP

#include <mutex>

#include "terminal.hpp"

#define CSI_SCP (ESC+"s")
#define CSI_RCP (ESC+"u")

using namespace std;

class Terminal_Util : public Terminal
{
private:
	mutex outputMutex;
	stringstream& OSStream( );

public:
	Terminal_Util( );

	void Clear( );
	void Fill( const class Position &from, const class Position &to, const class Format &format, const char character = '@' );
	void MsgPos( const string &msg, const class Position &pos, const class Format &format = ::Format( ) );
	template<typename T> void MsgPos( const T &msg, const class Position &pos, const class Format &format = ::Format( ) );

	friend ostream& operator<< ( ostream &output, Terminal_Util &terminalUtil );
};

#endif
