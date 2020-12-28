/*
 File: ChainedSPIinterface.h
 Class definition of a bit pattern merger for a chain configuration of several ASICs connected to the same SPI interface
 The connection scheme typically looks as follows:
   The CS & SCK are connected to all chips in the chain
   The Data lines are connected in a chain:
   SDO(FPGA) --> SDI(CHIP0) | SDO(CHIP0) --> SDI(CHIP1) .... --> SDI(FPGA)
 For this scheme to work with the configuration classes with compatibility to the normal interface/config class structure (without the need for writing specific patterns for this),
 this pattern merger is implemented. It connects several configuration instances to one interface using one (physical, that is, sent over to the interface) handle ID.
 Inheritance: public VirtualInterface, private VirtualConfig. The first implements connection to a set of VirtualConfig instances for the chain, the second implements connection to the Interface instance.
 Suppose you instanciate a number of configuration instances
 simple example:
   VirtualInterface* phys_if=new TXYInterface(...); // instanciate the interface for transmission
   VirtualConfig* config_nochain=new TXYConfig(phys_if); //connection of a "normal" handle without chaining --> handleID=0
   TBitPatternMerger* chainA=new TBitPatternMerger(phys_if) //set up the merger and bind it to the physical interface -> handleID=1
   VirtualConfig* config_chain[N]; //set up a number of configurations in the chain ...
   config_chain[i]=new XYConfig(chainA); // ... and bind them to the merger. The handleID for the chained configurations have to be considered 'virtual', as they are not actually sent physically.

   //issue configuration of the full chain by either
   config_chain[i]->UpdateConfig(); // passed down to the merger, or
   chainA->Update(); // directly

 In both configuration cases, the patterns from each of the configuration instances connected to the merger are read and merged together,
 starting with the configuration class with lowest handleID. Then, the merged pattern is sent over the physical interface

 */
#ifndef __PATTERN_MERGER_H
#define __PATTERN_MERGER_H

#include <stdio.h>
#include <sys/select.h>
#include "VirtualConfig.h"
#include "VirtualInterface.h"
#include <list>

class TPatternMerger : public TVirtualInterface, protected TVirtualConfig {
	protected:
		unsigned int raw_pattern_byte_length;
		unsigned int raw_pattern_bit_length;
		void MergePattern();
	public:
		TPatternMerger(TVirtualInterface* Interface);
		~TPatternMerger();

	//make some functions inherited from base classes public again
	//Overloaded VirtualConfig members
    	int UpdateConfig();

		/*
		Issue reconfiguration of the chain by calling Interface->Command(...).
		Possible returns:
		1:  all good
		0:  command ok, but returned bitpattern not correct
		-1: communication timeout
		-2: communication error

		*/
		virtual int ValidatePattern();

	//Overloaded VirtualInterface members
		virtual int AttachHandle(TVirtualConfig* me,int wish=-1);
		virtual int DetachHandle(int ifaceHandID);
		virtual int Command(unsigned int ifaceHandID, const char cmd,  int len, char* data, int reply_len=-1, char* reply=0, TVirtualConfig* sender=NULL);

		virtual short int GetPatternBitLength()const {return raw_pattern_bit_length;}; //get length of pattern in bits
		virtual short int GetPatternByteLength()const {	//get length of pattern in bytes
			return raw_pattern_bit_length/8 + (raw_pattern_bit_length%8!=0?1:0);
		};
	protected:
		virtual int read_Reply(unsigned int ifaceHandID,int reply_len, char* reply, TVirtualConfig* sender=NULL){};
		virtual int send_cmd(const char cmd, unsigned int ifaceHandID,  int len, char* data, TVirtualConfig* sender=NULL){};
};
#endif
