/*
 * MQInterface.h
 * Message passing to an other process via posix message queues
 *
 *  Created on: March.2013
 *      Author: Konrad
 */

#ifndef MQINTERFACE_H_
#define MQINTERFACE_H_


#include <sys/types.h>
#include <mqueue.h>

#include "VirtualInterface.h"
//#include "mq_message_dict.h"

class TMQInterface: public TVirtualInterface
{

public:
	TMQInterface(const char* qname="sticsc");
	~TMQInterface();



	static TVirtualInterface* Create(const char* qname){
		return new TMQInterface(qname);
	};

private:
	//the message queues for message passing 
	mqd_t queue_cmd;	//configuration --> communication dispatcher
	mqd_t queue_reply;	//v.v.

	virtual int read_Reply(unsigned int ifaceHandID,int reply_len, char* reply, TVirtualConfig* sender=NULL);
	virtual int send_cmd(const char cmd, unsigned int ifaceHandID,  int len, char* data, TVirtualConfig* sender=NULL);
};


#endif /* STIC2INTERFACE_H_ */
