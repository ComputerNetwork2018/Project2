#include<bits/stdc++.h>
#include"common.h"
using namespace std;
namespace DataBase
{
	class Message
	{
		private:
		string from,to;
		string text;
		string prev="",next="";
		public:
		Message(){}
		Message(const string &_from,const string &_to,const string &_text):from(_from),to(_to),text(_text){}
		string to_json()const{return "{from:"+from+",to:"+to+",text:"+text+"}";}
		bool belong_to(const string &name)const{return name==from||name==to;}
		void SetPrev(const string &message_id){assert(prev=="");prev=message_id;}
		void SetNext(const string &message_id){assert(next=="");next=message_id;}
		string GetPrev()const{return prev;}
		string GetNext()const{return next;}
	};
	mt19937 Rand(time(NULL));
	string generate_token()
	{
		const int len=16;
		string ans="";
		for(int i=0;i<len;i++)
		{
			const unsigned v=(unsigned)Rand()&0xf;
			if(v<=9)ans.push_back((char)('0'+v));
			else ans.push_back((char)('a'+(v-10)));
		}
		cerr<<ans<<endl;
		return ans;
	}
	map<string,string>usernames;// username, password
	map<string,string>session_tokens;// session_token, username
	map<string,set<string>>friends;// username, friends
	map<string,Message>messages;// message_id, message_content
	map<pair<string,string>,string>last_message;// <user1, user2>, message_id
	void ConnectMessage(const string &message_id1,const string &message_id2)
	{
		auto it1=messages.find(message_id1),
			 it2=messages.find(message_id2);
		assert(it1!=messages.end()&&it2!=messages.end());
		it1->second.SetNext(message_id2);
		it2->second.SetPrev(message_id1);
	}
	map<pair<string,string>,string>::iterator GetLastMessage(const string &user1,const string &user2)
	{
		if(user1>user2)return GetLastMessage(user2,user1);
		const auto &key=make_pair(user1,user2);
		return last_message.find(key);
	}
	void UpdateLastMessage(const string &user1,const string &user2,const string &message_id)
	{
		const auto it=GetLastMessage(user1,user2);
		if(it!=last_message.end())
		{
			ConnectMessage(it->second,message_id);
			it->second=message_id;
		}
		else
		{
			const auto &key=make_pair(min(user1,user2),max(user1,user2));
			last_message[key]=message_id;
		}
	}
	string InsertMessage(const string &from,const string &to,const string &text)
	{
		const auto msg=Message(from,to,text);
		const auto message_id=generate_token();
		messages[message_id]=msg;
		UpdateLastMessage(from,to,message_id);
		return message_id;
	}
	bool IsUserExists(const string &username)
	{
		return usernames.find(username)!=usernames.end();
	}
	bool TrySignup(const string &username,const string &password,string &msg)
	{
		if(IsUserExists(username))return msg="user \""+username+"\" is already signed up",false;
		usernames[username]=password;
		return true;
	}
	bool TryLogin(const string &username,const string &password,string &response)
	{
		const auto it=usernames.find(username);
		if(it==usernames.end())return response="user \""+username+"\" not existed",false;
		if(it->second!=password)return response="wrong password",false;
		const string &token=generate_token();
		session_tokens[token]=username;
		return response=token,true;
	}
	bool TryGetUsername(const string &session_token,string &username)
	{
		const auto it=session_tokens.find(session_token);
		if(it==session_tokens.end())return false;
		return username=it->second,true;
	}
	bool TryGetOnlineUsers(const string &session_token,string &response)
	{
		string username;
		if(!TryGetUsername(session_token,username))return response="invalid session_token",false;
		vector<string>ans;
		for(const auto p:session_tokens)if(p.first!=session_token)ans.push_back(p.second);
		response=to_string(ans.size());
		for(const string &name:ans)response+=" "+name;
		return true;
	}
	bool TryGetFriends(const string &session_token,string &response)
	{
		string username;
		if(!TryGetUsername(session_token,username))return response="invalid session_token",false;
		const set<string>&ans=friends[username];
		response=to_string(ans.size());
		for(const string &name:ans)response+=" "+name;
		return true;
	}
	bool TryAddFriend(const string &session_token,const string &friend_name,string &response)
	{
		string username;
		if(!TryGetUsername(session_token,username))return response="invalid session_token",false;
		if(!IsUserExists(friend_name))return response="friend \""+friend_name+"\" not exists",false;
		set<string>&fs=friends[username];
		if(fs.find(friend_name)!=fs.end())return response="you're already friends",false;
		return fs.insert(friend_name),true;
	}
	bool TryRemoveFriend(const string &session_token,const string &friend_name,string &response)
	{
		string username;
		if(!TryGetUsername(session_token,username))return response="invalid session_token",false;
		set<string>&fs=friends[username];
		const auto it=fs.find(friend_name);
		if(it==fs.end())return response="you're not friends",false;
		return fs.erase(it),true;
	}
	bool TryGetMessage(const string &session_token,const string &message_id,string &response)
	{
		string username;
		if(!TryGetUsername(session_token,username))return response="invalid session_token",false;
		const auto &it=messages.find(message_id);
		if(it==messages.end())return response="non-existing message_id",false;
		const Message &msg=it->second;
		if(!msg.belong_to(username))return response="permission denied",false;
		return response=msg.to_json(),true;
	}
	bool TrySendMessage(const string &session_token,const string &partner_name,const string &text,string &response)
	{
		string username;
		if(!TryGetUsername(session_token,username))return response="invalid session_token",false;
		if(!IsUserExists(partner_name))return response="partner \""+partner_name+"\" not existed",false;
		const string &message_id=InsertMessage(username,partner_name,text);
		return response=message_id,true;
	}
	bool TryGetLastMessage(const string &session_token,const string &partner_name,string &response)
	{
		string username;
		if(!TryGetUsername(session_token,username))return response="invalid session_token",false;
		const auto it=GetLastMessage(username,partner_name);
		if(it==last_message.end())return response="",true;
		else return response=it->second,true;
	}
	bool TryGetNextMessages(const string &username,const string &message_id,const int desired_count,string &response)
	{
		const auto it=messages.find(message_id);
		if(it==messages.end())return response="non-existing message_id",false;
		auto msg=it->second;
		if(!msg.belong_to(username))return response="permission denied",false;
		vector<string>ans;
		for(int i=0;i<desired_count;i++)
		{
			assert(msg.belong_to(username));
			const string &next_id=msg.GetNext();
			if(next_id=="")break;
			const auto iu=messages.find(next_id);
			assert(iu!=messages.end());
			msg=iu->second;
		}
		response=to_string(ans.size());
		for(const string &id:ans)response+=" "+id;
		return true;
	}
	bool TryGetNextMessages(const string &session_token,const string &message_id,const string &desired_count,string &response)
	{
		string username;
		if(!TryGetUsername(session_token,username))return response="invalid session_token",false;
		int n;
		try{n=stoi(desired_count);}
		catch(...){return response="desired_count isn't a number",false;}
		if(n<1||n>100)return response="supported range of desired_count is 1~100, got "+to_string(n),false;
		return TryGetNextMessages(username,message_id,n,response);
	}
	bool TryGetPreviousMessages(const string &username,const string &message_id,const int desired_count,string &response)
	{
		const auto it=messages.find(message_id);
		if(it==messages.end())return response="non-existing message_id",false;
		auto msg=it->second;
		if(!msg.belong_to(username))return response="permission denied",false;
		vector<string>ans;
		for(int i=0;i<desired_count;i++)
		{
			assert(msg.belong_to(username));
			const string &prev_id=msg.GetPrev();
			if(prev_id=="")break;
			const auto iu=messages.find(prev_id);
			assert(iu!=messages.end());
			msg=iu->second;
		}
		response=to_string(ans.size());
		for(const string &id:ans)response+=" "+id;
		return true;
	}
	bool TryGetPreviousMessages(const string &session_token,const string &message_id,const string &desired_count,string &response)
	{
		string username;
		if(!TryGetUsername(session_token,username))return response="invalid session_token",false;
		int n;
		try{n=stoi(desired_count);}
		catch(...){return response="desired_count isn't a number",false;}
		if(n<1||n>100)return response="supported range of desired_count is 1~100, got "+to_string(n),false;
		return TryGetPreviousMessages(username,message_id,n,response);
	}
}
bool ProcessMessage(const string &msg,string &response)
{
	const vector<string>&args=split(msg,' ');
	const string &title=args[0];
	// “signup <username> <password>”
	if(title=="signup")
	{
		if(args.size()!=3)return response="signup expect 3 params, got "+to_string(args.size()),false;
		return DataBase::TrySignup(args[1],args[2],response);
	}else
	// “login <username> <password>”
	if(title=="login")
	{
		if(args.size()!=3)return response="login expect 3 params, got "+to_string(args.size()),false;
		return DataBase::TryLogin(args[1],args[2],response);
	}else
	// “online_users <session_token>”
	if(title=="online_users")
	{
		if(args.size()!=2)return response="online_users expect 2 params, got "+to_string(args.size()),false;
		return DataBase::TryGetOnlineUsers(args[1],response);
	}else
	// “friends <session_token>”
	if(title=="friends")
	{
		if(args.size()!=2)return response="friends expect 2 params, got "+to_string(args.size()),false;
		return DataBase::TryGetFriends(args[1],response);
	}else
	// “add_friend <session_token> <friend username>”
	if(title=="add_friend")
	{
		if(args.size()!=3)return response="add_friend expect 3 params, got "+to_string(args.size()),false;
		return DataBase::TryAddFriend(args[1],args[2],response);
	}else
	// “remove_friend <session_token> <friend username>”
	if(title=="remove_friend")
	{
		if(args.size()!=3)return response="remove_friend expect 3 params, got "+to_string(args.size()),false;
		return DataBase::TryRemoveFriend(args[1],args[2],response);
	}else
	// “get_message <session_token> <message_id>”
	if(title=="get_message")
	{
		if(args.size()!=3)return response="get_message expect 3 params, got "+to_string(args.size()),false;
		return DataBase::TryGetMessage(args[1],args[2],response);
	}else
	// “send_message <session_token> <partner username> <message>”
	if(title=="send_message")
	{
		if(args.size()!=4)return response="send_message expect 4 params, got "+to_string(args.size()),false;
		return DataBase::TrySendMessage(args[1],args[2],args[3],response);
	}else
	// “last_message <session_token> <partner username>”
	if(title=="last_message")
	{
		if(args.size()!=3)return response="last_message expect 3 params, got "+to_string(args.size()),false;
		return DataBase::TryGetLastMessage(args[1],args[2],response);
	}else
	// “next_messages <session_token> <message_id> <desired number of messages to get>”
	if(title=="next_messages")
	{
		if(args.size()!=4)return response="next_messages expect 4 params, got "+to_string(args.size()),false;
		return DataBase::TryGetNextMessages(args[1],args[2],args[3],response);
	}else
	// “prev_messages <session_token> <message_id> <desired number of messages to get>”
	if(title=="prev_messages")
	{
		if(args.size()!=4)return response="prev_messages expect 4 params, got "+to_string(args.size()),false;
		return DataBase::TryGetPreviousMessages(args[1],args[2],args[3],response);
	}else
	{
		return response="unrecognized command: "+title,false;
	}
}
vector<int>SelectRead(const map<int,string>&fds)
{
	vector<int>s;
	for(const auto fd:fds)s.push_back(fd.first);
	return select_read(s);
}
int main(int argc,char *argv[])
{
	if(argc!=2)
	{
		cerr<<"argc = "<<argc<<endl;
		cerr<<"Usage: ./server listen_port"<<endl;
		exit(0);
	}
	const int listen_port=stoi(argv[1]);
	int server_fd;
	if(!init_server(listen_port,10,server_fd))
	{
		cerr<<"Error: init_server()"<<endl;
		exit(0);
	}
	cerr<<"Server fd = "<<server_fd<<". Listening..."<<endl;
	map<int,string>client_fds;
	while(true)
	{
		usleep(1);
		//cerr<<"loop";
		{
			int client_fd;
			uint32_t client_ip;
			int client_port;
			while(accept(server_fd,client_fd,client_ip,client_port))
			{
				//clog<<"accept new client: fd = "<<client_fd<<endl;
				ostringstream oss;
				oss<<ip_to_string(client_ip)<<':'<<client_port;
				oss.flush();
				//cerr<<"accept client: "<<s<<'('<<ip_to_string(client_ip)<<endl;
				client_fds[client_fd]=oss.str();
			}
		}
		//cerr<<" ac"<<endl;
		const auto &readable_client_fds=SelectRead(client_fds);
		vector<int>to_remove;
		for(const int fd:readable_client_fds)
		{
			string msg;
			bool unexpected_error;
			if(receive_string(fd,msg,unexpected_error))
			{
                cout<<"recv from "<<client_fds[fd]<<endl;
				string response="";
				string to_send=ProcessMessage(msg,response)?"AC":"WA";
				if(response!="")to_send+=" "+response;
				if(!send_string(fd,to_send,unexpected_error))cerr<<"send error"<<endl;
//                if(msg!="OK")cerr<<"msg: "<<msg<<endl;
//                cout<<"recv from "<<client_fds[fd]<<endl;
//                if(!send_string(fd,"OK",unexpected_error))
//                {
//                    cerr<<"send error"<<endl;
//                }
				close(fd);
				to_remove.push_back(fd);
			}
			if(unexpected_error)
			{
				close(fd);
				to_remove.push_back(fd);
			}
		}
		for(const int fd:to_remove)client_fds.erase(fd);
	}
	return 0;
}
