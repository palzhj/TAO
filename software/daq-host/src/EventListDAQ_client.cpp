//Get List of aquisitions from DAQ-board
//Author: K.Briggl


#include "EventType.h"
#include "EventListDAQ_client.h"
#include "TList.h"
EventListDAQ::EventListDAQ(std::string host):
	DAQctrl(host),
	m_res(NULL)
	{}


EventListDAQ::~EventListDAQ(){
}

void EventListDAQ::RegisterQueue(unsigned int buflen, unsigned int prescale){
	this->CommandWithAck("register list %u %u",buflen,prescale);
}

void EventListDAQ::ResetResults(){
	this->CommandWithAck("reset list");
}	


TList* EventListDAQ::FetchResults(){
	if(m_res!=NULL) delete m_res;
	m_res=(TList*)CommandRepliesObject(TList::Class(),"get list");
	if(m_res!=NULL){
		std::cout<<"EventListDAQ::FetchResults(): Got list of size "<<m_res->GetSize()<<std::endl;
	}
	else
		std::cout<<"EventListDAQ::FetchResults(): No list returned, forgot to register?"<<std::endl;
	return m_res;
}

ClassImp(EventListDAQ);

