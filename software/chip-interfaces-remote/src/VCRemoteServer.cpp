/*
	VC remote server and session handler classes. 
	Server will accept new connections and start session threads.
	The session threads will wait for requests to come in and pass to the underlying TVirtualConfig instance
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "VCRemoteServer.h"
#include <utility>

VCRemoteServer::VCRemoteSession::VCRemoteSession(int socket,VCRemoteServer* parent)
	:
	m_fd(socket),
	m_seek(0),
	m_type(VCRemoteSession::TYPE_NONE),
	m_config(NULL),
	m_parent(parent)
	{};


VCRemoteServer::VCRemoteSession::~VCRemoteSession(){close(m_fd);};

//Try to get delimited field from buffer.
//  Returns -1 if not enough data to read, 0 otherwise
//  Retrieved field is stored in line.
int VCRemoteServer::VCRemoteSession::GetField(std::string& line){
//	printf("VCRemoteSession::GetField<std::string>()\n");
//	printf("Buffer now: \"%s\", %d\n",m_buf.str().c_str(),GetBufAvail());
	std::getline(m_buf,line,':');
	if(!m_buf.good()){
		//undo last get, we want to read this later
		if(line.length()!=0){ for(int i=0;i<line.length();i++) m_buf.rdbuf()->sungetc(); }
		//printf("Waiting for more...\n");
		return -1;
	}
//	printf("New flag: \"%s\"\n",line.c_str());
	return 0;
}
//Try to get binary field from buffer, with given length.
//  Returns -1 if not enough data to read, 0 otherwise
//  data is reallocated as soon as it is possible to read all bytes.
//  if the method succeeds (return==0), the user is supposed to free the data
int VCRemoteServer::VCRemoteSession::GetField(int len,char*& data){
//	printf("VCRemoteSession::GetField<buffer>()\n");
	if(GetBufAvail()<len+1){ //all needed bytes available?
//		printf("Waiting for more...\nBuffer now: (%d bytes)",GetBufAvail());
		return -1;
	}
	data=(char*)realloc(data,len);
	if(data==NULL) return -1;
	m_buf.read(data,	len);
	char delim;
	m_buf.read(&delim,	1);
	if(delim!=':'){
		free(data);
		printf("VCRemoteSession::GetField(buffer): Delimiter failure\n");
		return -1;
	}
	return 0;
}
//Other field getters:
int VCRemoteServer::VCRemoteSession::GetField(unsigned short & value){
//	printf("VCRemoteSession::GetField<short unsigned int>()\n");
	std::string line;
	if(GetField(line)<0)
		return 0;
	if(sscanf(line.c_str(),"%u",&value)!=1)
	{
		printf("Error in GetField<unsigned short>: line is %s\n",line.c_str());
		return -1;
	}
	return 1;
}
int VCRemoteServer::VCRemoteSession::GetField(unsigned long long & value){
//	printf("VCRemoteSession::GetField<long long unsigned int>()\n");
	std::string line;
	if(GetField(line)<0)
		return 0;
	if(sscanf(line.c_str(),"%lu",&value)!=1)
	{
		printf("Error in GetField<long long unsigned>: line is %s\n",line.c_str());
		return -1;
	}
	return 1;
}
int VCRemoteServer::VCRemoteSession::GetField(char & value){
//	printf("VCRemoteSession::GetField<char>()\n");
	std::string line;
	if(GetField(line)<0)
		return 0;
	if(line.length()!=1)
	{
		printf("Error in GetField<char>: line is %s\n",line.c_str());
		return -1;
	}
	value=line.c_str()[0];
	return 1;
}

//Retrieve MethodID flag from stream, set m_type flag. Return 0 if not enough data, -1 if invalid flag.
int VCRemoteServer::VCRemoteSession::HandleMethodID(){
//	printf("VCRemoteSession::HandleMethodID()\n");
	std::string line;
	//printf("line: \"%s\"\n",line.c_str());
	if(GetField(line)<0)
		return 0;
	//get method ID
	if(line=="INI"){
//		printf("Got init\n");
		m_type=TYPE_INI;
	}else
	if(line=="UPD"){
//		printf("Got UpdateConfig\n");
		m_type=TYPE_UPD;
	}else
	if(line=="VAL"){
//		printf("Got ValidatePattern\n");
		m_type=TYPE_VAL;
	}else
	if(line=="CMD"){
//		printf("Got Command\n");
		m_type=TYPE_CMD;
	}else
	if(line=="GRD"){
//		printf("Got GetParValueRD\n");
		m_type=TYPE_GRD;
	}else
	if(line=="GWR"){
//		printf("Got GetParValueWR\n");
		m_type=TYPE_GWR;
	}else
	if(line=="SET"){
//		printf("Got SetParValue\n");
		m_type=TYPE_SET;
	}else{
		printf("VCRemoteSession::HandleMethodID(): Got Unknown command flag. Will disconnect.\n");
		return -1;
	}
	m_seek=1;
	return 1;
}

int VCRemoteServer::VCRemoteSession::NewData(char * buf, int len){
	//reset the error flags of the stream. it might be in a failbit-state due to a (failed) read in an earlier call. before appending new data, have to reset them.
	m_buf.clear();
	m_buf.write(buf,len);
	int ret;
	//printf("Session #%d: Got %d bytes of new data. Size is now: %d. Type:%d\n",this->GetFD(),len,this->GetBufAvail(),m_type);
	while(1){
		//handle types
		switch(m_type){
			case TYPE_NONE:{//handle method id field
				if(m_type==TYPE_NONE){
					if((ret=HandleMethodID())<=0)
						return ret;
					//enforce init is the first type after construction
					if((m_config==NULL)&&(m_type!=TYPE_INI)){
						printf("m_type!=TYPE_INIT && m_config==NULL. Will disconnect...\n");
						return -1;
					}
				}
				break;
			}
			case TYPE_INI:{
				short unsigned int ifaceHandID;
				//get the HandleID to bind to
				if((ret=GetField(ifaceHandID))<=0)
					return ret;
				printf("Would bind to interface %d\n",ifaceHandID);
				m_config=m_parent->GetConfiguration(ifaceHandID);
				if(m_config==NULL) return -1;
				//REPLY
				char reply[100];
				sprintf(reply,"INI:%d:",ifaceHandID);
				ret=send(GetFD(),reply,strlen(reply),0);
				if(ret<strlen(reply)){
					printf("Sending reply failed");
					return -1;
				}
				m_type=TYPE_NONE;
				m_seek=0;
				break;
			}
			case TYPE_UPD:{
				//virtual int UpdateConfig()
				//printf("Would do/g UpdateConfig()\n");
				int returnval=m_config->UpdateConfig();

				//REPLY
				char reply[100];
				sprintf(reply,"UPD:%d:",returnval);
				ret=send(GetFD(),reply,strlen(reply),0);
				if(ret<strlen(reply)){
					printf("Sending reply failed");
					return -1;
				}

				m_type=TYPE_NONE;
				m_seek=0;
				break;
			}
			case TYPE_VAL:{
				//virtual int ValidatePattern()
				//printf("Would do/g ValidatePattern()\n");
				int returnval=m_config->ValidatePattern();

				//REPLY
				char reply[100];
				sprintf(reply,"VAL:%d:",returnval);
				ret=send(GetFD(),reply,strlen(reply),0);
				if(ret<strlen(reply)){
					printf("Sending reply failed");
					return -1;
				}

				m_type=TYPE_NONE;
				m_seek=0;
				break;
			}
			case TYPE_SET:{
				//virtual int SetParValue(short int npar,unsigned long long value)
				if(m_seek==1){ //get the ParamID
					if((ret=GetField(m_streamed.paramID))<=0)
						return ret;
					m_seek=2;
				}
				if(m_seek==2){//bind to configuration
					unsigned long long value;
					if((ret=GetField(value))<=0){
						return ret;
					}
					//printf("Would do/g SetParValue(%d,%d)\n",m_streamed.paramID,value);
					m_config->SetParValue(m_streamed.paramID,value);
					m_type=TYPE_NONE;
					m_seek=0;
				}
				break;
			}
			case TYPE_GRD:{
				//virtual int GetParValueRD(short int npar,unsigned long long &value)
				if(m_seek==1){ //get the ParamID
					if((ret=GetField(m_streamed.paramID))<=0)
						return ret;
					unsigned long long value;
					//printf("Would do/g GetParValueRD(%d)\n",m_streamed.paramID);
					m_config->GetParValueRD(m_streamed.paramID,value);

					//REPLY
					char reply[100];
					sprintf(reply,"GRD:%d:",value);
					ret=send(GetFD(),reply,strlen(reply),0);
					if(ret<strlen(reply)){
						printf("Sending reply failed");
						return -1;
					}

					m_type=TYPE_NONE;
					m_seek=0;
				}
				break;
			}
			case TYPE_GWR:{
				//virtual int GetParValueWR(short int npar,unsigned long long &value)
				if(m_seek==1){ //get the ParamID
					if((ret=GetField(m_streamed.paramID))<=0)
						return ret;
					unsigned long long value;
					//printf("Would do/g GetParValueWR(%d)\n",m_streamed.paramID);
					m_config->GetParValueWR(m_streamed.paramID,value);

					//REPLY
					char reply[100];
					sprintf(reply,"GWR:%d:",value);
					ret=send(GetFD(),reply,strlen(reply),0);
					if(ret<strlen(reply)){
						printf("Sending reply failed");
						return -1;
					}

					m_type=TYPE_NONE;
					m_seek=0;
				}
				break;
			}
			case TYPE_CMD:{
				//virtual int IssueCommand(const char cmd,  int len, char* data, int reply_len=-1, char* reply=0)
				if(m_seek==1){ //get the cmd flag
					if((ret=GetField(m_streamed.cmd))<=0)
						return ret;
					m_seek=2;
				}
				if(m_seek==2){ //get the data_length flag
					if((ret=GetField(m_streamed.len))<=0)
						return ret;
					m_seek=3;
				}
				if(m_seek==3){ //get the reply_length flag
					if((ret=GetField(m_streamed.reply_len))<=0)
						return ret;
					m_seek=4;
				}
				if(m_seek==4){ //get the data, do it
					char* data=NULL;
					if((ret=GetField(m_streamed.len,data))<=0)
						return ret;
					if(m_streamed.reply_len<0)
						m_streamed.reply_len=0;
					char* reply=(char*)malloc(4+m_streamed.reply_len+4); //cmd reply header+reply+delimiter
					if(reply==NULL) return -1;
					//printf("Would do/g Command(%c,%d,[...],%d)\n",m_streamed.cmd,m_streamed.len,m_streamed.reply_len);
					m_config->IssueCommand(m_streamed.cmd,m_streamed.len,data,m_streamed.reply_len,reply);
					//append header&trailer
					sprintf(reply,"CMD:");
					reply[4+m_streamed.reply_len]=':';
					//send reply
					ret=send(GetFD(),reply,4+m_streamed.reply_len+1,0);
					if(ret<strlen(reply)){
						printf("Sending reply failed");
						return -1;
					}

					m_type=TYPE_NONE;
					m_seek=0;
				}	
				break;
			}
			default: {
				printf("Undefined type\n");
				return -1;
			}
		}//switch type
	}//while 1
	return 0;
}

void VCRemoteServer::AddSession(int socket){
	//register StrmClientBuf
	//FD_CLR(m_serversocket,&master_set);
	FD_SET(socket,&m_rd_set);
	FD_SET(socket,&m_rd_set); if(m_fdsets_maxfd <= socket) m_fdsets_maxfd=socket+1;
	m_sessionList.push_back(new VCRemoteSession(socket,this));
}

void VCRemoteServer::RemoveSession(std::list<VCRemoteSession*>::iterator session){
	FD_CLR((*session)->GetFD(),&m_rd_set);
	delete *session;
	session=m_sessionList.erase(session);
}

VCRemoteServer::VCRemoteServer(const char* port)
	:
	m_runflag(true)
{
//fd select related
	int retval;

//connection stuff
	struct addrinfo hints, *servinfo, *p;
	int yes=1;

//---- prepare m_serversocket socket
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((retval = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((m_serversocket = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(m_serversocket, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
			close(m_serversocket);
			m_serversocket=-1;
			perror("setsockopt");
			return;
		}
		if (bind(m_serversocket, p->ai_addr, p->ai_addrlen) == -1) {
			close(m_serversocket);
			m_serversocket=-1;
			perror("server: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		m_serversocket=-1;
		return;
	}
	freeaddrinfo(servinfo); // all done with this structure


	if (listen(m_serversocket, 1) == -1) {
		close(m_serversocket);
		m_serversocket=-1;
		perror("listen");
		return;
	}


//prepare select() structures to wait for data
	//set for people who can talk to server (m_serversocket ->new Session to connect; m_sessionList[] -> get data )
	FD_ZERO(&m_rd_set);		//wait until data can be read from fd
	FD_ZERO(&m_wr_set);		//wait until data can be sent (probably an empty set)

	//add standard fd's to list
	m_fdsets_maxfd=-1;
	FD_SET(m_serversocket,&m_rd_set); if(m_fdsets_maxfd <= m_serversocket) m_fdsets_maxfd=m_serversocket+1;	

}


void VCRemoteServer::Run(){
//network related
	struct timeval timeval;
	int retval;
	fd_set rd_setTMP,wr_setTMP;
	struct sockaddr_storage session_addr; // connector's address information
	socklen_t sin_size;
//streaming
	char* buf=(char*)malloc(2048);

//lets go!
	while (m_runflag){
		timeval.tv_sec=1;
		timeval.tv_usec=0;
		memcpy(&rd_setTMP,&m_rd_set,sizeof(m_rd_set));
		memcpy(&wr_setTMP,&m_wr_set,sizeof(m_wr_set));

		//check what IO can be done.
		retval=select(m_fdsets_maxfd+1,&rd_setTMP,&wr_setTMP,NULL,&timeval);
		if (retval== -1){
			perror("(W) Server: select() error");
			if(errno==EINTR) continue;
			break;
		}
		if (retval==0){
			//printf("select timeout\n");
			continue;
		}

		//printf("(I) Server: New data on %d fd's, %d clients in list\n",retval,m_sessionList.size());

		if (FD_ISSET(m_serversocket,&rd_setTMP)){//data on m_serversocket
				int sess_sock = accept(m_serversocket, (struct sockaddr *)&session_addr, &sin_size);
				if (sess_sock == -1) {
					perror("accept");
					break;
				}
				

				char s[INET6_ADDRSTRLEN];
				inet_ntop(AF_INET, &(((struct sockaddr_in*)&session_addr)->sin_addr),
					s, sizeof(s));
				printf("(I) Server: New connection from %s\n", s);
				AddSession(sess_sock);

		}//m_serversocket readout


		for (std::list<VCRemoteSession*>::iterator it=m_sessionList.begin(); it!=m_sessionList.end();it++){//check each Session socket for data
			if ( (((*it)->GetFD())<=0) ||  !(FD_ISSET((*it)->GetFD(),&rd_setTMP)) ) //no data to read from this socket
				continue;
			//data can be read from StrmClientBuf
			int len_is = recv((*it)->GetFD(), buf, sizeof(buf), 0);
			if (len_is == -1) {
				perror("(W) Server, recv from session socket");
				continue;
			}

			//add data to the Sessions's streambuffer
			//remove Session if it has disconnected
			if( ((*it)->NewData(buf,len_is)<0)  || (len_is==0) ){
				printf("(I) Server: Session #%d seems to have disconnected\n",(*it)->GetFD());				
				RemoveSession(it);
				it--;
			}	
		}//StrmClientBuf list for loop
	}
}

int VCRemoteServer::AddConfiguration(TVirtualConfig* conf){
	m_configList.push_back(conf);
	return m_configList.size()-1;
}

TVirtualConfig* VCRemoteServer::GetConfiguration(int n){
	//printf("VCRemoteServer::GetConfiguration(%d): list has %d entries:\n",n,m_configList.size());
	try{
		return m_configList.at(n);
	}catch(std::out_of_range){return NULL;}

}
