
/*
	Implementation of VC remote client base class
	To be inherited by the vc remote client template class.
	Here: Implementation of tcp connection methods, Initial handshake
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include "VCRemoteClientBase.h"

VCRemoteClientBase::VCRemoteClientBase(const char* host, unsigned int ifaceHandID, int bufsize)
	:
	m_sockfd(-1),
	m_host(host),
	m_port(1234),
	m_ifaceHandID(ifaceHandID)
{
	m_buffer=(char*)malloc(bufsize);
}

VCRemoteClientBase::~VCRemoteClientBase(){
	if(m_sockfd) close(m_sockfd);
	free(m_buffer);
}

int VCRemoteClientBase::Probe(const char* host, unsigned int ifaceHandID,bool block_until_connected){
	VCRemoteClientBase probe(host,ifaceHandID,1024);
	return probe.Socket(block_until_connected);
};

//Try connection or reconnection to host. m_sockfd member is modified, to -1 on connection error
void VCRemoteClientBase::Connect(){
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char initbuf[255];
	if(m_sockfd>=0){
		printf("Closing socket..\n");
		close(m_sockfd);
	}

	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sockfd < 0) {
		perror("ERROR opening socket");
		return;
	}

	server = gethostbyname(m_host);

	if (server == NULL) {
		fprintf(stderr,"ERROR, host %s not found\n",m_host);
		m_sockfd=-1;
		return;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
	 (char *)&serv_addr.sin_addr.s_addr,
	 server->h_length);

	serv_addr.sin_port = htons(m_port);
	if (connect(m_sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		printf("Error opening socket to %s:%d. Port seems to be closed.\n",m_host,m_port);
		close(m_sockfd);
		m_sockfd=-1;
		return;
	}
	
	printf("Connected to host %s  ->  %s on port %d\n",m_host,inet_ntoa(serv_addr.sin_addr),m_port);
	//register client for given handleID
	sprintf(initbuf,"INI:%u:",m_ifaceHandID);
	int ret;
	//printf("Sending initial handshake: %d bytes, \"%s\"\n",strlen(initbuf),initbuf);
	ret=send(m_sockfd,initbuf,strlen(initbuf),0);
	if(ret<strlen(initbuf)){
		printf("Sending initial handshake failed, sent %d\n",ret);
		close(m_sockfd);
		m_sockfd=-1;
		return;
	}
	ret=recv(m_sockfd,initbuf,10,0);
	if(ret<=0){
		printf("Receiving acknowledgement failed, got %d\n",ret);
		close(m_sockfd);
		m_sockfd=-1;
		return;
	}
	printf("Initial Handshake succeeded, bound to handle #%d\n",m_ifaceHandID);
	return;
}


int VCRemoteClientBase::Socket(bool retry){
	//printf("VCRemoteClientBase::Socket():%d\n",m_sockfd);
	if(m_sockfd>=0)
		return m_sockfd;
	else{
		printf("Socket not valid, trying (re)connect\n");
		while(m_sockfd<0){
			Connect();
			if(m_sockfd<0){
				if(retry==true){ printf("Retry enabled, will try again.\n");};
				if(retry) sleep(1);
				else return -1;
			}else return m_sockfd;
		};
	}
}

void VCRemoteClientBase::TransmissionError(){
	printf("VCRemoteClientBase::TransmissionError()\n");
	if(m_sockfd>=0)
	if(m_sockfd) close(m_sockfd);
	m_sockfd=-1;
}
