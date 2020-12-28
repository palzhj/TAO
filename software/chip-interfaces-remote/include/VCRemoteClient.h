/*
	VC remote client class
	Template class inherited from any TVirtualConfig implementation
	The class will overload all setters and getters to communicate directly with the VC remote server object somewhere else
	Based on libasio, without direct boost binding
*/
/*
	INTERNALS:
	Method request is sent as | METHOD_ID | ...
	Return is | METHOD_ID | RETURNVAL | ...
	The requests are serialized using stream, as text
	METHOD_ID is an integer 
*/
#ifndef __VC_REMOTE_H
#define __VC_REMOTE_H
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include "VirtualInterface.h"
#include "VCRemoteClientBase.h"
//Template class, CclassT has to  be derived from TVirtualConfig
template<class CclassT>

class VCRemoteClient : public CclassT, public VCRemoteClientBase {
private:


//bitpattern access
	void GetFromPattern(char * const pattern , int N,unsigned long long& value) const{
		printf("VCRemoteClient::GetInPattern() should never be used in this implementation!\n");
	};
	void SetInPattern(char * const pattern , int N,unsigned long long const& value) const{
		printf("VCRemoteClient::GetInPattern() should never be used in this implementation!\n");
	};
public:
    //Constructor for the remote client.
   //Parameters:
    // io_service: the asio service object
    // endpoint_iterator: the ip endpoint to use
    // ifaceHandID: the interfaceHandleID to bind to on the other side
VCRemoteClient(const char* host, unsigned int ifaceHandID)
	: CclassT(NULL),
	VCRemoteClientBase(host,ifaceHandID,1024)
	{};


    ~VCRemoteClient()
{

}


//Methods overloaded from TVirtualConfig
///////////////////////////


	/*
	Possible returns:
	0:  all good
	-1: communication timeout
	-2: communication error
	<=-10:  command ok, but returned bitpattern not correct
	*/
	int UpdateConfig()
	{
		int clen;
		int ret;
		//std::cout<<"VCRemoteClient::UpdateConfig()"<<std::endl;
		sprintf(m_buffer,"UPD:");
		clen=strlen(m_buffer);
		int sockfd=Socket();
		//send
		ret=send(sockfd,m_buffer,clen,0);
		if(ret<clen){
			TransmissionError();
			return -1;
		}
		//get reply
		ret=recv(sockfd,m_buffer,512,0);
		//handle reply
		if(ret>0){
			m_buffer[ret]='\0';
			if(sscanf(m_buffer,"UPD:%d:",&ret)==1){
				return ret;
			}
		}
		//else: error
		TransmissionError();
		return -1;
	};

	/*
	check bitpattern: send and received the same?
	0:  all good
	<=-10:  returned bitpattern not correct: - 10*{NUMBER_OF_WRONG_BYTES}
	*/
	virtual int ValidatePattern()
	{
		int clen;
		int ret;
		int sockfd=Socket();
		//std::cout<<"VCRemoteClient::ValidatePattern()"<<std::endl;
		sprintf(m_buffer,"VAL:");
		clen=strlen(m_buffer);
		//send
		ret=send(sockfd,m_buffer,clen,0);
		if(ret<clen){
			TransmissionError();
			return -1;
		}
		//get reply
		ret=recv(sockfd,m_buffer,512,0);
		//handle reply
		if(ret>0){
			m_buffer[ret]='\0';
			if(sscanf(m_buffer,"VAL:%d:",&ret)==1){
				return ret;
			}
		}
		//else: error
		TransmissionError();
		return -1;
	};


	/*
	Connection to interface command method, to be used by derived classes to access the interface
	*/
	virtual int IssueCommand(const char cmd,  int len, char* data, int reply_len=-1, char* reply=0)
	{
		int ret;
		int clen;
		int sockfd=Socket();
		//std::cout<<"VCRemoteClient::IssueCommand()"<<std::endl;
		sprintf(m_buffer,"CMD:%c:%d:%d:",cmd,len,reply_len);
		clen=strlen(m_buffer);
		memcpy(&m_buffer[clen],data,len);
		clen+=len;
		m_buffer[clen]=':';
		clen++;

		//send
		ret=send(sockfd,m_buffer,clen,0);
		if(ret<len){
			TransmissionError();
			return -1;
		}
		//no reply, no acknowledge needed
		if(reply_len<0){
			return 0;
		}
		//get ack&reply
		clen=strlen("CMD:")+reply_len+strlen(":");
		ret=recv(sockfd,m_buffer,clen,0);
		//handle reply
		if(ret!=clen){
			TransmissionError();
			return -1;
		}
		//check header
		if(strncmp("CMD:",m_buffer,4)!=0){
			TransmissionError();
			return -1;
		}
		if(m_buffer[ret-1]!=':'){
			TransmissionError();
			return -1;
		}
		//everything fine, copy reply
		memcpy(reply,&m_buffer[strlen("CMD:")],reply_len);
		return 0;
	}

//Pattern Access
///////////////////////////
    virtual char* GetPatternRD() const{printf("VCRemoteClient::GetPatternRD() should never be used in this implementation!\n");return NULL;};
    virtual char* GetPatternWR() const{printf("VCRemoteClient::GetPatternWR() should never be used in this implementation!\n");return NULL;};

//Parameter finding [No overload needed, this is done locally]
///////////////////////////
	using TVirtualConfig::GetNParameters;
//Parameter Setters+Getters
///////////////////////////
//[Low level functions accessing the bit pattern are overloaded, other methods calling these are not]


	/** @brief Get Parameter (read)
	* Return: number of parameter, -1 if not in range/found
	*/
	using TVirtualConfig::GetParValueRD;
	virtual int GetParValueRD(short int npar,unsigned long long &value)
	{
		if ( (npar<0) | (npar>=TVirtualConfig::GetNParameters()) )
			return -1;

		int clen;
		int ret;
		int sockfd=Socket();
		//std::cout<<"VCRemoteClient::GetParValueRD()"<<std::endl;
		sprintf(m_buffer,"GRD:%d:",npar);
		clen=strlen(m_buffer);
		//send
		ret=send(sockfd,m_buffer,clen,0);
		if(ret<clen){
			TransmissionError();
			return -1;
		}
		//get reply
		ret=recv(sockfd,m_buffer,512,0);
		//handle reply
		if(ret>0){
			m_buffer[ret]='\0';
			if(sscanf(m_buffer,"GRD:%llu:",&value)==1){
				return npar;
			}
		}
		//else: error
		TransmissionError();
		return -1;
	}

	/** @brief Get Parameter (to be written)
	* Return: number of parameter, -1 if not in range/found
	*/
	using TVirtualConfig::GetParValueWR;
	virtual int GetParValueWR(short int npar,unsigned long long &value)
	{
		if ( (npar<0) | (npar>=GetNParameters()) )
			return -1;

		int clen;
		int ret;
		int sockfd=Socket();
		//std::cout<<"VCRemoteClient::GetParValueWR()"<<std::endl;
		sprintf(m_buffer,"GWR:%d:",npar);
		clen=strlen(m_buffer);
		//send
		ret=send(sockfd,m_buffer,clen,0);
		if(ret<clen){
			TransmissionError();
			return -1;
		}
		//get reply
		ret=recv(sockfd,m_buffer,512,0);
		//handle reply
		if(ret>0){
			m_buffer[ret]='\0';
			if(sscanf(m_buffer,"GWR:%llu:",&value)==1){
				return npar;
			}
		}
		//else: error
		TransmissionError();
		return -1;
	};


	/** @brief Set Parameter (to be written)
	* Return: number of parameter, -1 if not in range/found
	*/
	using TVirtualConfig::SetParValue;
	virtual int SetParValue(short int npar,unsigned long long value)
	{
		if ( (npar<0) | (npar>=GetNParameters()) )
			return -1;

		int clen;
		int ret;
		int sockfd=Socket();
		//std::cout<<"VCRemoteClient::SetParValue()"<<std::endl;
		sprintf(m_buffer,"SET:%d:%lld:",npar,value);
		clen=strlen(m_buffer);
		//send
		ret=send(sockfd,m_buffer,clen,0);
		if(ret<clen){
			TransmissionError();
			return -1;
		}
		//no reply here to speed things up. We expect the user to do the right thing ... :)
		return npar;
	};

};

#endif
