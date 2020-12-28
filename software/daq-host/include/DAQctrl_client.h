//Histogramming DAQ for K4
//Based on ROOT spy.C example
//Author: K.Briggl

#ifndef __DAQCTRL_CLIENT_H
#define __DAQCTRL_CLIENT_H
#include <stdarg.h>
#include "TSocket.h"
#ifndef __CINT__
#include "TError.h"
#endif
#include <list>
#include "TMutex.h"
class DAQctrl {
	private:
		TSocket *m_Socket;      // client socket
		std::list<unsigned char> m_ASICs;
		std::string m_hostname;
		TMutex m_MutexSock; 
	protected:


		//Special communication commands
		std::list<unsigned char> FetchListOfASICs();
		void CommandWithAck(std::string cmdstring, ...);
		int CommandRepliesOpCode(std::string cmdstring, ...);
		TObject* CommandRepliesObject(const TClass* target_obj, std::string cmdstring, ...);

	public:
		DAQctrl(std::string host="");
		~DAQctrl();
		void Connect(std::string host);
		bool Good(){ if ((m_Socket==NULL) || (!m_Socket->IsValid())) return false; else return true;}
		std::string GetHostname(){return m_hostname;};
		//Commands
		std::list<unsigned char> GetListOfASICs(){return m_ASICs;};

		void FlushFIFO(int minEvents=0);
		void ReadChipUntilEmpty(int minEvents=0,int maxEvents=-1);
		void ReadChipAsyncStart(int usec_sleep=0, int minEvents=0,int maxEvents=-1);
		void ReadChipAsyncStop();
	ClassDef(DAQctrl,1);
};
#endif
