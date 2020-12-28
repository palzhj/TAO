/*
	VC remote client base class
	To be inherited by the vc remote client template class.
	Here: Implementation of tcp connection methods, Initial handshake
*/
/*
*/
#ifndef __REMOTE_BASE_H
#define __REMOTE_BASE_H
//#include <cstdlib>
//#include <iostream>


//Template class, CclassT has to  be derived from TVirtualConfig
class VCRemoteClientBase
{
private:

	//members for tcp operations
	int m_sockfd;
	const char* m_host;
	short unsigned int m_port;
	short unsigned int m_ifaceHandID;
	
	//connect if unconnected or try reconnect
	//register client for given handleID
	void Connect();
protected:
	char* m_buffer;
	//Get socket, connect if not connected
	//Invalidate socket, to be called on read/write error
	void TransmissionError();
	//Constructor for the remote client.
	//Parameters:
	// host: the server socket to connect to
	// ifaceHandID: the interfaceHandleID to bind to on the other side
	VCRemoteClientBase(const char* host, unsigned int ifaceHandID, int bufsize);

public:
	//Probe if a host would bind to the given handleID
	static int Probe(const char* host, unsigned int ifaceHandID,bool block_until_connected=true);

	//Returns socket, try to connect if invalid. If retry==true, repeat connecting forever. Returns socket on success, -1 on Connection failure (if retry==false)
	int Socket(bool retry=true);
	~VCRemoteClientBase();

};

#endif
