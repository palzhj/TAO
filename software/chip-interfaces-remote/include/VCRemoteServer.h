/*
	VC remote server and session handler classes. 
	Server will accept new connections and start session threads.
	The session threads will wait for requests to come in and pass to the underlying TVirtualConfig instance
*/

#include <iostream>
#include "VirtualConfig.h"
#include <deque>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <mutex>
#include <memory>
const int max_length = 1024;
class VCRemoteServer{
private:
	std::deque<TVirtualConfig*> m_configList;
	//server listener
	int m_serversocket;
	bool m_runflag;
	//sessions

	class VCRemoteSession{
	private: 
		int m_fd;
		std::stringstream m_buf;
		int m_seek;
		enum {
			TYPE_NONE,
			TYPE_INI,
			TYPE_UPD,
			TYPE_VAL,
			TYPE_CMD,
			TYPE_GRD,
			TYPE_GWR,
			TYPE_SET
		} m_type;
		struct {
			unsigned short int paramID;
			char cmd;
			unsigned short int len;
			unsigned short int reply_len;
		} m_streamed;
		TVirtualConfig* m_config;
		VCRemoteServer* m_parent;

		int GetBufAvail(){return m_buf.tellp() - m_buf.tellg();};
		int GetField(std::string& line);	
		int GetField(int len,char*& data);
		int GetField(short unsigned int &val);		
		int GetField(unsigned long long int &val);		
		int GetField(char &val);		
		int HandleMethodID();
	public:
		VCRemoteSession(int socket,VCRemoteServer* parent);
		~VCRemoteSession();
		int GetFD(){return m_fd;};
		int NewData(char * buf, int len);
	}; //class VCRemoteSession

	std::list<VCRemoteSession*> m_sessionList;
	fd_set m_rd_set, m_wr_set;
	int m_fdsets_maxfd; 	//which fd's to check

	void AddSession(int socket);
	void RemoveSession(std::list<VCRemoteSession*>::iterator session_it);
public:
	VCRemoteServer(const char* port);
	void Run();
	void Stop();
	int AddConfiguration(TVirtualConfig* conf);
	TVirtualConfig* GetConfiguration(int n);
}; //Class Server

