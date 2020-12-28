
#include <stdio.h>
#include "string.h"
#include "PatternMerger.h"


//#define DEBUG


//void DbgPrintPatternHex(const char* prefix, char* data,int len,bool inv=false);
/*{
	printf("%s",prefix);
	for (int i=len-1;i>=0;i--)
		printf(" %2.2x ",0xff&((const char*)(data))[inv?len-i-1:i]);
	printf("\n");
}
*/
TPatternMerger::TPatternMerger(TVirtualInterface* Interface):
	TVirtualInterface(255),
	TVirtualConfig(Interface,NULL,0,0)
{
	raw_pattern_bit_length =0;
	isReady = 1;
};

TPatternMerger::~TPatternMerger(){
};


int TPatternMerger::AttachHandle(TVirtualConfig* me,int wish){
	//call base class method
	int ret=TVirtualInterface::AttachHandle(me,wish);
	if (ret == -1) return -1;
	//update length of pattern
	raw_pattern_bit_length+=me->GetPatternBitLength();
	bitpattern_read=(char*)realloc(bitpattern_read,this->GetPatternByteLength());
	bitpattern_write=(char*)realloc(bitpattern_write,this->GetPatternByteLength());
#ifdef DEBUG
	fprintf(stderr,"TPatternMerger::AttachHandle(): Length is now %d\n",this->GetPatternByteLength());
#endif
	return ret;
}

int TPatternMerger::DetachHandle(int ifaceHandID){
	//call base class method

	int length_to_detach_byte = attachedHandles[ifaceHandID]->GetPatternByteLength();	// length needs to be aquired bevore detaching, and needs to be updated, when succesfully detached
	int length_to_detach_bit = attachedHandles[ifaceHandID]->GetPatternBitLength();

	int ret=TVirtualInterface::DetachHandle(ifaceHandID);
	if (ret == -1) return -1;

	raw_pattern_bit_length-=length_to_detach_bit;		// update the length here.
	bitpattern_read=(char*)realloc(bitpattern_read,this->GetPatternByteLength());
	bitpattern_write=(char*)realloc(bitpattern_write,this->GetPatternByteLength());
#ifdef DEBUG
	fprintf(stderr,"TPatternMerger::DetachHandle(): Length is now %d\n",this->GetPatternByteLength());
#endif
	return ret;
}


int TPatternMerger::ValidatePattern(){
	//validate returned pattern for each Handle
	int status=0;
	for(int i = 0; i< attachedHandles.size(); i++)
		status+=attachedHandles[i]->ValidatePattern();
	return status;
}


//Update configuration command of pattern merger: Calls Command function of this class to merge the patterns of the connected classes into one.
//That function will call the Command() function of the underlying physical interface.
int TPatternMerger::UpdateConfig(){
#ifdef DEBUG
	fprintf(stderr, "TPatternMerger::UpdateConfig() called\n");
#endif
	//issue merging of pattern and configuration
	int status=this->Command(this->ifaceHandID, 'c',0,0);
	if (status<0)
		return status;

	return ValidatePattern();
};


int TPatternMerger::Command(unsigned int ifaceHandID, const char cmd, int len, char* data, int reply_len, char* reply, TVirtualConfig* sender){
#ifdef DEBUG
	fprintf(stderr, "TPatternMerger::Command() (type %x ; '%c') called\n",cmd&0xff,cmd);
#endif

	if (cmd!='c'){
		//this is not a configuration command, just feed through to the physical interface
		return IssueCommand(cmd, len, data,reply_len,reply);
	}



#ifdef DEBUG
	fprintf(stderr, "TPatternMerger::Command(): Will merge pattern of %d handles\n", attachedHandles.size());
#endif

	//merge patterns from children
	int offset = 0;
	bzero(bitpattern_write, this->GetPatternByteLength());
	for(int i = 0; i< attachedHandles.size(); i++){
	#ifdef DEBUG
		DbgPrintPatternHex("TI:", attachedHandles[i]->GetPatternWR(),attachedHandles[i]->GetPatternByteLength());
	#endif
		//bitwise copy of patterns
		for(int j=0;j<attachedHandles[i]->GetPatternBitLength();j++){
			bool bit=attachedHandles[i]->GetPatternWR()[j/8]&(1<<(j%8));
			bitpattern_write[offset/8]|=bit<<offset%8;
			offset++;
		}
	}
#ifdef DEBUG
	DbgPrintPatternHex("TM:", bitpattern_write,this->GetPatternByteLength());
#endif

	//printf("len: %i\n", this->GetPatternByteLength());
	//write command to physical interface
	int status = IssueCommand( 'c', this->GetPatternByteLength(), bitpattern_write,  this->GetPatternByteLength(), bitpattern_read);
	if (status<0)
		return status;

	#ifdef DEBUG
	DbgPrintPatternHex("RM:", bitpattern_read,this->GetPatternByteLength());
	#endif

	//decompose pattern to child configuration classes
	offset = 0;
	for(int i = 0; i< attachedHandles.size(); i++){
		//bitwise copy of patterns
		bzero(attachedHandles[i]->GetPatternRD(),attachedHandles[i]->GetPatternByteLength());
		for(int j=0;j<attachedHandles[i]->GetPatternBitLength();j++){
			bool bit=bitpattern_read[offset/8]&(1<<offset%8);
			attachedHandles[i]->GetPatternRD()[j/8]|=bit<<j%8;
			offset++;
		}
		#ifdef DEBUG
		DbgPrintPatternHex("RO:", attachedHandles[i]->GetPatternRD(),attachedHandles[i]->GetPatternByteLength());
		#endif
	}

	return 0;
}

