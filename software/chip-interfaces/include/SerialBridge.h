/*
 * klaus2Config.h
 *
 *  Created on: 29.09.2011
 *      Author: Konrad
 */
#ifndef SERIALBRIDGE_H__
#define SERIALBRIDGE_H__


#include "VirtualConfig.h"
#include "VirtualInterface.h"



class TSerialBridge: private TVirtualConfig
{
public:
    TSerialBridge(TVirtualInterface* Interface);
    ~TSerialBridge(){};
	
    int Read(char* data, int len);
    int Write(char* data, int len);
    int Reply(char* cmd, int reply_len,char* reply);
    int Reply(int cmd_len, char* cmd, int reply_len,char* reply);
};

#endif
