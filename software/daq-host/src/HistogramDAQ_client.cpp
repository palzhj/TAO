//Histogramming DAQ for K4
//Based on ROOT spy.C example
//Author: K.Briggl


//#include "EventType.h"
#include "HistogrammedResults.h"
#include "HistogramDAQ_client.h"
#include "TMessage.h"
#include "TClass.h"

HistogramDAQ::HistogramDAQ(std::string host):
	DAQctrl(host),
	m_results(NULL),
	m_ASIC_bound(-1)
	{}


HistogramDAQ::~HistogramDAQ(){
	if(m_results!=NULL) delete m_results;
}

void HistogramDAQ::UpdateParams_12b(int channel, int branch, int branchDis){
	this->CommandWithAck("pipe_param %d %d %d %d",m_ASIC_bound,channel,branch,branchDis);
}

void HistogramDAQ::ResetResults(){
	this->CommandWithAck("reset histos");
}	


void HistogramDAQ::FetchResults(){
	HistogrammedResults* res=(HistogrammedResults*)CommandRepliesObject(HistogrammedResults::Class(),"get histos %d", m_ASIC_bound);
	if(res!=NULL){
		if(m_results!=NULL) delete m_results;
		m_results=res;
	}
}

ClassImp(HistogramDAQ);

