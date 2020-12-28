/*
 * SerialInterface.cpp
 *
 *  Created on: 29.09.2011
 *      Author: Konrad
 */


#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<iostream>


#include "../include/SerialInterface.h"

using namespace std;

//#define DEBUG

TSerialInterface::TSerialInterface(const char *devname):TVirtualInterface(1)
{
    char deviceName[255];


    if (!devname)
    	strcpy(deviceName,"/dev/ttyUSB1");
    else 
	strcpy(deviceName,devname);


    //try to open device


//    printf("Trying to open device \"%s\"\n",deviceName);
    fd = open(deviceName,O_RDWR | O_NOCTTY,0) ;
    if(fd<0)
    {
        perror("Open");
        return;
    }
    isReady=1;

//    fcntl(fd, F_SETFL,0);//FNDELAY);

    tcgetattr(fd,&old_tio);
    tcgetattr(fd,&options);

    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
 //   cfsetispeed(&options, B9600);
 //   cfsetospeed(&options, B9600);

    options.c_cflag |= (CLOCAL | CREAD);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;

    options.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON | IXOFF);
    options.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL | OLCUC | OPOST);
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    tcsetattr(fd,TCSAFLUSH,&options);


    FD_ZERO(&readfs);
    FD_SET(fd,&readfs);
    return;
}



TSerialInterface::~TSerialInterface()
{
    fprintf(stderr,"SerialI destruct\n");
    close(fd);
}

TVirtualInterface* TSerialInterface::Create()
{
    return new TSerialInterface();
}


int TSerialInterface::send_cmd(const char cmd, unsigned int ifaceHandID, int len, char* data, TVirtualConfig* sender){
    int status;
    status=write(fd,data,len);
    if(status==-1)
    {
        fprintf(stderr,"Error writing to Serial device.\n");
        return -2;
    }

    return status;
}
int TSerialInterface::read_Reply(unsigned int ifaceHandID,int reply_len, char* reply, TVirtualConfig* sender){
    int status;
    usleep(100000);
    status=read(fd,reply,reply_len);
    if (status<reply_len)
    {
        fprintf(stderr,"Error reading all bytes from Device. Status=%d\n",status);
        return -2;
	perror("Read:");
    }
    return status;
}


int TSerialInterface::Command(unsigned int ifaceHandID, const char cmd, int len, char* data, int reply_len, char* reply,TVirtualConfig* sender)
{
    if(cmd=='c'){
	    char c = '1';
	    send_cmd(0,ifaceHandID,1,&c);
	    usleep(10000);
	    send_cmd(0,ifaceHandID,len,data);
	    usleep(10000);
	    c='S';
	    send_cmd(0,ifaceHandID,1,&c);
	    usleep(100000);
	    c='R';
	    return TVirtualInterface::Command(0,ifaceHandID,1,&c,reply_len,reply);
    }else{
	TVirtualInterface::Command(cmd,ifaceHandID,len,data,reply_len,reply);
    }
    return 0;
}

