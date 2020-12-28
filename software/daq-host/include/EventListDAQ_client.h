//Histogramming DAQ for K4
//Based on ROOT spy.C example
//Author: K.Briggl

#ifndef __EVENTLISTDAQ_CLIENT_H
#define __EVENTLISTDAQ_CLIENT_H
#include <stdarg.h>
#include "DAQctrl_client.h"
//#include "EventType.h"
#include "TList.h"

class EventListDAQ : public DAQctrl{
	private:
		TList* m_res;
	public:
		EventListDAQ(std::string host="");
		~EventListDAQ();
	
		//Commands
		void RegisterQueue(unsigned int buflen, unsigned int prescale=1);
		//Fetch List for the initialized Queue, return the List (Object will be deleted if existing)
		TList* FetchResults();
		//Get previously fetched list of aquisition (might be invalidated by FetchResults())
		TList* GetResults(){return m_res;};

		void ResetResults();
		
	ClassDef(EventListDAQ,1);
};

#endif
