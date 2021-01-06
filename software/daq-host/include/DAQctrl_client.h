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
#include "EventType.h"
class DAQctrl {
	private:
		TSocket *m_Socket;      // client socket
		std::list<unsigned char> m_ASICs;
		std::string m_hostname;
		unsigned short m_port;
		TMutex m_MutexSock; 
	public:
		//Special communication commands
		std::list<unsigned char> FetchListOfASICs();
		void CommandWithAck(std::string cmdstring, ...);
		int CommandRepliesOpCode(std::string cmdstring, ...);
		TObject* CommandRepliesObject(const TClass* target_obj, std::string cmdstring, ...);
	public:
		DAQctrl(std::string host, unsigned short port);//=9090);
		~DAQctrl();
		void Connect(std::string host, unsigned short port);//=9090);
		//bool Good(){ if ((m_Socket<=NULL) || (!m_Socket->IsValid())) return false; else return true;}
		bool Good(){ if ((m_Socket==NULL) || (!m_Socket->IsValid())) return false; else return true;}
		std::string GetHostname(){return m_hostname;};
		//Commands
		std::list<unsigned char> GetListOfASICs(){return m_ASICs;};

		void FlushFIFO(int minEvents=0);
		void ReadChipUntilEmpty(int minEvents=0,int maxEvents=-1);
		void ReadChipAsyncStart(int usec_sleep=0, int minEvents=0,int maxEvents=-1);
		void ReadCECAsyncStart(int usec_sleep=0);
		void ReadChipAsyncStop();

		void ResetCEC();
		klaus_cec_data* FetchCEC();

	ClassDef(DAQctrl,1);
};
#endif
