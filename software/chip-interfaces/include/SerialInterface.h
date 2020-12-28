/*
 * SerialInterface.h
 *
 *  Created on: 29.09.2011
 *      Author: Konrad
 */

#ifndef SerialINTERFACE_H_
#define SerialINTERFACE_H_
#include<termios.h>
#include<fcntl.h>
#include "VirtualInterface.h"


class TSerialInterface: public TVirtualInterface
{
private:
    int fDevice;
    //device status&configuration
    int status;
    struct termios old_tio,options;
    fd_set readfs;


    int read_Reply(unsigned int ifaceHandID,int reply_len, char* reply, TVirtualConfig* sender=NULL);
    /*read data from the interface.
	returns:
		-2 if reading failed,
		-1 if the reply was not for the given Handle
		0  else.
    */

    int send_cmd(const char cmd, unsigned int ifaceHandID, int len, char* data, TVirtualConfig* sender=NULL);

public:
    TSerialInterface(const char* devname=NULL);
    ~TSerialInterface();

    static TVirtualInterface* Create();

    virtual int Command(unsigned int ifaceHandID, const  char cmd, int len, char* data, int reply_len, char* reply,TVirtualConfig* sender=NULL);
};


#endif /* SerialINTERFACE_H_ */
