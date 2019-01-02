#include<bits/stdc++.h>

using namespace std;

string Decode( const string &s )
{
	assert( s.size( ) % 2 == 0 );
	vector<uint8_t>t;
	for( const char c : s )t.push_back( '0' <= c && c <= '9' ? (uint8_t) ( c - '0' ) : (uint8_t) ( c - 'a' + 10 ) );
	string ans = "";
	for( int i = 0; i < (int) t.size( ); i += 2 )
	{
		ans += (char) ( t[ i ] * 16 + t[ i + 1 ] );
	}
	return ans;
}

string Encode( const char c )
{
	uint32_t v = (uint32_t) c;
	string ans = "";
	for( int i = 0; i < 2; i++, v >>= 4 )
	{
		const auto r = v & 0xf;
		if( r <= 9 )ans += (char) ( '0' + r );
		else ans += (char) ( 'a' + ( r - 10 ) );
	}
	reverse( ans.begin( ), ans.end( ) );
	return ans;
}

string Encode( const string &s )
{
	string ans = "";
	for( const char c : s )ans += Encode( c );
	return ans;
}

int main( )
{
	ios::sync_with_stdio( 0 );
	cin.tie( 0 );
	while( true )
	{
		string s;
		getline( cin, s );
		//cout<<"gets: "<<s<<endl;
		cout << "encode: " << Encode( s ) << endl;
		cout << "decode: " << Decode( Encode( s ) ) << endl;
	}
	return 0;
}
