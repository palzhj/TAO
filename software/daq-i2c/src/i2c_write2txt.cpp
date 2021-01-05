//I2C driver for I2C datapaths. prepared to be used in larger daq software. Based on the eeprommer source code in the i2c-tools package
//Author: K.Briggl

#include <sys/ioctl.h>
#include <errno.h>                                                              
#include <string.h>                                                             
#include <stdio.h>                                                              
#include <stdlib.h>                                                             
#include <unistd.h>                                                             
#include <fcntl.h>                                                              
#include <time.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <fstream>
#include <iostream>
using namespace std;


#define REG_ADDR_LEN 1
#define REG_DATA_LEN 2
#define EVENT_DATA_LEN 3
#define MAX_BLK_SIZE 255
#define NEVENTS 1000

#define WRITE_REG 0
#define READ_REG 1
#define READ_EVENTS 2
#define CHECK_EVENTS 3
#define SET_INTERFACE 4
#define BLOCK_READ 5
#define BLOCK_WRITE 6

#define VER       "KIP F11 - I2C test\n"

int block_write(int file,int dev_addr,int block_addr,unsigned char *buf,int length);

int block_read(int file,int dev_addr,unsigned char *buf,int length);

int block_read(int file,int dev_addr,int block_addr,unsigned char *buf,int length);

void build_address(int addr,unsigned char *buf);

int set_address(int file, int addr);

int init(char *device);




typedef struct {
	bool gainsel_evt;
	bool gainsel_busy;
	unsigned short ADC_10b;
	unsigned short ADC_6b;
	unsigned short ADC_PIPE;
	unsigned short time;
	unsigned char channelID;
	unsigned char groupID;
	unsigned short channel; //mapped to channel 0..7; T0 channel is mapped to 8
	static void PrintHeader(){
		printf("CH_MAP\tGROUP\tCHANNEL\tGS\tGSbusy\tADC_10b\tADC_6b\tPIPE\tTIME\n");
	};
	void Print(){
		printf("%2.2u\t%2.2u\t%2.2u\t%2.2u\t%2.2u\t%3.3u\t%2.2u\t%3.3u\t%3.3x\n",
			channel,
			groupID,
			channelID,
			gainsel_evt,
			gainsel_busy,
			ADC_10b,
			ADC_6b,
			ADC_PIPE,
			time
		);
	}
	void Parse(unsigned char* event){
		groupID=		( event[0] >> 4 ) & 0x003;
		channelID=		( event[0] >> 0 ) & 0x00f;
		gainsel_evt	=	( event[1] >> 7 ) & 0x001;
		gainsel_busy=		( event[1] >> 6 ) & 0x001;
		ADC_10b=		( (event[1] << 4 )&  0x3f0 ) | ( (event[2] >> 4)& 0x00f);
		ADC_6b=		( event[1] >> 0 ) & 0x03f;

		ADC_PIPE=		( (event[2] << 4)&  0xf0 ) | ( (event[3] >> 4)& 0x0f);
		time=			( (event[3] << 8)& 0xf00 ) | ( (event[4] << 0)& 0x0ff);

		ADC_10b = 1023 - ADC_10b;
		ADC_6b = 63 - ADC_6b;
		ADC_PIPE = 255 - ADC_PIPE;


		switch(groupID<<4 | channelID){
			case 0b110000 : channel=8; break; // T0 channel
			case 0b000000 : channel=0; break;
			case 0b000001 : channel=1; break;
			case 0b010000 : channel=2; break;
			case 0b010001 : channel=3; break;
			case 0b100000 : channel=4; break;
			case 0b100001 : channel=5; break;
			case 0b100010 : channel=6; break;
			//default: channel=0xff; break;
			default: channel=15; break;
		}



	}
} k4_event;

void events_print(int nevents, k4_event* events){
	k4_event::PrintHeader();
	for(int i=0;i<nevents;i++){
		events[i].Print();
	}
}

 
#define EVT_LEN 6
int events_read(int file, int dev_addr, int nevents, k4_event* events, unsigned char* tmp_buf){
	//read buffer
	if(block_read(file,dev_addr,tmp_buf,EVT_LEN*nevents)<0) return -1;
	//parse events, only keeping non empty ones
	int i=0;
	for(int n=0;n<nevents;n++){
		//if(tmp_buf[n*EVT_LEN]==0xfe){ //remove empty ones
		if(tmp_buf[n*EVT_LEN]==0x3f){ //remove empty ones
			//printf("-- Empty\n");
			continue;
		}	

		//for (int j=0;j<5;j++){printf("%2.2x ",tmp_buf[n*EVT_LEN+j]); } printf("\n");
		//for (int j=0;j<5;j++){for(int k=0;k<8;k++){printf("%d",!!( (tmp_buf[n*EVT_LEN+j]<<k)  & 0x80)); if(k==3){printf(" ");} } printf("\t"); } printf("\n");
		
		unsigned char* event= &(tmp_buf[n*EVT_LEN]);
		events[i].Parse(event);
		//k4_event::PrintHeader();
		//events[i].Print();
		i++;
	}

	return i;
}


#define CEC_LEN 8+1
#define CEC_ADDR 0
int cec_read(int file, int dev_addr, unsigned short* results, unsigned char* tmp_buf){
	//read register
	if(block_read(file,dev_addr,CEC_ADDR,tmp_buf,CEC_LEN)<0) return -1;
	unsigned char* buf = tmp_buf;
	//decompose results for each channel
	results[0]=(0x100&(buf[1]<<7)) | (0xff&(buf[0]>>0));
	results[1]=(0x180&(buf[2]<<6)) | (0x7f&(buf[1]>>1));
	results[2]=(0x1C0&(buf[3]<<5)) | (0x3f&(buf[2]>>2));
	results[3]=(0x1F0&(buf[4]<<4)) | (0x1f&(buf[3]>>3));
	results[4]=(0x1F8&(buf[5]<<3)) | (0x0f&(buf[4]>>4));
	results[5]=(0x1FC&(buf[6]<<2)) | (0x07&(buf[5]>>5));
	results[6]=(0x1FF&(buf[7]<<1)) | (0x03&(buf[6]>>6));
	return 7;
}

void cec_print(unsigned short* results,bool header=true){
	if(header) {
		for(int i=0;i<7;i++)
			printf("CH%d\t",i);
		printf("\n");
	}
	for(int i=0;i<7;i++){
		printf("%3.3u\t",results[i]);
	}
	printf("\n");
}

#define SLAVEARB_ADDR 0
int register_slave(int file, int dev_addr){
	unsigned char tmp_buf[2];
	if(block_write(file,dev_addr,SLAVEARB_ADDR,tmp_buf,2)<0) return -1;
	return 0;
}

/************************************************************************/
/*            Low level function	                                */	
/*									*/
/************************************************************************/

// Write block of data with address (not implemented in any chip yet...)
int block_write(int file,int dev_addr,int block_addr,unsigned char *buf,int length){
	struct i2c_msg msg[2];
	struct i2c_ioctl_rdwr_data {
		struct i2c_msg *msgs;  /* ptr to array of simple messages */              
		int nmsgs;             /* number of messages to exchange */ 
	} msgst;

	if ( length > (MAX_BLK_SIZE) ) {
		fprintf(stderr,"Error: Block too large:\n"); 
	}

	//unsigned char blk_addr=(addr & 0xFF);
	unsigned char blk_addr=(block_addr & 0xFF);
	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = REG_ADDR_LEN;
	msg[0].buf = &blk_addr;

	msg[1].addr = dev_addr;
	msg[1].flags = I2C_M_NOSTART;
	msg[1].len = length;
	msg[1].buf = buf;

	msgst.msgs = msg;	
	msgst.nmsgs = 2;
	if (ioctl(file,I2C_RDWR,&msgst) < 0){
		fprintf(stderr,"Error: Write block transaction failed: %s\n",strerror(errno)); 
		return 1;
	}
	return 0;
}


// Read block of data with address
int block_read(int file,int dev_addr,int block_addr,unsigned char *buf,int length){
	int ln=0;
	struct i2c_msg msg[2];
	struct i2c_ioctl_rdwr_data {
		struct i2c_msg *msgs;  /* ptr to array of simple messages */              
		int nmsgs;             /* number of messages to exchange */ 
	} msgst;

	if ( length > (MAX_BLK_SIZE) ) {
		fprintf(stderr,"Error: Block too large:\n"); 
	}	
	
	//printf("Reading from block address %d\n",0xFF&buff[0]);
	//unsigned char blk_addr=(addr & 0xFF);
	unsigned char blk_addr=(block_addr & 0xFF);
	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = REG_ADDR_LEN;
	msg[0].buf = &blk_addr;
	
	msg[1].addr = dev_addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = length;
	msg[1].buf = buf;

	msgst.msgs = msg;	
	msgst.nmsgs = 2;
	if ((ln = ioctl(file, I2C_RDWR, &msgst)) < 0) {
		fprintf(stderr,"Error: Write block transaction failed: %s\n",strerror(errno)); 
		return ln;
	}
	return ln;
}


// Read block of data without addressing
int block_read(int file,int dev_addr,unsigned char *buf,int length){
	int ln=0;
	int pos=0;
	int len;
	if ( length > (MAX_BLK_SIZE*100000) ) {
		fprintf(stderr,"Error: Total size too large:\n"); 
	}	

	while (pos<length){
		unsigned char* buffer=&buf[pos];
		if(pos+MAX_BLK_SIZE < length){
			len = MAX_BLK_SIZE;
		}else{
			len = length-pos;
		}
		//printf("Reading %d of %d bytes \n",len,length);
		ln = read(file, buffer,len);
		pos+=ln;
		if (ln < len) {
			fprintf(stderr,"Error: Read block transaction failed or finished after %d bytes read: %s\n",ln,strerror(errno)); 
			return pos;
		}
	}
	return len;
}

void build_address(int addr,unsigned char *buf){
		int k;
		k=addr;
		buf[0]=(unsigned char) (addr & 0xFF);
		//k=addr & 0xFF00 ;
		//buf[0]= ((unsigned char) (k >> 8)) & 0x7F;
	return;
}

int init(char *device) { 
	int file;	
	if ((file = open(device,O_RDWR)) < 0) {
  	 	fprintf(stderr,"Error: Could not open file %s\n", device);             
		return 0;
	}

	return file;
}

int set_address(int file, int addr){
	/* The I2C address */                                        
	if (ioctl(file,I2C_SLAVE,addr) < 0) {                                         
		fprintf(stderr,                                                             
	                  "Error: Cannot communicate with slave: %s\n",      
	                              strerror(errno));                                                   
		close(file);	      
		return 0;                                                                    
	}
	return 1;
}

k4_event gen_Event(int nevents)
{
	k4_event current_event;
	current_event.gainsel_evt = 0;
	current_event.gainsel_busy = 1;
	current_event.ADC_10b = 233;
	current_event.ADC_6b = 23;
	current_event.ADC_PIPE = 8;
	current_event.time = 12;
	current_event.channelID = 1;
	current_event.groupID = 0;

	return current_event;

}

int main(int argc, char **argv)
{
	
	static k4_event  current_event;
	k4_event* events = new k4_event[300];
	unsigned char* tmp_buf = (unsigned char*) malloc(300*EVT_LEN);
	int nevents = 10;
	if(argc<6)
	{
		printf("Usage: %s [/dev/i2c-x] [dev_addr] [nevents] [voltage] [output_filename]\n", argv[0]);
		return -1;
	}
	//if(argc>5) sleeptime = atoi(argv[4]);
	
	int ifile = init(argv[1]);	// open the bus
	set_address(ifile,atoi(argv[2]));	// Initiating communication with the slave device
	nevents = atoi(argv[3]);		// get the number of events to read
	double voltage;
	sscanf(argv[4],"%le",&voltage);
	voltage = voltage*1.0;
	char* filename=argv[5];

	printf("Reading %u events @ V=%e mV, fout=%s\n",nevents,voltage,filename);

	ofstream fout(filename,ios::out | ios::app);
	if(!fout.is_open())
	{
		printf("Error while opening the file\n");
		return -1;
	}

	unsigned int n_total=0;
	unsigned int n_read;
	unsigned int n_cycles=0;
	// get rid of the first 30 events
	//while (n_total<30){ n_read = events_read(ifile,atoi(argv[2]),10, events, tmp_buf);n_total += n_read; }
	
	n_cycles = 0;
	n_total = 0;
	while (n_total<nevents){
		n_read = events_read(ifile,atoi(argv[2]),10, events, tmp_buf);
		n_cycles++;
		for(int i=0;i<n_read;i++)
		{
			current_event = events[i];
			fout<<n_total+i<<"\t"<<voltage<<"\t"<<current_event.channel<<"\t"<<current_event.ADC_10b<<"\t";
			fout<<current_event.ADC_6b<<"\t"<<current_event.ADC_PIPE<<std::endl;
		}
		n_total+=n_read;
		//if(n_total%1000 == 1 ) printf("%u Events read\n",n_total);
	}

	// flush the FIFO to empty
	n_cycles = 0;
	while(n_cycles<300){
		n_read = events_read(ifile,atoi(argv[2]),10, events, tmp_buf);
		n_cycles++;
	}

	printf("Finished reading %u events after %u cycles\n",n_total,n_cycles);
	fout.close();
	printf("The END...\n");
	return 0;
}

