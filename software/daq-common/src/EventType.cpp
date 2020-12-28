#include <stdio.h>
#include <stdlib.h>
#include "EventType.h"

////////////////////////////// klaus_event //////////////////////////////////

void klaus_event::PrintHeader(FILE* fd)
{
	fprintf(fd,"CH_MAP\tGROUP\tCHANNEL\tGS\tGSbusy\tADC_10b\tADC_6b\tPIPE\tTIME\n");
}

void klaus_event::Print(FILE* fd)
{
	fprintf(fd,"%2.2u\t%2.2u\t%2.2u\t%2.2u\t%2.2u\t%3.3u\t%2.2u\t%3.3u\t%3.3x\n",
		channel,
		groupID,
		channelID,
		gainsel_evt,
		gainsel_busy,
		ADC_10b,
		ADC_6b,
		ADC_PIPE,
		AbsTime()
	);
}

// This function needs modifications when it comes to the new version ASIC
//
// For KlauS5
void  klaus_event::Parse(unsigned char* event)
{
	groupID=		( event[0] >> 4 ) & 0x0003;
	channelID=		( event[0] >> 0 ) & 0x000f;
	gainsel_evt	=	( event[1] >> 7 ) & 0x0001;
	gainsel_busy=		( event[1] >> 6 ) & 0x0001;
	ADC_10b=		((event[1] << 4 ) & 0x03f0) | ((event[2] >> 4) & 0x000f);
	ADC_6b=		        ( event[1] >> 0 ) & 0x003f;
	ADC_PIPE=               ( event[2] >> 0 ) & 0x00ff;
	time=			((event[3] << 8 ) & 0xff00) | ((event[4] << 0) & 0x00ff);

	ADC_10b= 	1023-ADC_10b;
	ADC_6b=		  63-ADC_6b;
	ADC_PIPE= 	 255-ADC_PIPE;
        
        // This is verfied, for T0 channel: groupID=3, channelID=0
        channel = 0xff;
        if(groupID==3&&channelID==0) channel = 36; // T0 channel
        else if(channelID<12) channel = 12*groupID + (int)channelID;
	
}

/* Parse method for KLauS4
void  klaus_event::Parse(unsigned char* event)
{
	groupID=		( event[0] >> 4 ) & 0x003;
	channelID=		( event[0] >> 0 ) & 0x00f;
	gainsel_evt	=	( event[1] >> 7 ) & 0x001;
	gainsel_busy=		( event[1] >> 6 ) & 0x001;
	ADC_10b=		( (event[1] << 4 )&  0x3f0 ) | ( (event[2] >> 4)& 0x00f);
	ADC_6b=		( event[1] >> 0 ) & 0x03f;

	ADC_PIPE=		( (event[2] << 4)&  0xf0 ) | ( (event[3] >> 4)& 0x0f);
	time=			( (event[3] << 8)& 0xf00 ) | ( (event[4] << 0)& 0x0ff);

	ADC_10b= 	1023-ADC_10b;
	ADC_6b=		  63-ADC_6b;
	ADC_PIPE= 	 255-ADC_PIPE;

	switch(groupID<<4 | channelID)
	{
		case 0b110000 : channel=7; break; // T0 channel
		case 0b000000 : channel=0; break;
		case 0b000001 : channel=1; break;
		case 0b010000 : channel=2; break;
		case 0b010001 : channel=3; break;
		case 0b100000 : channel=4; break;
		case 0b100001 : channel=5; break;
		case 0b100010 : channel=6; break;
		default: channel=0xff; break;
	}
}
*/
//convert gray encoded data into binary representation [LSB : MSB]
inline unsigned short grayToBinary(unsigned short num) {
	unsigned short mask;
	for (mask = num >> 1; mask != 0; mask = mask >> 1)
		num = num ^ mask;
	return num;
}

unsigned int	klaus_event::AbsTime() const{
	unsigned int res=grayToBinary(time);
	return res;
}

int klaus_event::DiffTime(const klaus_event& evt){
	return (this->AbsTime()-evt.AbsTime());
}

ClassImp(klaus_event);


////////////////////////////// klaus_acquisition //////////////////////////////////

klaus_aquisition::klaus_aquisition(){
	nEvents=0;
	failcnt=0;
	aqu_ID=0;
};

void klaus_aquisition::Print(FILE* fd){
	fprintf(fd," AQUISITION #%d:\n\t Total %d entries\n\t Total %d TX failures.\n",aqu_ID,nEvents,failcnt);
	for (std::map<unsigned char, std::list<klaus_event> >::iterator it=data.begin();it!=data.end();it++){
		fprintf(fd," AQUISITION #%d:\t CHIP %u: %d Entries\n",aqu_ID,it->first,it->second.size());
		klaus_event::PrintHeader(fd);
		for(std::list<klaus_event>::iterator itE=it->second.begin();itE!=it->second.end();itE++){
			itE->Print(fd);
		}
	}
	
}

ClassImp(klaus_aquisition);


////////////////////////////// klaus_cec_event //////////////////////////////////

void klaus_cec_data::Print(timeval* last_time, FILE* fd) {
	long unsigned musec_tot=1;
	if(last_time!=NULL){
		timeval difftime;
		timersub(last_time,&time,&difftime);
		musec_tot=difftime.tv_sec*1000000+difftime.tv_sec;
	}
	fprintf(fd,"TIME_SEC\tTIME_USEC\tSLAVE\tCH\tCNTS\tRATE\n");
	for (int i=0; i<constants::CH_NUM;i++){
		fprintf(fd,"%5.5u\t%5.5u\t%3.3u\t%2.2u\t%3.3u\t%3.3e Hz\n",
			time.tv_sec,
			time.tv_usec,
			slave_addr,
			i,
			cnts[i],
			cnts[i]*1.0/(1e-6*musec_tot)
		);
	}
}

void	klaus_cec_data::PrintHeaderTransposed(FILE* fd){
		for(int i=0;i<constants::CH_NUM;i++)
			printf("S%u.CH%d\t",slave_addr,i);
		printf("\n");
}

void	klaus_cec_data::PrintTransposed(FILE* fd){
	for(int i=0;i<constants::CH_NUM;i++){
			printf("%3.3u\t",cnts[i]);
		}
		printf("\n");
}

ClassImp(klaus_cec_data);

