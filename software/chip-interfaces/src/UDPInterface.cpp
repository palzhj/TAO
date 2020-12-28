//STIC2 Interface for configuration
//open udp socket for communication with fpga
//after RWOperation, send magic_char+handle_id packet to fpga.
//wait for magic+1 + pattern packet
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../include/UDPInterface.h"
#include "mq_message_dict.UDP.h"

//#define DEBUG

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



TUDPInterface::TUDPInterface(const char* host, const char* port):
TVirtualInterface(10)
{

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    int rv;
    if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("config: socket");
            continue;
        }

        if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(fd);
            perror("config: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "config: failed to prepare socket\n");
        return;
    }
	freeaddrinfo(servinfo);
    isReady=1;
}

TUDPInterface::~TUDPInterface(){
	close(fd);
};



int TUDPInterface::send_cmd(const char cmd, unsigned int ifaceHandID,  int len, char* data, TVirtualConfig* sender){
	int numbytes;
	udp_msg_struct msg_buf;
	msg_buf.cmd=cmd;
	msg_buf.handleID=ifaceHandID&0xFF;
	msg_buf.len=htons(len);
	memcpy(msg_buf.msg,data, len);

#ifdef DEBUG
	printf("Sending packet:");
	for (int i=0;i<ntohs(msg_buf.len);i++)
		printf(" %.1x ",0xff&((const char*)(msg_buf.msg))[i]);
	printf("\n");
#endif
	if ((numbytes = write(fd,(const char*) &msg_buf,sizeof(msg_buf)) == -1)) {
        	perror("iface: sendto");
		return -2;
        }

	return 0;
}




    /*read data from the interface.
	returns:
		-2 if reading failed,
		-1 if the reply was not for the given Handle
		0  else.
    */
int TUDPInterface::read_Reply(unsigned int ifaceHandID,int reply_len, char* reply, TVirtualConfig* sender){
	int numbytes;
	udp_msg_struct msg_buf;
	struct sockaddr_storage their_addr;
	socklen_t addr_len = sizeof their_addr;

	if ((numbytes = recvfrom(fd,(char*) &msg_buf, reply_len + sizeof(udp_msg_header), 0,
        	(struct sockaddr *)&their_addr, &addr_len)) == -1) {
        	perror("recvfrom");
		return -2;
	}
	msg_buf.len=ntohs(msg_buf.len);
#ifdef DEBUG
	//printf("iface: got packet from %s\n",
        //inet_ntop(their_addr.ss_family,
        //    get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
    	printf("iface: packet is %d bytes long\n", numbytes);
	for (int i=0; i<numbytes; i++){
		printf("%.2x ",msg_buf.msg[i]&0xff);
	}
	printf("\n");
	printf("cmd:  '%c'\n",msg_buf.cmd);
	printf("hand: '%d'\n",msg_buf.handleID);
	printf("len:  '%d'\n",msg_buf.len);
	printf("raw:");
	for (int i=0; i<35; i++){
		printf("%.2x ",((char*) &msg_buf)[i] & 0xff);		
	}
	printf("\n");


#endif

	if (reply!=NULL)
		memcpy(reply,msg_buf.msg,reply_len);

	return numbytes -  2;
}



