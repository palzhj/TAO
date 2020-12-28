//Histogrammed results collection for K4: Manage histograms filled from klaus_events
//Author: K.Briggl

#ifndef __HISTOGRAMDAQ_SERVER_H
#define __HISTOGRAMDAQ_SERVER_H

#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TList.h"
#ifndef __CINT__
#include "TError.h"
#endif

#include "TThread.h"
#include "TMutex.h"

#include "EventType.h"
#include "klaus_i2c_iface.h"

#include "HistogrammedResults.h"
struct events_buffer_type;

class DAQServ {
private:
//Run conditions
	enum runcondition {RUN,SUSPEND,EXIT} m_SERVruncondition, m_DAQruncondition;
	//thread object and status
	TThread* m_DAQ_thread;
	struct{
		int min_chip;
		int max_tot;
		int usec_sleep;
	} m_DAQ_options;
	TMutex m_DAQ_mutex;

//ASIC interface
	klaus_i2c_iface& m_iface;
	std::list<unsigned char> m_ASICs;
//Parsed raw event data per aquisition (all ASICs in the object)
	unsigned long m_current_aqu_ID;
	
//DAQ results
	//ASIC list changed, update storage
	void UpdateResultsList();
	//storage of histograms for each ASIC, shared by all connected sockets
	std::map<unsigned char,HistogrammedResults> m_hist_results;

	std::map<int,events_buffer_type> m_EventQueues;

//TCP connection
	TServerSocket *m_Serv;      // server socket
	TMonitor      *m_Mon;       // socket monitor
	TList         *m_Sockets;   // list of open spy sockets


public:
   DAQServ(klaus_i2c_iface& iface);
   ~DAQServ();
//Control the main Loop: Wait for requests from clients, handle sequentially in the calling thread
// Run() sets runcondition to 'RUN' -> Thread listening
// Suspend()	| Callable from other thread | sets runcondition to 'RUN' or 'SUSPEND' -> Thread listening or idle
// Stop()	| Callable from other thread | will issue the Run() method to return and set the runcondition to STOP
   void Run();
   void Suspend(bool status);
   void Stop();

//Get list of accessible ASICs (I2C addresses) from slow-control server
//The resulting list is stored in m_ASICs, which is reset on call. The number of ASICs reteived from the Server is returned
//Note: the function will not try to reconnect if the 
   int  AutoFetchASICList(const char* host);
//Manually append the ASIC list
   void AppendASICList(char slave_addr);
//Manually reset the ASIC list
   void ResetASICList();
//Get copy of ASIC list
   std::list<unsigned char> GetASICList(){return m_ASICs;};
   static void* ReadChipThreadStart(void* classptr){((DAQServ*)classptr)->ReadChipThread();};
protected:
//Client request & connection handling
   void HandleSocket(TSocket *s);
   void HandleClientRequest(TSocket* s, char* request);
//Readchip command
   void ReadChipThread();
   void ReadChipCmd(int min_chip=0, int max_tot=-1);
};



#endif
