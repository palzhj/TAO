//Histogramming DAQ for K4
//Based on ROOT spyserv.C example
//Author: K.Briggl


#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TList.h"
#ifndef __CINT__
#include "TError.h"
#endif
#include "stdlib.h"
#include "EventType.h"
#include "klaus_i2c_iface.h"

#include "Klaus4Config.h"
#include "VCRemoteClient.h"

#include "HistogrammedResults.h"
#include "HistogramDAQ_server.h"

//#define ddprintf(args...) printf(args)
#define ddprintf(args...)

//#define dprintf(args...) printf(args)
#define dprintf(args...)

struct events_buffer_type{
	unsigned int bufsize;
	unsigned int prescale;
	TList* aqu_buffer;
	events_buffer_type();
	~events_buffer_type();
	void Add(klaus_aquisition& aqu);
	void Clear();
};
events_buffer_type::events_buffer_type(){
	bufsize=1000;
	prescale=1;
	aqu_buffer=NULL;
}

events_buffer_type::~events_buffer_type(){
	dprintf("events_buffer_type::~events_buffer_type()\n");
	Clear();
	if(aqu_buffer!=NULL)
		delete aqu_buffer;
}

void events_buffer_type::Add(klaus_aquisition& aqu){
	if(aqu_buffer==NULL){
		dprintf("DAQServ::events_buffer_type::Add(): New list created.\n");
		aqu_buffer=new TList;
		aqu_buffer->SetOwner();
	}
	if(aqu.aqu_ID%prescale!=0) return;
	if(aqu.nEvents==0) return;
	while(aqu_buffer->GetSize()>=bufsize){
		TObject* atmp=aqu_buffer->Remove(aqu_buffer->FirstLink());
		if(atmp) delete atmp;
	}
	aqu_buffer->AddLast(aqu.Clone());
	ddprintf("DAQServ::events_buffer_type::Add(): Added Aquisition to list; New size=%d.\n",aqu_buffer->GetSize());
}

void events_buffer_type::Clear(){
	if(aqu_buffer==NULL) return;
	aqu_buffer->Clear();
}

DAQServ::DAQServ(klaus_i2c_iface& iface):
	m_iface(iface),
	m_current_aqu_ID(0),
	m_DAQ_thread(NULL)
{
   // Create the server process to fills a number of histograms.
   // A spy process can connect to it and ask for the histograms.
   // There is no apriory limit for the number of concurrent spy processes.

   // Open a server socket looking for connections on a named service or
   // on a specified port
   //TServerSocket *ss = new TServerSocket("spyserv", kTRUE);
   m_Serv = new TServerSocket(9090, kTRUE);
   if (!m_Serv->IsValid())
      exit(1);

   // Add server socket to monitor so we are notified when a client needs to be
   // accepted
   m_Mon  = new TMonitor;
   m_Mon->Add(m_Serv);

   // Create a list to contain all client connections
   m_Sockets = new TList;
   TMessage::EnableSchemaEvolutionForAll();
}

void DAQServ::Run(){
	// Main Loop: Do Sockets
	const Int_t kUPDATE = 1000;
	m_SERVruncondition=DAQServ::RUN;
	while (m_SERVruncondition!=DAQServ::EXIT) {
		while (m_SERVruncondition==DAQServ::RUN) {
			// Check if there is a message waiting on one of the sockets.
			// Wait not longer than 20ms (returns -1 in case of time-out).
			TSocket *s;
			if ((s = m_Mon->Select(20)) != (TSocket*)-1)
				HandleSocket(s);
			//if (gROOT->IsInterrupted())
			//   break;
		}
	}
};
void DAQServ::Suspend(bool status){
	if(status)
		m_SERVruncondition=DAQServ::SUSPEND;
	else
		m_SERVruncondition=DAQServ::RUN;
};
void DAQServ::Stop(){
	m_SERVruncondition=DAQServ::EXIT;
}

//Manually append the ASIC list
void DAQServ::AppendASICList(char slave_addr){m_ASICs.push_back(slave_addr); UpdateResultsList();};
//Manually reset the ASIC list
void DAQServ::ResetASICList(){m_ASICs.clear(); UpdateResultsList();};

// Getting current list of ASICs from SC server
int DAQServ::AutoFetchASICList(const char* host){
	int handle=1;
	ResetASICList();

	//Test if the IB config could be bound, this should always be the case. Retry until this succeeds
	printf("******* STARTING CONFIG SERVER PROBE ******\n");
	printf("Probing config-server @ %s\n",host);
	VCRemoteClientBase::Probe(host,0,true);

	//connect to config server, try binding to handle id 1++ until the connection fails.
	//for succeeded connections, store the I2C address of this ASIC
	printf("Getting list of ASICs from config-server @ %s\n",host);
	while(1){

		VCRemoteClient<TKLauS4Config> it(host,handle);
		if(it.Socket(false)>0){
			long long unsigned int addr;
			it.GetParValueWR("digital/i2c_address",addr);
			printf("HANDLE %d -> ADDR 0x%x\n",handle,addr);
			m_ASICs.push_back(addr);
		}else{
			printf("HANDLE %d NOT VALID, STOPPING\n",handle);
			break;
		}
		handle++;
	}
	printf("******* FINISHED CONFIG SERVER PROBE ******\n");
	printf("*******        ASICS FOUND: %d       ******\n",m_ASICs.size());
	UpdateResultsList();
	return m_ASICs.size();
}

void DAQServ::ReadChipThread(){
	printf("DAQServ::ReadChipThread(): Started\n");
	m_DAQruncondition=DAQServ::RUN;
	while(m_DAQruncondition==DAQServ::RUN){
		ReadChipCmd(m_DAQ_options.min_chip, m_DAQ_options.max_tot);
		usleep(m_DAQ_options.usec_sleep);
	}
	printf("DAQServ::ReadChipThread(): Stopped\n");
}

void DAQServ::ReadChipCmd(int min_chip, int max_tot){
	klaus_aquisition current_aqu=m_iface.ReadEventsUntilEmpty(m_ASICs,min_chip,max_tot);
	current_aqu.aqu_ID=m_current_aqu_ID++;

	//TODO: verify if filling histogram is thread-save
	//Fill HISTO monitor objects
	/*!*/m_DAQ_mutex.Lock();
	std::map<unsigned char, std::list<klaus_event> >::iterator itH;
	for(itH=current_aqu.data.begin();itH!=current_aqu.data.end();++itH){
		//printf("DAQServ::HandleClientRequest(): filling %d events for chip %x\n",itH->first,itH->second.size());
		m_hist_results[itH->first].Fill(itH->second);
	}
	//Fill LIST monitor objects
	ddprintf("ListDAQ: %d queues installed\n",m_EventQueues.size());
	for(auto itLists=m_EventQueues.begin();itLists!=m_EventQueues.end();itLists++){
		ddprintf("ListDAQ Entries: %d : bs=%d ; ps=%d ; nEv=%d\n",itLists->first,itLists->second.bufsize, itLists->second.prescale, itLists->second.aqu_buffer!=NULL?itLists->second.aqu_buffer->GetSize():-1);
		ddprintf("DAQServ::HandleClientRequest(): filling %d events for Queue #%d\n",current_aqu.nEvents,itLists->first);
		itLists->second.Add(current_aqu);
	}
	/*!*/m_DAQ_mutex.UnLock();
}
void DAQServ::UpdateResultsList(){
	dprintf("DAQServ::UpdateResultsList()\n");
	m_hist_results.clear();

	for(std::list<unsigned char>::iterator it=m_ASICs.begin();it!=m_ASICs.end();it++){
		//Histogram results for each ASIC, shared by all connected sockets
		m_hist_results[*it].SetPrefix(*it);
	}
}


void DAQServ::HandleSocket(TSocket *s)
{
	if (s->IsA() == TServerSocket::Class()) { //New client wants to connect: Set Up
		TSocket *sock = ((TServerSocket*)s)->Accept();
		int sockID=sock->GetDescriptor();
		m_Mon->Add(sock);
		m_Sockets->Add(sock);
		printf("DAQServ::HandleSocket(): accepted connection from %s : sockID=%d\n", sock->GetInetAddress().GetHostName(),sockID);
	} else {
		int sockID=s->GetDescriptor();
		char request[64];
		if (s->Recv(request, sizeof(request)) <= 0) { //Client disconnected: Clean up
			printf("DAQServ::HandleSocket(): closed connection from %s : sockID=%d\n", s->GetInetAddress().GetHostName(),sockID);
			//Remove Event buffer if any
			auto buffer_obj=m_EventQueues.find(sockID);
			if(buffer_obj!=m_EventQueues.end()){
				printf("DAQServ::HandleSocket(): removing DAQ buffer\n");
				m_EventQueues.erase(buffer_obj);
			}
			//Remove Socket from from client list
			m_Mon->Remove(s);
			m_Sockets->Remove(s);
			delete s;
			return;

		}else{ //Good request: Handle it!
			HandleClientRequest(s,request);
		}
	}
}

void DAQServ::HandleClientRequest(TSocket* s, char* request){
	int n,o,p,q;
	int ret;
	// send requested object back
	TMessage answer(kMESS_OBJECT);
	//SYSTEM requests
	if (!strcmp(request,"get asiclist")){
		printf("DAQServ::HandleClientRequest(): get asiclist request\n");
		answer.WriteInt(m_ASICs.size());
		for (unsigned char ID : m_ASICs){
			answer.WriteInt(ID);
		}
		answer.SetWhat(kMESS_ANY);
		s->Send(answer);


	//PIPE reconstruction parameters update
	}else if(sscanf(request,	"pipe_param %d %d %d %d",&n,&o,&p,&q)==4){
		if(m_hist_results.find(n)==m_hist_results.end() || o>6){
			printf("DAQServ::HandleClientRequest(): pipeline parameters ASIC %u Channel %d out of range / not found\n",n,o);
			//answer.SetWhat(kMESS_NOTOK);
		}else{
			printf("DAQServ::HandleClientRequest(): Pipeline parameters of ASIC %u Channel %d updated for subrange %d with offset %d\n",n,o,p,q);
			//Now you have to update the subrange offset range one by one
                        m_hist_results[n].setSubRangeOffset(o,p,q);
			//answer.SetWhat(kMESS_OK);
		}
		//s->Send(answer);


	//DAQ requests
	}else if(sscanf(request,	"flush %d",&n)==1){
		dprintf("DAQServ::HandleClientRequest(): flush request\n");
		/*!*/m_DAQ_mutex.Lock();
		ret=m_iface.FlushFIFO(m_ASICs,n);
		/*!*/m_DAQ_mutex.UnLock();
		//s->Send(kMESS_OK);

	}else if(sscanf(request,	"readchip %d %d",&n,&o)==2){
		dprintf("DAQServ::HandleClientRequest(): readchip request\n");
		ReadChipCmd(n,o);
		s->Send(kMESS_OK);

	}else if(sscanf(request,	"readchip-start %d %d %d",&m_DAQ_options.usec_sleep, &m_DAQ_options.min_chip, &m_DAQ_options.max_tot)==3){
		if(m_DAQ_thread==NULL){
			printf("DAQServ::HandleClientRequest(): readchip-start request: Starting\n");
			m_DAQ_thread=new TThread("DAQ",&DAQServ::ReadChipThreadStart,(void*) this);
			m_DAQ_thread->Run();
		}else{
			printf("DAQServ::HandleClientRequest(): readchip-start request: Already running\n");
		}
		//s->Send(kMESS_OK);

	}else if (!strcmp(request, 	"readchip-stop")){
		if(m_DAQ_thread==NULL){
			printf("DAQServ::HandleClientRequest(): readchip-start request: Not running\n");
		}else{
			printf("DAQServ::HandleClientRequest(): readchip-start request: Stopping\n");
			m_DAQruncondition=DAQServ::EXIT;
			m_DAQ_thread->Join();
			delete m_DAQ_thread;
			m_DAQ_thread=NULL;
			printf("DAQServ::HandleClientRequest(): readchip-start request: Stopped\n");
		}
		s->Send(kMESS_OK);

	//histogram requests
	}else if ((sscanf(request,	"get histos %d",&n)==1)){
		if((n>=0) && (m_hist_results.find(n)==m_hist_results.end())){
			printf("DAQServ::HandleClientRequest(): get histos: ASIC %u not found\n",n);
			answer.SetWhat(kMESS_NOTOK);
		}else{
			dprintf("DAQServ::HandleClientRequest(): get histos request for ASIC (%d)\n",n);
			if(n<0)
				answer.WriteObject(&(m_hist_results.begin()->second));
			else
				answer.WriteObject(&(m_hist_results[n]));

			answer.SetWhat(kMESS_OBJECT);
		}
		//printf("DAQServ::HandleClientRequest(): current list of results: %d ASICs\n",m_hist_results.size());
		s->Send(answer);

	}else if (!strcmp(request, 		"reset histos")){
		dprintf("DAQServ::HandleClientRequest(): reset histos request\n");
		std::map<unsigned char, HistogrammedResults >::iterator it=m_hist_results.begin();
		while(it!=m_hist_results.end()){
			it->second.Reset();
			++it;
		}
		//answer.SetWhat(kMESS_OK);
		//s->Send(answer);

	//list DAQ requests
	}else if(sscanf(request,	"register list %d %d",&n,&o)==2){
		int sockID=s->GetDescriptor();
		dprintf("DAQServ::HandleClientRequest(): register list request: sockID=%d, buffer=%d, prescale=%d\n",sockID,n,o);

		//Update/Create Events buffer. Resize is done at next Add() when needed
		/*!*/m_DAQ_mutex.Lock();
		m_EventQueues[sockID].bufsize=n;
		m_EventQueues[sockID].prescale=o;
		/*!*/m_DAQ_mutex.UnLock();
		

	}else if (!strcmp(request, 		"get list")){
		int sockID=s->GetDescriptor();
		/*!*/m_DAQ_mutex.Lock();
		auto buffer_obj=m_EventQueues.find(sockID);
		if( buffer_obj==m_EventQueues.end() || (buffer_obj->second.aqu_buffer==NULL)){
			printf("DAQServ::HandleClientRequest(): get list for queue #%d: Not installed or empty\n",sockID);
			answer.SetWhat(kMESS_NOTOK);
		}else{
			dprintf("DAQServ::HandleClientRequest(): get list for queue #%d: request\n",sockID);
			answer.WriteObject(buffer_obj->second.aqu_buffer);
			answer.SetWhat(kMESS_OBJECT);
			dprintf("DAQServ::HandleClientRequest(): get list for queue #%d: buffer had %d Entries\n",sockID, buffer_obj->second.aqu_buffer->GetSize());
			buffer_obj->second.Clear();
		}
		s->Send(answer);
		/*!*/m_DAQ_mutex.UnLock();
	}else if (!strcmp(request, 		"reset list")){
		int sockID=s->GetDescriptor();
		/*!*/m_DAQ_mutex.Lock();
		auto buffer_obj=m_EventQueues.find(sockID);
		if( buffer_obj!=m_EventQueues.end() ){
			dprintf("DAQServ::HandleClientRequest(): clearing queue for sockID=%d\n",sockID);
			if(buffer_obj->second.aqu_buffer != NULL){
				dprintf("   buffer had %d Entries\n",buffer_obj->second.aqu_buffer->GetSize());
				buffer_obj->second.Clear();
			}
			//answer.SetWhat(kMESS_OK);
		}else{
			printf("DAQServ::HandleClientRequest(): reset list request: sockID=%d not registered\n",sockID);
			//answer.SetWhat(kMESS_NOTOK);
		}
		/*!*/m_DAQ_mutex.UnLock();
		//s->Send(answer);

	//unknown request
	}else{
		printf("DAQServ::HandleClientRequest(): Unexpected request %s\n",request);
		//answer.SetWhat(kMESS_NOTOK);
		//s->Send(answer);
	}
}

DAQServ::~DAQServ()
{
   // Clean up

   m_Sockets->Delete();
   delete m_Sockets;
   delete m_Serv;
}



