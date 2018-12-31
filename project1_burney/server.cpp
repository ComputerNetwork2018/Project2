#include<bits/stdc++.h>
#include"common.h"
using namespace std;
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
				if(msg!="OK")cerr<<"msg: "<<msg<<endl;
				cout<<"recv from "<<client_fds[fd]<<endl;
				if(!send_string(fd,"OK",unexpected_error))
				{
					cerr<<"send error"<<endl;
				}
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
