//Based on ROOT spy.C example
//Author: K.Briggl


#include "EventType.h"
#include "DAQctrl_client.h"
#include "TMessage.h"
#include "TClass.h"

DAQctrl::DAQctrl(std::string host):
	m_Socket(NULL)
{
	if(host!="") Connect(host);
}


DAQctrl::~DAQctrl(){
	if(m_Socket!=NULL) delete m_Socket;
}

void DAQctrl::Connect(std::string host)
{
	if(m_Socket!=NULL) delete m_Socket;
	// Connect to DAQctrlServ
	m_Socket = new TSocket(host.c_str(), 9090);
	if ((m_Socket<=NULL) || (!m_Socket->IsValid())){
		m_Socket=NULL;
		return;
	}
	std::cout<<"DAQctrl::Connect(): Connected to "<<host<<std::endl;
	m_hostname=host;
	m_ASICs=FetchListOfASICs();
}

void DAQctrl::CommandWithAck(std::string cmdstring, ...){
	if ((m_Socket==NULL) || (!m_Socket->IsValid()))
		return;

	char buffer[64];
	va_list argptr;
	va_start(argptr, cmdstring);
	int status = vsnprintf(buffer,64, cmdstring.c_str(), argptr);
	va_end(argptr);
	if(status>64){
		std::cout<<"DAQctrl::CommandWithAck() Error: Command string truncated."<<std::endl;
		return;
	}
//**** MUTEX-PROTECTED AREA BEGIN***///
	m_MutexSock.Lock(); 
	//std::cout<<"DAQctrl::CommandWithAck() Sending \""<<buffer<<"\""<<std::endl;
	m_Socket->Send(buffer, kMESS_STRING | kMESS_ACK);
	//std::cout<<"DAQctrl::CommandWithAck() Sending finished, Acknowledge received. \""<<buffer<<"\""<<std::endl;
	m_MutexSock.UnLock(); 
//**** MUTEX-PROTECTED AREA END***///
}

int DAQctrl::CommandRepliesOpCode(std::string cmdstring, ...){
	if ((m_Socket==NULL) || (!m_Socket->IsValid()))
		return -1;

	char buffer[64];
	va_list argptr;
	va_start(argptr, cmdstring);
	int status = vsnprintf(buffer,64, cmdstring.c_str(), argptr);
	va_end(argptr);
	if(status>64){
		std::cout<<"DAQctrl::CommandRepliesOpCode() Error: Command string truncated."<<std::endl;
		return -1;
	}
//**** MUTEX-PROTECTED AREA BEGIN***///
	m_MutexSock.Lock(); 
	//std::cout<<"DAQctrl::CommandWithAck() Sending \""<<buffer<<"\""<<std::endl;
	m_Socket->Send(buffer, kMESS_STRING);
	//std::cout<<"DAQctrl::CommandWithAck() Sending finished, Acknowledge received. \""<<buffer<<"\""<<std::endl;
	int code;
	if (m_Socket->Recv(status,code) <= 0) {
		Error("DAQctrl::CommandRepliesOpCode()", "error receiving message");
		do{
			Error("DAQctrl::CommandRepliesObject()", "Will try to reconnect");
			Connect(m_hostname);
		}while(!Good());
		return -1;
	}
	//std::cout<<"DAQctrl::CommandWithAck() Sending finished, Reply received. \""<<buffer<<"\""<<std::endl;
	m_MutexSock.UnLock(); 
//**** MUTEX-PROTECTED AREA END***///
	return code;
}
TObject* DAQctrl::CommandRepliesObject(const TClass* target_obj, std::string cmdstring, ...){
	TObject* obj=NULL;
	if ((m_Socket==NULL) || (!m_Socket->IsValid()))
		return obj;

//send request
	char buffer[64];
	va_list argptr;
	va_start(argptr, cmdstring);
	int status = vsnprintf(buffer,64, cmdstring.c_str(), argptr);
	va_end(argptr);
	if(status>64){
		std::cout<<"DAQctrl[priv]::CommandRepliesObject() Error: Command string truncated."<<std::endl;
		return obj;
	}

//**** MUTEX-PROTECTED AREA BEGIN***///
	m_MutexSock.Lock(); 
	//std::cout<<"DAQctrl::CommandRepliesObject() Sending \""<<buffer<<"\""<<std::endl;
	m_Socket->Send(buffer, kMESS_STRING);
//get reply
	TMessage *mess;
	if (m_Socket->Recv(mess) <= 0) {
		Error("DAQctrl::CommandRepliesObject()", "error receiving message");
		do{
			Error("DAQctrl::CommandRepliesObject()", "Will try to reconnect");
			Connect(m_hostname);
		}while(!Good());
		return obj;
	}

	m_MutexSock.UnLock(); 
//**** MUTEX-PROTECTED AREA END***///

	if(mess->What()!=kMESS_OBJECT){
		std::cout<<"DAQctrl[priv]::CommandRepliesObject() Error: Message kind ("<<mess->What()<<") is not object!"<<std::endl;
		delete mess;
		return obj;
	}

	//i f (m_results!=NULL) delete m_results; -- note: ownership of object passed to calling function
	if (mess->GetClass()->InheritsFrom(target_obj)){
		obj = mess->ReadObject(mess->GetClass());
	}else{
		std::cout<<"DAQctrl::FetchResults() Error: Reply Object is a "<<mess->ClassName()<<std::endl;
		delete mess;
		return obj;
	}
	//std::cout<<"DAQctrl::CommandRepliesObject(): Finished"<<std::endl;
	delete mess;
	return obj;
}

std::list<unsigned char> DAQctrl::FetchListOfASICs(){
	std::list<unsigned char> li;
	if ((m_Socket==NULL) || (!m_Socket->IsValid()))
		return li;
//**** MUTEX-PROTECTED AREA BEGIN***///
	m_MutexSock.Lock(); 
	m_Socket->Send("get asiclist", kMESS_STRING);
	TMessage *mess;
	//std::cout<<"DAQctrl::FetchListOfASICs() Request sent"<<std::endl;
	if (m_Socket->Recv(mess) <= 0) {
		Error("DAQctrl::FetchListOfASICs()", "error receiving message");
		do{
			Error("DAQctrl::CommandRepliesObject()", "Will try to reconnect");
			Connect(m_hostname);
		}while(!Good());
		return li;
	}
	//std::cout<<"DAQctrl::FetchListOfASICs() Reply received"<<std::endl;
	if(mess->What()!=kMESS_ANY){
		std::cout<<"DAQctrl::FetchListOfASICs() Error: Message kind ("<<mess->What()<<") is not kMESS_ANY!"<<std::endl;
		delete mess;
		return li;
	}

	int n;
        mess->ReadInt(n);
	//std::cout<<"DAQctrl::FetchListOfASICs() List size: "<<n<<std::endl;
	for (;n>0;n--){
		int m;
        	mess->ReadInt(m);
		//std::cout<<"DAQctrl::FetchListOfASICs() List item: "<<m<<std::endl;
		li.push_back(m);
	}
	m_MutexSock.UnLock(); 
//**** MUTEX-PROTECTED AREA END***///
	return li;
}

void DAQctrl::FlushFIFO(int minEvents){
	this->CommandWithAck("flush %d",minEvents);
}	

void DAQctrl::ReadChipUntilEmpty(int minEvents, int maxEvents){
	this->CommandRepliesOpCode("readchip %d %d",minEvents,maxEvents);
}	

void DAQctrl::ReadChipAsyncStart(int usec_sleep, int minEvents,int maxEvents){
	this->CommandWithAck("readchip-start %d %d %d",usec_sleep,minEvents,maxEvents);
}
void DAQctrl::ReadChipAsyncStop(){
	this->CommandRepliesOpCode("readchip-stop");
}

ClassImp(DAQctrl);

