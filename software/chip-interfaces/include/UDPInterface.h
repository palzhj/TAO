/*
 * UDPInterface.h
 *
 *  Created on: June.2012
 *      Author: Konrad
 */

#ifndef UDPINTERFACE_H_
#define UDPINTERFACE_H_


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include "VirtualInterface.h"


class TUDPInterface: public TVirtualInterface
{

public:
    TUDPInterface(const char* host, const char* port="5002");
    ~TUDPInterface();



    static TVirtualInterface* Create(const char* host, const char* port="5002"){
	return new TUDPInterface(host,port);
    };

/*  removed
    virtual void ChipReset(int ifaceHandID);
    virtual void ChannelReset(int ifaceHandID);
*/
private:
    struct addrinfo hints, *servinfo, *p;


    virtual int read_Reply(unsigned int ifaceHandID,int reply_len, char* reply, TVirtualConfig* sender=NULL);
    virtual int send_cmd(const char cmd, unsigned int ifaceHandID, int len, char* data, TVirtualConfig* sender=NULL);
};


#endif /* STIC2INTERFACE_H_ */
