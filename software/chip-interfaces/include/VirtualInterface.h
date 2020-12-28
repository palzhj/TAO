/*
 Beschreibungsklasse fuer Controller - Interface
 rein virtuell, Send(),Receive() je nach typ des interfaces (rs232, fpga, ...)

 die eigentliche Interface - Klasse wird als interface(klaus2board) vererbt.
 */
#ifndef __VIRTUAL_INTERFACE_H
#define __VIRTUAL_INTERFACE_H

#include <map>
#include <stdio.h>
#include <sys/select.h>
//forward declaration to keep track of attached handles
class TVirtualConfig;

class TVirtualInterface
{
protected:
//interface is open and ready for rw-Op
//	bool isReady;

//IDs of configurations (chips) attached to the interface.
// The pointer of the configuration instance is stored for later use in some special interfaces
    std::map<int, TVirtualConfig*> attachedHandles;
    const int nAttable;
    int isReady;

//packetizing - done in implementation
//file descriptor of underlying device
    int fd;

//retries for cmd
    struct timeval tv_timeout;
    int retries;


    virtual int read_Reply(unsigned int ifaceHandID,int reply_len, char* reply,TVirtualConfig* sender=NULL)=0;
    /*read data from the interface.
	returns:
		-2 if reading failed,
		-1 if the reply was not for the given Handle
		0  else.
    */

    virtual int send_cmd(const char cmd, unsigned int ifaceHandID,  int len, char* data, TVirtualConfig* sender=NULL)=0;
    /*send a command to the interface. packetizing&sending should be done in this method
    	returns:
		-2 if writing failed
		0 else
    */


public:

    TVirtualInterface(int nAttacheable=1);
    virtual ~TVirtualInterface();


    static TVirtualInterface* Create()
    {
        return NULL;
    };//  is static function in children, useless here

    //request a new handle ID. If < 0 , the lowest available ID is issued. Returns HandleID, or -1 if requested ID is already in use. 
    virtual int AttachHandle(TVirtualConfig* me,int wish=-1);
    //free handleID. Returns -1 if ID was not in list, handleID otherwise
    virtual int DetachHandle(int ifaceHandID);

    int IsReady(){ return isReady; }
    //get socket id for the internal communication socket.
    int GetFD(){return fd;};


    int SetTimeout(long tv_sec=1,long tv_usec=0, int Nretry=5);

    virtual int Command(unsigned int ifaceHandID, const char cmd,  int len, char* data, int reply_len=-1, char* reply=0, TVirtualConfig* sender=NULL);
    /* send command to the interface.
       if  reply_len < 0, no reply is expected.
       otherwise, the method will wait on the fd (using select() ) 
       and read up to reply_len bytes (using the overloaded read_Reply())
       in case of a timeout on the select, retries will be done until the maximum number of retries is reached

	returns:
		-2 if reading failed,
		-1 if a timeout happened after the maximum number of retries
		0  else.
    */



};

#endif
