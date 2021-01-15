//Histogramming DAQ for K4
//Based on ROOT spyserv.C example
//Author: K.Briggl


#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TList.h"
#include "TClass.h"
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
	void Add(klaus_acquisition& aqu);
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

void events_buffer_type::Add(klaus_acquisition& aqu){
	if(aqu_buffer==NULL){
		dprintf("DAQServ::events_buffer_type::Add(): New list created.\n");
		aqu_buffer=new TList;
		aqu_buffer->SetOwner();
	}
	if(aqu.acqu_ID%prescale!=0) return;
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

void *ReadChipThreadStart(void* classptr){((DAQServ*)classptr)->ReadChipThread(); return 0;};
void *ReadCECThreadStart(void* classptr) {((DAQServ*)classptr)->ReadCECThread(); return 0;};

DAQServ::DAQServ(klaus_i2c_iface& iface):
	m_iface(iface),
	m_current_aqu_ID(0),
	m_DAQ_thread(NULL),
	m_ASIC_bound(-1),
	m_results(NULL),
	m_res(NULL)
{
   // Create the server process to fills a number of histograms.
   // A spy process can connect to it and ask for the histograms.
   // There is no apriory limit for the number of concurrent spy processes.

   // Open a server socket looking for connections on a named service or
   // on a specified port
   //TServerSocket *ss = new TServerSocket("spyserv", kTRUE);
   //m_Serv = new TServerSocket(9090, kTRUE);
   //if (!m_Serv->IsValid())
   //   exit(1);

   // Add server socket to monitor so we are notified when a client needs to be
   // accepted
   //m_Mon  = new TMonitor;
   //m_Mon->Add(m_Serv);

   // Create a list to contain all client connections
   //m_Sockets = new TList;
   //TMessage::EnableSchemaEvolutionForAll();
   m_DAQ_options.min_chip = 0;
   m_DAQ_options.max_tot = -1;
   m_DAQ_options.usec_sleep = 0;
   m_DAQ_options.usec_sleep_cec = 0;
                
   RegisterQueue(2000,1);
}

void DAQServ::Run(){
	// Main Loop: Do Sockets
	const Int_t kUPDATE = 1000;
	m_SERVruncondition=DAQServ::RUN;
/*	while (m_SERVruncondition!=DAQServ::EXIT) {
		while (m_SERVruncondition==DAQServ::RUN) {
			// Check if there is a message waiting on one of the sockets.
			// Wait not longer than 20ms (returns -1 in case of time-out).
			//TSocket *s;
			//if ((s = m_Mon->Select(20)) != (TSocket*)-1)
				//HandleSocket(m_ASIC_bound);
			if (gROOT->IsInterrupted())
			   break;
		}
	}*/
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

void DAQServ::FetchResults() {
	//FetchListResults();
	ReadChipAsyncStart(0,0,-1);
    
	if((m_ASIC_bound>=0) && (m_hist_results.find(m_ASIC_bound)==m_hist_results.end())){
            printf("DAQServ::FetchResults(): get histos: ASIC %u not found\n",m_ASIC_bound);
        }else{
            dprintf("DAQServ::FetchResults(): get histos request for ASIC (%d)\n",m_ASIC_bound);
           if(m_ASIC_bound<0)
             m_results = &(m_hist_results.begin()->second);
           else
             m_results = &(m_hist_results[m_ASIC_bound]);
        }
}

HistogrammedResults* DAQServ::GetResults() {
	return m_results;
}

void DAQServ::UpdateParams_12b(int channel, int branch, int branchDis){
	if(m_hist_results.find(m_ASIC_bound)==m_hist_results.end() || channel>6){
          printf("DAQServ::UpdateParams_12b(): pipeline parameters ASIC %u Channel %d out of range / not found\n", m_ASIC_bound, channel);
        }else{
          printf("DAQServ::UpdateParams_12b(): Pipeline parameters of ASIC %u Channel %d updated for subrange %d with offset %d\n",m_ASIC_bound,channel,branch,branchDis);
          //Now you have to update the subrange offset range one by one
          m_hist_results[m_ASIC_bound].setSubRangeOffset(channel, branch, branchDis);
        }
}

void DAQServ::FlushFIFO(int minEvents) {
	dprintf("DAQServ::FlushFIFO(): flush request\n");
        ///*!*/m_DAQ_mutex.Lock();
        int ret=m_iface.FlushFIFO(m_ASICs, minEvents);
        klaus_cec_data tmp_cec;
        m_iface.ReadCEC(*m_ASICs.begin(),tmp_cec);
        ///*!*/m_DAQ_mutex.UnLock();
        //s->Send(kMESS_OK);
}

void DAQServ::ReadChipUntilEmpty(int minEvents, int maxEvents) {
	dprintf("DAQServ::ReadChipUntilEmpty(): readchip request\n");
        ReadChipCmd(minEvents, maxEvents);
}

void DAQServ::ReadChipAsyncStart(int usec_sleep, int minEvents,int maxEvents) {
	if(m_DAQ_thread==NULL){
           printf("DAQServ::ReadChipAsyncStart(): readchip-start request: Starting\n");
           m_DAQ_thread=new TThread("DAQ",::ReadChipThreadStart,(void*) this);
           m_DAQ_thread->Run();
        }else{
           dprintf("DAQServ::ReadChipAsyncStart(): readchip-start request: Already running\n");
        }
}

void DAQServ::ReadCECAsyncStart(int usec_sleep) {
	if(m_DAQ_thread==NULL){
           printf("DAQServ::ReadCECAsyncStart(): readCEC-start request: Starting\n");
           m_DAQ_thread=new TThread("DAQ",::ReadCECThreadStart,(void*) this);
           m_DAQ_thread->Run();
         }else{
           printf("DAQServ::ReadCECAsyncStart(): readCEC-start request: Already running\n");
         }
}

void DAQServ::ReadChipAsyncStop() {
	if(m_DAQ_thread==NULL){
           printf("DAQServ::ReadChipAsyncStop(): readchip-start request: Not running\n");
        }else{
           printf("DAQServ::ReadChipAsyncStop(): readchip-start request: Stopping\n");
           m_DAQruncondition=DAQServ::EXIT;
           m_DAQ_thread->Join();
           delete m_DAQ_thread;
           m_DAQ_thread=NULL;
           printf("DAQServ::ReadChipAsyncStop(): readchip-start request: Stopped\n");
        }
}

void DAQServ::ResetCEC() {
        ///*!*/m_DAQ_mutex.Lock();
        auto buffer_obj=m_cec_results.find(m_ASIC_bound);
        if( buffer_obj!=m_cec_results.end() ){
            dprintf("DAQServ::ResetCEC(): resetting CEC results for sockID=%d\n",m_ASIC_bound);
            buffer_obj->second.Clear();
            //answer.SetWhat(kMESS_OK);
        }else{
            printf("DAQServ::ResetCEC(): reset CEC request: sockID=%d not registered\n",m_ASIC_bound);
            m_cec_results[m_ASIC_bound].Clear();
            //answer.SetWhat(kMESS_NOTOK);
        }
        ///*!*/m_DAQ_mutex.UnLock();
        //s->Send(answer);
}

klaus_cec_data* DAQServ::FetchCEC(){
        ///*!*/m_DAQ_mutex.Lock();
        auto buffer_obj=m_cec_results.find(m_ASIC_bound);
        if( buffer_obj==m_cec_results.end()){
            printf("DAQServ::FetchCEC(): get cec results #%d: Not installed or empty\n",m_ASIC_bound);
            m_cec_results[m_ASIC_bound].Clear();
            //answer.SetWhat(kMESS_NOTOK);
        }else{
            dprintf("DAQServ::FetchCEC(): get cec results #%d request\n",m_ASIC_bound);
            return &(buffer_obj->second);
            //answer.SetWhat(kMESS_OBJECT);
        }
        //s->Send(answer);
        ///*!*/m_DAQ_mutex.UnLock();
}

void DAQServ::RegisterQueue(unsigned int buflen, unsigned int prescale) {
        dprintf("DAQServ::RegisterQueue(): register list request: sockID=%d, buffer=%d, prescale=%d\n",m_ASIC_bound, buflen, prescale);

        //Update/Create Events buffer. Resize is done at next Add() when needed
        ///*!*/m_DAQ_mutex.Lock();
        m_EventQueues[m_ASIC_bound].bufsize=buflen;
        m_EventQueues[m_ASIC_bound].prescale=prescale;
        ///*!*/m_DAQ_mutex.UnLock();
}

void DAQServ::ResetListResults(){
        ///*!*/m_DAQ_mutex.Lock();
        auto buffer_obj=m_EventQueues.find(m_ASIC_bound);
        if( buffer_obj!=m_EventQueues.end() ){
            dprintf("DAQServ::ResetListResults(): clearing queue for sockID=%d\n",m_ASIC_bound);
            if(buffer_obj->second.aqu_buffer != NULL){
                dprintf("   buffer had %d Entries\n",buffer_obj->second.aqu_buffer->GetSize());
                buffer_obj->second.Clear();
            }
        }else{
            printf("DAQServ::ResetListResults(): reset list request: sockID=%d not registered\n",m_ASIC_bound);
                        //answer.SetWhat(kMESS_NOTOK);
        }
}

TList* DAQServ::FetchListResults(){
        if(m_res!=NULL) delete m_res;
        ///*!*/m_DAQ_mutex.Lock();
        auto buffer_obj=m_EventQueues.find(m_ASIC_bound);
        if( buffer_obj==m_EventQueues.end() || (buffer_obj->second.aqu_buffer==NULL)){
            printf("DAQServ::FetchListResults(): get list for queue #%d: Not installed or empty\n",m_ASIC_bound);
        }else{
            dprintf("DAQServ::FetchListResults(): get list for queue #%d: request\n",m_ASIC_bound);
            dprintf("DAQServ::FetchListResults(): get list for queue #%d: buffer had %d Entries\n",m_ASIC_bound, buffer_obj->second.aqu_buffer->GetSize());
            m_res = (TList*) buffer_obj->second.aqu_buffer;
            buffer_obj->second.Clear();
        }
        ///*!*/m_DAQ_mutex.UnLock();

        return m_res;
}


//Manually append the ASIC list
void DAQServ::AppendASICList(char slave_addr){m_ASICs.push_back(slave_addr); UpdateResultsList();};
//Manually reset the ASIC list
void DAQServ::ResetASICList(){m_ASICs.clear(); UpdateResultsList();};

void DAQServ::ResetResults() {
	std::map<unsigned char, HistogrammedResults >::iterator it=m_hist_results.begin();
        while(it!=m_hist_results.end()){
              it->second.Reset();
              ++it;
        }
}

// Getting current list of ASICs from SC server
int DAQServ::AddASICList(long long unsigned int addr){

	//Test if the IB config could be bound, this should always be the case. Retry until this succeeds
	printf("******* STARTING CONFIG SERVER PROBE ******\n");

	//connect to config server, try binding to handle id 1++ until the connection fails.
	//for succeeded connections, store the I2C address of this ASIC

	m_ASICs.push_back(addr);
	
	printf("*******        ASICS FOUND AT: %d       ******\n", addr);

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

void DAQServ::ReadCECThread(){
	printf("DAQServ::ReadCECThread(): Started\n");
	for(auto it=m_cec_results.begin();it!=m_cec_results.end();it++)
		it->second.Clear();
	m_DAQruncondition=DAQServ::RUNCEC;
	while(m_DAQruncondition==DAQServ::RUNCEC){
		ReadCECCmd();
		usleep(m_DAQ_options.usec_sleep_cec);
	}
	printf("DAQServ::ReadCECThread(): Stopped\n");
}

void DAQServ::ReadChipCmd(int min_chip, int max_tot){
	klaus_acquisition current_aqu=m_iface.ReadEventsUntilEmpty(m_ASICs,min_chip,max_tot);
	current_aqu.acqu_ID=m_current_aqu_ID++;

	//TODO: verify if filling histogram is thread-save
	//Fill HISTO monitor objects
	///*!*/m_DAQ_mutex.Lock();
	std::map<unsigned char, std::list<klaus_event> >::iterator itH;
	for(itH=current_aqu.data.begin();itH!=current_aqu.data.end();++itH){
		//printf("DAQServ::ReadChipCmd(): filling %d events for chip %x\n",itH->first,itH->second.size());
		m_hist_results[itH->first].Fill(itH->second);
	}

	//Fill LIST monitor objects
	ddprintf("DAQServ::ReadChipCmd(): %d queues installed\n",m_EventQueues.size());
	for(auto itLists=m_EventQueues.begin();itLists!=m_EventQueues.end();itLists++){
		ddprintf("DAQServ::ReadChipCmd() Entries: %d : bs=%d ; ps=%d ; nEv=%d\n",itLists->first,itLists->second.bufsize, itLists->second.prescale, itLists->second.aqu_buffer!=NULL?itLists->second.aqu_buffer->GetSize():-1);
		ddprintf("DAQServ::HandleClientRequest(): filling %d events for Queue #%d\n",current_aqu.nEvents,itLists->first);
		itLists->second.Add(current_aqu);
	}
	///*!*/m_DAQ_mutex.UnLock();
}

void DAQServ::ReadCECCmd(){
	klaus_cec_data new_data;
	m_iface.ReadCEC(*m_ASICs.begin(),new_data);
	//new_data.Print();
	///*!*/m_DAQ_mutex.Lock();
	for(auto it=m_cec_results.begin();it!=m_cec_results.end();it++){
		it->second.Add(&new_data);
		//it->second.Print();
	}
	///*!*/m_DAQ_mutex.UnLock();
}

void DAQServ::UpdateResultsList(){
	dprintf("DAQServ::UpdateResultsList()\n");
	m_hist_results.clear();

	for(std::list<unsigned char>::iterator it=m_ASICs.begin();it!=m_ASICs.end();it++){
		//Histogram results for each ASIC, shared by all connected sockets
		m_hist_results[*it].SetPrefix(*it);
	}
}


DAQServ::~DAQServ()
{
   // Clean up

   //m_Sockets->Delete();
   //delete m_Sockets;
   //delete m_Serv;
}



