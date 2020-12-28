/*
 * MQInterface.cpp
 * Message passing to an other process via posix message queues
 *
 *  Created on: March.2013
 *      Author: Konrad
 */


#include <sys/types.h>
#include <mqueue.h>
#include <string.h>
#include <arpa/inet.h>

#include "MQInterface.h"
#include "mq_message_dict.h"



TMQInterface::TMQInterface(const char* qname):
TVirtualInterface(0xff)
{
	struct mq_attr attr;
	attr.mq_maxmsg=10;
	attr.mq_msgsize=sizeof(msg_struct);
	attr.mq_flags=0;


	char name[30];
	sprintf(name,"/%s_cmd.0",qname);
	queue_cmd=	mq_open(name,O_WRONLY,0664,&attr);
	sprintf(name,"/%s_reply.1",qname);
	queue_reply=	mq_open(name,O_RDONLY,0664,&attr);

	if(queue_cmd < 0 || queue_reply<0){
		fprintf(stderr,"MQInterface: Error opening message queues. Is the server running\n");
		isReady=-1;
	}

	fd=queue_reply;
	isReady=1;

}
TMQInterface::~TMQInterface(){
	mq_close(queue_cmd);
	mq_close(queue_reply);
}


/*read data from the interface.
returns:
	-2 if reading failed,
	-1 if the reply was not for the given Handle
	0  else.
*/
int TMQInterface::read_Reply(unsigned int ifaceHandID,int reply_len, char* reply, TVirtualConfig* sender)
{
	int ret;
	//get message
	msg_struct msg_buf;
	ret=mq_receive(queue_reply,(char*)&msg_buf,sizeof(msg_buf),NULL);
	if(ret<0){
		fprintf(stderr,"MQInterface: receive failed\n");
		return -2;
	}
	msg_buf.len=msg_buf.len;
	if ( msg_buf.len < reply_len){
		fprintf(stderr,"MQInterface: received message is shorter than expected.\n");
		return -1;
	}
	if(ifaceHandID != msg_buf.handleID){
		fprintf(stderr,"Packet with wrong handleID ommited (%d!=%d)\n",msg_buf.handleID,ifaceHandID);
		return -1;
	}
	memcpy(reply,msg_buf.msg,reply_len);
	return 0;
}


/*send a command to the interface. packetizing&sending should be done in this method
 * Port can be set differently from 15 which is the default config port
returns:
	-2 if writing failed
	0 else
*/

int TMQInterface::send_cmd(const char cmd, unsigned int ifaceHandID,  int len, char* data, TVirtualConfig* sender){
	int ret;
	msg_struct msg_buf;
	msg_buf.cmd=cmd;
	msg_buf.port=15;	//DEFAULT CONFIGURATION PORT
	msg_buf.handleID=ifaceHandID&0xFF;
	msg_buf.len=len;
	memcpy(msg_buf.msg,data, len);


#ifdef DEBUG
	//printf("Sending packet with len: %d ntohs: %d\n",msg_buf.len, ntohs(msg_buf.len));

	printf("Sending packet: ");
	for (int i=0;i<len+4;i++){
//		if (i%4 == 0) printf("\n");
		printf("%02X ",0xFF&((const char*)(&msg_buf))[i]);
	}
	printf("\n");
#endif


	ret=mq_send(queue_cmd,(const char*)&msg_buf,sizeof(msg_buf),0);
	if(ret<0){
		fprintf(stderr,"MQInterface: send failed\n");
		return -2;
	}
	return 0;
}


