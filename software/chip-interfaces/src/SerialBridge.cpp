/*
 * klaus2Config.h
 *
 *  Created on: 29.09.2011
 *      Author: Konrad
 */

#include "VirtualConfig.h"
#include "VirtualInterface.h"
#include "SerialBridge.h"
#include "string.h"

TSerialBridge::TSerialBridge(TVirtualInterface* Interface)
:TVirtualConfig(Interface,NULL,0,0){}

int TSerialBridge::Read(char* data, int len){
    return IssueCommand('r',0,0, len, data);
}
int TSerialBridge::Write(char* data, int len){
    char reply_dummy[500];
    return IssueCommand('s',len, data,500,reply_dummy);
}
int TSerialBridge::Reply(int cmd_len, char* cmd, int reply_len,char* reply){
    return IssueCommand('R',cmd_len, cmd, reply_len, reply);
}
int TSerialBridge::Reply(char* cmd, int reply_len,char* reply){
    return IssueCommand('R',strlen(reply),cmd , reply_len, reply);
}


