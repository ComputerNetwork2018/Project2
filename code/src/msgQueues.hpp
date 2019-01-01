#ifndef MSG_SEND_QUEUE_HPP
#define MSG_SEND_QUEUE_HPP

#include <string>
#include <queue>
#include <deque>

#include "msg.hpp"

using namespace std;

namespace Client
{
	class MsgSendQueue : public queue< LocalMsg >;

	class MsgCache : public deque< RemoteMsg >
	{
	public:
		bool TryQueryOlder( const int count, vector< RemoteMsg > &result ) const;
		bool TryQueryNewer( const int count, vector< RemoteMsg > &result ) const;
	};
}

#endif