
#include <stdio.h>
#include "string.h"
#include "VirtualInterface.h"


//#define ECHO_COMMAND

TVirtualInterface::TVirtualInterface(int nAttacheable)
:
nAttable(nAttacheable)
    {
	fd=-1;
        isReady = 0;
	tv_timeout.tv_sec=0;
	tv_timeout.tv_usec=500000;
	retries=2;
    }
    ;

TVirtualInterface::~TVirtualInterface()
    {
        //fprintf(stderr,"VI destruct\n");
        if (attachedHandles.size() != 0)
            fprintf(stderr,
                "Warning: TVirtualInterface::~TVirtualInterface(): There still are configuration instances registered.\n");
    }
    ;

int TVirtualInterface::AttachHandle(TVirtualConfig* me, int wish)
    {
	if (wish<0){
		//find lowest available ID
		wish=0;
		for(wish=0; wish<nAttable; wish++)
			if(attachedHandles.count(wish) == 0){
				attachedHandles[wish]=me;
				fprintf(stderr,"TVirtualInterface::AttachHandle(): Registering handle ID %d\n",wish);
				return wish;
			}
		return -1;
	}else{
		//try to get this ID
		if(attachedHandles.count(wish) == 1)
			return -1;
		attachedHandles[wish]=me;
		fprintf(stderr,"TVirtualInterface::AttachHandle(): Registering handle ID %d\n",wish);
		return wish;
	}
    }
    ;

int TVirtualInterface::DetachHandle(int ifaceHandID)
    {
	if(attachedHandles.count(ifaceHandID) != 1)
			return -1;
	attachedHandles.erase(ifaceHandID);
	fprintf(stderr,"TVirtualInterface::DetachHandle(): Unregistering handle ID %d\n",ifaceHandID);
	return ifaceHandID;
    }
    ;


int TVirtualInterface::SetTimeout(long tv_sec,long tv_usec, int Nretry){
	tv_timeout.tv_sec=tv_sec;
	tv_timeout.tv_usec=tv_usec;
	retries=Nretry;
	return 0;
    };


/*
    virtual int read_Reply(unsigned int ifaceHandID,int reply_len, char* reply)=0;
    virtual int send_cmd(int cmd, unsigned int ifaceHandID,int len, char* data)=0;
*/

int TVirtualInterface::Command(unsigned int ifaceHandID, const char cmd, int len, char* data, int reply_len, char* reply, TVirtualConfig* sender){
	int op_left=retries;
	if(reply_len<0)		//no reply expected
		op_left=-1;

	#ifdef DEBUG
		fprintf(stderr, "TVirtualInterface::Command (type %x ; '%c') called. Len=%d ReplyLen=%d\n",cmd&0xff,cmd,len,reply_len);
	#endif
	#ifdef ECHO_COMMAND
		fprintf(stderr, "TVirtualInterface::Command: Dummy echo reply.\n");
		memcpy(reply, data, reply_len);
		return 0;
	#endif
	// END REMOVE

	struct timeval tv;
	fd_set rfds;
	int retval;

	/*send a command*/
	if (send_cmd(cmd,ifaceHandID,len,data,sender) <0)
		return -2;	//writing command failed

	if (op_left<0)
		return 0;	//no reply

	/*wait for reply*/
	while(1){
		memcpy(&tv, &tv_timeout,sizeof(struct timeval));
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		retval = select(fd+1, &rfds, NULL, NULL, &tv);
		if (retval == -1){
			perror("VirtualInterface::ReplyCmd::select()");
			return -2;


		/*Timout, retry*/
		}else if(retval==0){
			if(op_left < 0){
				fprintf(stderr,"VirtualInterface::Command(): Final Timeout\n");
				return -1;
			}else{
				fprintf(stderr,"VirtualInterface::Command(): Reply Timeout -> Retry (%d)\n",op_left);
				if (send_cmd(cmd,ifaceHandID,len,data,sender) <0)
					return -2;	//writing command failed
			}


		/*got reply*/
		}else if( retval &&(FD_ISSET(fd,&rfds)) ){
    			retval = read_Reply(ifaceHandID,reply_len,reply,sender);
			if(retval>=0){
				return 0;
			}else if (retval==-2)
				return -2;
			//else: read_Reply not for this handle
		}
		op_left--;
	}
	return -1;
}


