#ifndef MSG_HPP
#define MSG_HPP

#include <string>
#include <vector>

#include <cstdint>

typedef uint64_t Id;

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
		::Id id;
	
	public:
		RemoteMsg( const string &msg, const ::Id &id );

		::Id Id( ) const;
		bool tryQueryOlder( const int count, vector< RemoteMsg > &result ) const;
		bool tryQueryNewer( const int count, vector< RemoteMsg > &result ) const;
	};
}

#endif
