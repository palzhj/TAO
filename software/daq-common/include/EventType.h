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
		bool		gainsel_busy;
		unsigned short	ADC_10b;
		unsigned short	ADC_6b;
		unsigned short	ADC_PIPE;
		unsigned short	time;
		unsigned char	channelID;
		unsigned char	groupID;
		unsigned char	channel;
                
                // Parse the data stream received through the I2C interface
		void		Parse(unsigned char* event);

                // Print 
		static void	PrintHeader(FILE* fd=stdout);
		void		Print(FILE* fd=stdout);

                // transfer the time from Gray to decimal, and get the time interval wrt the last event 
		unsigned int	AbsTime() const;
		int 		DiffTime(const klaus_event& evt);

		//DAQ fields
		unsigned short	acq_blk; //Aquisition block cycle

        ClassDef(klaus_event,2);
};

//Collection of klaus events with the same Aquisition number for different ASICs
//This data type is intended to be stored or transmitted!
class klaus_aquisition: public TObject{
        public:
        	klaus_aquisition();

                // the ASIC ID and all the events in the same acquisition ID for this ASIC are paired
        	std::map<unsigned char, std::list<klaus_event> > data;
        	unsigned long aqu_ID;   // a unique aquisition ID for the klaus_aquisition type
        
        	unsigned short nEvents;
        	unsigned short failcnt;
        	void	Print(FILE* fd=stdout);


        ClassDef(klaus_aquisition,1);
};









////////////////////////////////////////////////////////////////
///                 KLauS CEC TYPES
///////////////////////////////////////////////////////////////
class klaus_cec_data: public TObject
{
	public:
		klaus_cec_data(){}

		timeval time;
		unsigned short slave_addr;
		unsigned short cnts[constants::CH_NUM];

		void		Print(timeval* last_time=NULL, FILE* fd=stdout);
		void		PrintHeaderTransposed(FILE* fd=stdout);
		void		PrintTransposed(FILE* fd=stdout);
ClassDef(klaus_cec_data,1);
};

#endif
