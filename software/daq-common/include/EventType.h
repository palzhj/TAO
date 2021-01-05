#ifndef EVENT_TYPE_H__
#define EVENT_TYPE_H__

#include "TObject.h"
#include <fstream>
#include <iostream>
#include <map>
#include <list>

//#ifndef __CINT__
#include <sys/time.h>
//#endif
#include "klaus_constants.h"

////////////////////////////////////////////////////////////////
///                 KLauS EVENT TYPES
///////////////////////////////////////////////////////////////

//Single event type for KLauS ASIC
class klaus_event: public TObject
{
	public:
		klaus_event(){}
		klaus_event(unsigned char* buffer, unsigned short blockID){acq_blk=blockID; Parse(buffer);}

                // Basic information  of a event given by KLauS ASIC
		bool		gainsel_evt;
		unsigned short	ADC_10b;
		unsigned short	ADC_6b;
		unsigned short	ADC_PIPE;
		unsigned long	T_CC;
                unsigned short  T_MC;
                unsigned short  T_FC;
		unsigned char	channelID;
		unsigned char	groupID;
		unsigned char	channel;
                unsigned short  badhit;
                
                // Parse the data stream received through the I2C interface
		void		Parse(unsigned char* event);

                // Print 
		static void	PrintHeader(FILE* fd=stdout);
		void		Print(FILE* fd=stdout);

                // transfer the time from Gray to decimal, and get the time interval wrt the last event 
		unsigned long	GetTime() const;
		int 		DiffTime(klaus_event& evt) const;

		//DAQ fields
		unsigned short	acq_blk; //Aquisition block cycle

		//time correction
		static int foffset;
		static int moffset;
        ClassDef(klaus_event,2);
};

//Collection of klaus events with the same Aquisition number for different ASICs
//This data type is intended to be stored or transmitted!
class klaus_acquisition: public TObject{
        public:
        	klaus_acquisition();

                // the ASIC ID and all the events in the same acquisition ID for this ASIC are paired
        	std::map<unsigned char, std::list<klaus_event> > data;
        	unsigned long acqu_ID;   // a unique acquisition ID for the klaus_aquisition type
        
        	unsigned short nEvents;
        	unsigned short failcnt;
        	void	Print(FILE* fd=stdout);


        ClassDef(klaus_acquisition,1);
};


////////////////////////////////////////////////////////////////
///                 KLauS CEC TYPES
///////////////////////////////////////////////////////////////
class klaus_cec_data: public TObject
{
	public:
		klaus_cec_data(){}

		unsigned long time[2]; //start and end of counting
		unsigned short slave_addr;
		//accumulated count values
		unsigned long cnts[36];
		void		Clear();
		void		Now();
		void		Add(klaus_cec_data* other);
		float		Duration();
		float		Rate(int channel);
		void		Print(int channel=-1, FILE* fd=stdout);
		void		PrintHeaderTransposed(FILE* fd=stdout);
		void		PrintTransposed(FILE* fd=stdout);
ClassDef(klaus_cec_data,1);
};

#endif
