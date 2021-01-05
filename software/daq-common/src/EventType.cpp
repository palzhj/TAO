#include <stdio.h>
#include <stdlib.h>
#include "EventType.h"

////////////////////////////// klaus_event //////////////////////////////////
int klaus_event::foffset=0;
int klaus_event::moffset=1;



void klaus_event::PrintHeader(FILE* fd)
{
	fprintf(fd,"CHANNEL\tGS\tADC_10b\tADC_6b\tPIPE\tT_CC\tT_MC\tT_FC\tTime\n");
}

void klaus_event::Print(FILE* fd)
{
	fprintf(fd,"%2.2u\t%1.1u\t%3.3u\t%2.2u\t%3.3u\t%d\t%d\t%d\t%d\n",
		channel,
		gainsel_evt,
		ADC_10b,
		ADC_6b,
		ADC_PIPE,
		T_CC,
                T_MC,
		T_FC,
		this->GetTime()
	);
}

// this function is implemented in KLauS-6 
// because the MC[2:0] is mistaken as MC[0:2],
// so that a remap is needed. Also compensates on-chip gray to binary, MC[2] inversion.
//MC offset must be set to zero on the chip; factory mode enabled
unsigned short	MapMC(unsigned short MC){
	const unsigned short map[]={6,1,2,5,4,3,0,7};
	return map[MC];
}

//convert gray encoded data into binary representation [LSB : MSB]
//inline unsigned short grayToBinary(unsigned short num) {
/*
inline unsigned short grayToBinary(unsigned long num) {
	unsigned short mask;
	for (mask = num >> 1; mask != 0; mask = mask >> 1)
		num = num ^ mask;
	return num;
}
*/



// This function needs modifications when it comes to the new version ASIC
void  klaus_event::Parse(unsigned char* event)
{
	groupID         =	( event[0] >> 6 ) & 0x0003;     // evt[0]: (7:6)
	channelID       =	( event[0] >> 2 ) & 0x000f;     // evt[0]: (5:2)
	gainsel_evt	=	( event[0] >> 1 ) & 0x0001;     // evt[0]: (1:1)
	ADC_10b         =	((event[0] << 9 ) & 0x0200) | ((event[1] << 1) & 0x01fe) | ((event[2] >> 7) & 0x0001);
	ADC_6b          =	((event[0] << 5 ) & 0x0020) | ((event[1] >> 3) & 0x001f);
	ADC_PIPE        =       ((event[1] << 5 ) & 0x0007) | ((event[2] >> 3) & 0x001f);
	T_CC            =	((event[2] <<16 ) &0x70000) | ((event[3] << 8 )& 0x0ff00) | ((event[4] << 0)& 0x000ff);
        T_MC            =       ((event[5] >> 5 ) & 0x0007);
        T_FC            =       ((event[5] << 0 ) & 0x001f);

	ADC_10b         = 	1023    -       ADC_10b;
	ADC_6b          =	63      -       ADC_6b;
	ADC_PIPE        = 	255     -       ADC_PIPE;
	T_FC		= 	31	-	T_FC;	// correct
	T_MC		= 	MapMC(T_MC);	// correct
        
        // This is verfied, for T0 channel: groupID=3, channelID=0
        channel = 0xff;
        if(groupID==3&&channelID==15) channel = 36; // T0 channel
        else if(channelID<9) channel = 9*groupID + (int)channelID;
	
}

unsigned long	klaus_event::GetTime() const{
	unsigned short  FC = (T_FC + foffset)            & 0x1f;        // offset correction
	unsigned short  MC = (T_MC + moffset)            & 0x07;        // offset correction
	unsigned short  mctemp = MC>>1 &0x03;                                 // align MC according to FC
	unsigned long   cctemp = (T_CC%(1<<19))>>1;                     // align CC according to aligned-MC
	if(MC%2==1 && FC/8==0) mctemp = ((MC/2) + 1)     & 0x03;
	if(MC%2==0 && FC/8==3) mctemp = ((MC/2) - 1 + 4) & 0x03;

	if(T_CC%2==1 && mctemp==0) cctemp = (cctemp + 1) & 0x1ffff;
	if(T_CC%2==0 && mctemp==3) cctemp = (cctemp + (1<<18) - 1) & 0x1ffff;
	return (cctemp<<7)+(mctemp<<5)+FC;
}

int klaus_event::DiffTime(klaus_event& evt) const{
	return (this->GetTime()-evt.GetTime());
}
ClassImp(klaus_event);


////////////////////////////// klaus_acquisition //////////////////////////////////

klaus_acquisition::klaus_acquisition(){
	nEvents=0;
	failcnt=0;
	acqu_ID=0;
};

void klaus_acquisition::Print(FILE* fd){
	fprintf(fd," ACQUISITION #%d:\n\t Total %d entries\n\t Total %d TX failures.\n",acqu_ID,nEvents,failcnt);
	for (std::map<unsigned char, std::list<klaus_event> >::iterator it=data.begin();it!=data.end();it++){
		fprintf(fd," ACQUISITION #%d:\t CHIP %u: %d Entries\n",acqu_ID,it->first,it->second.size());
		klaus_event::PrintHeader(fd);
		for(std::list<klaus_event>::iterator itE=it->second.begin();itE!=it->second.end();itE++){
			itE->Print(fd);
		}
	}
	
}

ClassImp(klaus_acquisition);


////////////////////////////// klaus_cec_event //////////////////////////////////
void klaus_cec_data::Clear(){
	Now();
	time[0]=time[1];
	//printf("klaus_cec_data::Clear(): T0=%lu\n",time[0]);
	slave_addr=0;
	for(int i=0;i<constants::CH_NUM;i++)
		cnts[i]=0;
}
void klaus_cec_data::Now(){
	timeval t;
	gettimeofday(&t,0);
	time[1]=(t.tv_sec*1000+(t.tv_usec/1000));
}

void klaus_cec_data::Add(klaus_cec_data* other){
	time[1]=other->time[1]; //update end time, assuming other is newer
//	if(slave_addr==0)
//		slave_addr=other->slave_addr;

//	if(slave_addr==other->slave_addr){
		for(int i=0;i<constants::CH_NUM;i++)
			cnts[i]+=other->cnts[i];
//	}
}


float klaus_cec_data::Duration(){
	return (time[1]-time[0])*1e-3;
}
float klaus_cec_data::Rate(int channel){
	if(channel < 0 || channel > 35)
		return -1;
	if(Duration()==0)
		return -1;
	return 1e6*cnts[channel]/Duration();
}

void klaus_cec_data::Print(int channel, FILE* fd) {
	float dt=Duration();
	if(dt==0) dt=-1;	
	fprintf(fd,"TIME_USEC0\tTIME_USEC1\tDT_sec\tSLAVE\tCH\tCNTS\tRATE\n");
	
	for (int i=0; i<constants::CH_NUM;i++){
		if(channel>=0)
			i=channel;
		fprintf(fd,"%5.5lu\t%5.5lu\t%2.5f\t%3.3u\t%2.2u\t%3.3u\t%3.3e Hz\n",
			time[0],
			time[1],
			Duration(),
			slave_addr,
			i,
			cnts[i],
			Rate(i)
		);
		if(channel>=0)
			break;
	}
}

void	klaus_cec_data::PrintHeaderTransposed(FILE* fd){
		for(int i=0;i<constants::CH_NUM;i++)
			printf("S%u.CH%d\t",slave_addr,i);
		printf("\n");
}

void	klaus_cec_data::PrintTransposed(FILE* fd){
	for(int i=0;i<5;i++){
			printf("     %3.3u ",cnts[i]);
		}
	printf("\n");
	for(int i=0;i<5;i++){
			printf("%2.2e ",Rate(i));
		}
	printf("\n");
}

ClassImp(klaus_cec_data);

