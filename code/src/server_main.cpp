#include <iostream>
#include <string>

#include "server.hpp"

using namespace std;
using namespace TCP_Ping;

int main( int argc, char **argv )
{
	vector< string > args( 0 );
	for( int i = 0; i < argc; ++i )
	{
		args.push_back( argv[ i ] );
	}

	Server server;
	return server.Main( args );
}
