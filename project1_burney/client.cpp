#include<bits/stdc++.h>
#include<time.h>
#include<chrono>
#include"common.h"
using namespace std;
using namespace chrono;
class PingJob
{
	public:
	PingJob(const string &_host,const int _port,const int _timeout,const int _id)
		:host(_host),port(_port),timeout(_timeout),id(_id),start_clock(steady_clock::now()),result("timeout when connect to "+host){}
	string host;
	int port,timeout,id;
	void TryPing(string &info,bool &is_time_out)
	{
		if(!nothing_to_do)_TryPing();
		is_time_out=(get_current_delay()>timeout);
		if(!info_given&&(nothing_to_do||is_time_out))info=result,info_given=true;
		if(!nothing_to_do&&is_time_out&&server_fd!=-1)close(server_fd);
	}
	/*bool is_time_out()
	{
		const int current_delay=(int)((clock()-start_clock)*1000/CLOCKS_PER_SEC);
		return current_delay>timeout;
	}*/
	int get_current_delay()
	{
		return (int)(duration_cast<microseconds>(steady_clock::now()-start_clock).count()/1000);
	}
	private:
	steady_clock::time_point start_clock;
	int server_fd;
	bool nothing_to_do=false;
	bool is_handshaking=false,is_connected=false,is_sent=false,info_given=false;
	string result;
	struct sockaddr_in socket_address_info;
	void _TryPing()
	{
		if(!is_handshaking)
		{
			int error_number;
			if(!connect_to(host,port,server_fd,error_number,socket_address_info))
			{
				if(server_fd==-1)return;
				if(error_number!=EINPROGRESS)
				{
					//clog<<host<<" connect error"<<endl;
					close(server_fd);
					nothing_to_do=true;
					return;
				}
				//else clog<<host<<" hand shaking"<<endl;
				is_handshaking=true;
				return;
			}
			assert(server_fd!=-1);
			is_handshaking=is_connected=true;
			return;
		}
		if(!is_connected)
		{
			index_wow_lets_make_it_fail:;
			const auto &rfds=select_read({server_fd});
			const auto &wfds=select_write({server_fd});
			if(!rfds.empty()||!wfds.empty())
			{
				/*if(!wfds.empty())//connect error
				{
					fprintf(stderr,"hand-shaking error\n");
					nothing_to_do=true;
					return;
				}
				else*/
				int error_number;
				if(try_connect(server_fd,error_number,socket_address_info))goto index_wow_lets_make_it_fail;
				if(error_number!=EISCONN)
				{
					//if(error_number==EINPROGRESS)return;
					/*if(error_number!=EINPROGRESS)//EINPROGRESS: maybe the port is incorrect
					{
						clog<<host<<" error_number="<<strerror(error_number)<<endl;
					}*/
					close(server_fd);
					nothing_to_do=true;
					return;
				}
				//clog<<host<<" connected!"<<endl;
				is_connected=true;
			}
			return;
		}
		if(!is_sent)
		{
			bool unexpected_error;
			if(send_string(server_fd,"OK",unexpected_error))
			{
				//clog<<host<<" sent!"<<endl;
				is_sent=true;
			}
			if(unexpected_error)
			{
				close(server_fd);
				nothing_to_do=true;
				return;
			}
			return;
		}
		{
			bool unexpected_error;
			string msg;
			if(receive_string(server_fd,msg,unexpected_error))
			{
				if(msg!="OK")clog<<"msg: "<<msg<<endl;
				const int current_delay=get_current_delay();
				result="recv from "+host+", RTT = "+to_string(current_delay)+" msec";
				close(server_fd);
				nothing_to_do=true;
				return;
			}
			if(unexpected_error)
			{
				close(server_fd);
				nothing_to_do=true;
				return;
			}
			return;
		}
		//cerr<<"no response yet."<<endl;
	}
};
void Ping(const int number,const int timeout,const vector<string>&hosts)
{
	vector<PingJob>jobs;
	for(const auto s:hosts)
	{
		const int i=(int)s.find(':');
		assert(i!=-1);
		jobs.push_back(PingJob(s.substr(0,i),stoi(s.substr(i+1)),timeout,0));
	}
	while(!jobs.empty())
	{
		for(auto it=jobs.begin();it!=jobs.end();++it)
		{
			auto &job=*it;
			string result="";
			bool is_time_out;
			job.TryPing(result,is_time_out);
			if(result!="")cout<<result<<endl;//<<", id="<<job.id<<endl;
			if(is_time_out)
			{
				const auto newJob=PingJob(job.host,job.port,job.timeout,job.id+1);
				jobs.erase(it);
				if(number==0||newJob.id<number)
				{
					jobs.push_back(newJob);
				}
				break;
			}
		}
	}
}
int main(int argc,char *argv[])
{
	int number=0;
	int timeout=1000;
	vector<string>hosts;
	for(int i=1;i<argc;i++)
	{
		const string v=argv[i];
		if(v=="-n")number=stoi(argv[++i]);
		else if(v=="-t")timeout=stoi(argv[++i]);
		else hosts.push_back(v);
	}
	cerr<<"-n "<<number<<endl;
	cerr<<"-t "<<timeout<<endl;
	cerr<<"hosts:";
	for(const string &h:hosts)cerr<<' '<<h;
	cerr<<endl;
	Ping(number,timeout,hosts);
	return 0;
}
