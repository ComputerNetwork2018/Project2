#ifndef MSG_HPP
#define MSG_HPP

#include <string>

#include <cstdint>

#define Id uint64_t

using namespace std;

namespace Client
{
	class LocalMsg : public string
	{
	public:
		bool TrySend( ) const;
	};

	class RemoteMsg : public string
	{
	private:
		class Id id;
	
	public:
		RemoteMsg( const string &msg, const Id &id );

		class Id Id( ) const;
		bool tryQueryOlder( const int count, vector< RemoteMsg > &result ) const;
		bool tryQueryNewer( const int count, vector< RemoteMsg > &result ) const;
	};
}

#endif