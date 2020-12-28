#include "klaus_i2c_iface.h"
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
#include <vector>
#include <iostream>
using namespace std;

#define EVT_LEN 5
#define CEC_LEN 8
#define CEC_ADDR 2
#define REG_ADDR_LEN 1
#define MAX_BLK_SIZE 255
#define MAX_CHUNKS MAX_BLK_SIZE/EVT_LEN
#define DAQLIMIT_AQULEN 10000

//#define ddprintf(args...) printf(args)
#define ddprintf(args...)

//#define dprintf(args...) printf(args)
#define dprintf(args...)



klaus_i2c_iface::klaus_i2c_iface(char *device)
{
	if ((m_fd = open(device,O_RDWR)) < 0) {
  	 	fprintf(stderr,"Error: Could not open file %s\n", device);             
		return;
	}
	m_current_chipaddr=0;
	m_chunksize=10;
	m_i2c_buf=(unsigned char*) malloc(EVT_LEN*MAX_CHUNKS);
	SetSlaveAddr(0x40);
}

klaus_i2c_iface::~klaus_i2c_iface()
{
	close(m_fd);
	free(m_i2c_buf);
}

void klaus_i2c_iface::SetChunksize(int size){
	if(size > MAX_CHUNKS){
		printf("klaus_i2c_iface::SetChunksize(%d): Maximum chunksize is limited to %d\n",MAX_CHUNKS);
		return;
	}
	m_chunksize=size;
};


int klaus_i2c_iface::SetSlaveAddr(unsigned char slave_addr){
	if(m_current_chipaddr!=slave_addr){
		// The I2C address 
		if (ioctl(m_fd,I2C_SLAVE,slave_addr) < 0) {                                         
			fprintf(stderr,"Error: Cannot communicate with slave: %s\n",
				strerror(errno));
			return -1;
		}
		m_current_chipaddr = slave_addr;
		return 1;
	}
	return 0;
}
int  klaus_i2c_iface::ReadEvents(unsigned char slave_addr, int nevents, std::list<klaus_event>& events, unsigned short current_blockID)
{
	SetSlaveAddr(slave_addr);
	//read buffer
	if(block_read(EVT_LEN*nevents)<0) return -1;
	//parse events, only keeping non empty ones
	int i=0;
	for(int n=0;n<nevents;n++)
	{
		if(m_i2c_buf[n*EVT_LEN]==0xfe)
		{ //remove empty ones
			continue;
		}	
		unsigned char* event= &(m_i2c_buf[n*EVT_LEN]);
		events.push_back(klaus_event(event,current_blockID));
		i++;
	}
	return i;
}

klaus_aquisition klaus_i2c_iface::ReadEventsUntilEmpty(unsigned char slave_addr, int min_chip, int max_tot){
	std::list<unsigned char> tmp_list;
	tmp_list.push_back(slave_addr);
	return ReadEventsUntilEmpty(tmp_list,min_chip,max_tot);
};

klaus_aquisition klaus_i2c_iface::ReadEventsUntilEmpty(std::list<unsigned char> ASICs, int min_chip, int max_tot){
	int n;
	unsigned short blockID=0;
	klaus_aquisition events;
	while(!ASICs.empty()){
		for(std::list<unsigned char>::iterator it=ASICs.begin();it!=ASICs.end();it++){
			n=ReadEvents(*it, m_chunksize, events.data[*it],blockID);
			if(n>0 || blockID<10) ddprintf("ReadEventsUntilEmpty() ASIC%u @ blk %u: Now %d Events ; ASIC %d ; Total %d \n",*it,blockID,n,events.data[*it].size(),events.nEvents);
			//Error reading, remove chip from list, increase data.failcnt:
			if(n<0){
				printf("ReadEventsUntilEmpty() ASIC%u @ blk %u: Now %d Events ; ASIC %d ; Total %d : TXERR -> dropping ASIC\n",*it,blockID,n,events.data[*it].size(),events.nEvents);
				events.failcnt++;
				it=ASICs.erase(it);
				continue;
			}

			events.nEvents+=n;

			//chip empty and enough events read per chip - remove chip from list:
			if((events.data[*it].size()>=min_chip) && (n<m_chunksize)){
				dprintf("ReadEventsUntilEmpty() ASIC%u @ blk %u: Now %d Events ; ASIC %d ; Total %d : EMPTY -> dropping ASIC\n",*it,blockID,n,events.data[*it].size(),events.nEvents);
				it=ASICs.erase(it);
			}

			//total number of read events above max_tot - return:
			if((events.nEvents> DAQLIMIT_AQULEN)||((max_tot>0) && (events.nEvents>= max_tot)))
				return events;
			blockID++;
		}
	}
	ddprintf("ReadEventsUntilEmpty() Finished, Total %d \n",events.nEvents);
	return events;
}

int  klaus_i2c_iface::FlushFIFO(unsigned char slave_addr,	int min_chip){
	std::list<unsigned char> tmp_list;
	tmp_list.push_back(slave_addr);
	return FlushFIFO(tmp_list,min_chip);
}

int  klaus_i2c_iface::FlushFIFO(std::list<unsigned char> ASICs,	int min_chip){
	klaus_aquisition sink=ReadEventsUntilEmpty(ASICs,min_chip,ASICs.size()*300);
	if(sink.failcnt>0) return -sink.failcnt;
	if(sink.nEvents<ASICs.size()*300) return 0;
	return -sink.nEvents;
}

int klaus_i2c_iface::ReadCEC(unsigned char slave_addr, klaus_cec_data& result){
	//read register
	gettimeofday(&(result.time),0);
	if(block_read(slave_addr,CEC_ADDR,m_i2c_buf,CEC_LEN+1)<0) return -1;
	//decompose results for each channel
        /*
        // TODO: below are codes for klaus4, I am quite confused with this codes
        // A CEC events contains 1-bit overflow sign and 8-bit datas
	result.cnts[0]=(0x100&(m_i2c_buf[1]<<7)) | (0xff&(m_i2c_buf[0]>>0));
	result.cnts[1]=(0x180&(m_i2c_buf[2]<<6)) | (0x7f&(m_i2c_buf[1]>>1));
	result.cnts[2]=(0x1C0&(m_i2c_buf[3]<<5)) | (0x3f&(m_i2c_buf[2]>>2));
	result.cnts[3]=(0x1F0&(m_i2c_buf[4]<<4)) | (0x1f&(m_i2c_buf[3]>>3));
	result.cnts[4]=(0x1F8&(m_i2c_buf[5]<<3)) | (0x0f&(m_i2c_buf[4]>>4));
	result.cnts[5]=(0x1FC&(m_i2c_buf[6]<<2)) | (0x07&(m_i2c_buf[5]>>5));
	result.cnts[6]=(0x1FF&(m_i2c_buf[7]<<1)) | (0x03&(m_i2c_buf[6]>>6));
        */
        // TODO: this code should be more generic
        if(constants::CH_NUM==7){
            result.cnts[0]=(0x100&(m_i2c_buf[1]<<8)) | (0xFF&(m_i2c_buf[0]>>0));
            result.cnts[1]=(0x180&(m_i2c_buf[2]<<7)) | (0x7F&(m_i2c_buf[1]>>1));
            result.cnts[2]=(0x1C0&(m_i2c_buf[3]<<6)) | (0x3F&(m_i2c_buf[2]>>2));
            result.cnts[3]=(0x1E0&(m_i2c_buf[4]<<5)) | (0x1F&(m_i2c_buf[3]>>3));
            result.cnts[4]=(0x1F0&(m_i2c_buf[5]<<4)) | (0x0F&(m_i2c_buf[4]>>4));
            result.cnts[5]=(0x1F8&(m_i2c_buf[6]<<3)) | (0x07&(m_i2c_buf[5]>>5));
            result.cnts[6]=(0x1FC&(m_i2c_buf[7]<<2)) | (0x03&(m_i2c_buf[6]>>6));
        }
        if(constants::CH_NUM==36){
            result.cnts[0]=(0x100&(m_i2c_buf[1]<<8)) | (0xFF&(m_i2c_buf[0]>>0));
            result.cnts[1]=(0x180&(m_i2c_buf[2]<<7)) | (0x7F&(m_i2c_buf[1]>>1));
            result.cnts[2]=(0x1C0&(m_i2c_buf[3]<<6)) | (0x3F&(m_i2c_buf[2]>>2));
            result.cnts[3]=(0x1E0&(m_i2c_buf[4]<<5)) | (0x1F&(m_i2c_buf[3]>>3));
            result.cnts[4]=(0x1F0&(m_i2c_buf[5]<<4)) | (0x0F&(m_i2c_buf[4]>>4));
            result.cnts[5]=(0x1F8&(m_i2c_buf[6]<<3)) | (0x07&(m_i2c_buf[5]>>5));
            result.cnts[6]=(0x1FC&(m_i2c_buf[7]<<2)) | (0x03&(m_i2c_buf[6]>>6));
            result.cnts[7]=(0x1FE&(m_i2c_buf[8]<<1)) | (0x01&(m_i2c_buf[7]>>7));

            result.cnts[8] =(0x100&(m_i2c_buf[10]<<8)) | (0xFF&(m_i2c_buf[9]>>0));
            result.cnts[9] =(0x180&(m_i2c_buf[11]<<7)) | (0x7F&(m_i2c_buf[10]>>1));
            result.cnts[10]=(0x1C0&(m_i2c_buf[12]<<6)) | (0x3F&(m_i2c_buf[11]>>2));
            result.cnts[11]=(0x1E0&(m_i2c_buf[13]<<5)) | (0x1F&(m_i2c_buf[12]>>3));
            result.cnts[12]=(0x1F0&(m_i2c_buf[14]<<4)) | (0x0F&(m_i2c_buf[13]>>4));
            result.cnts[13]=(0x1F8&(m_i2c_buf[15]<<3)) | (0x07&(m_i2c_buf[14]>>5));
            result.cnts[14]=(0x1FC&(m_i2c_buf[16]<<2)) | (0x03&(m_i2c_buf[15]>>6));
            result.cnts[15]=(0x1FE&(m_i2c_buf[17]<<1)) | (0x01&(m_i2c_buf[16]>>7));

            result.cnts[16]=(0x100&(m_i2c_buf[19]<<8)) | (0xFF&(m_i2c_buf[18]>>0));
            result.cnts[17]=(0x180&(m_i2c_buf[20]<<7)) | (0x7F&(m_i2c_buf[19]>>1));
            result.cnts[18]=(0x1C0&(m_i2c_buf[21]<<6)) | (0x3F&(m_i2c_buf[20]>>2));
            result.cnts[19]=(0x1E0&(m_i2c_buf[22]<<5)) | (0x1F&(m_i2c_buf[21]>>3));
            result.cnts[20]=(0x1F0&(m_i2c_buf[23]<<4)) | (0x0F&(m_i2c_buf[22]>>4));
            result.cnts[21]=(0x1F8&(m_i2c_buf[24]<<3)) | (0x07&(m_i2c_buf[23]>>5));
            result.cnts[22]=(0x1FC&(m_i2c_buf[25]<<2)) | (0x03&(m_i2c_buf[24]>>6));
            result.cnts[23]=(0x1FE&(m_i2c_buf[26]<<1)) | (0x01&(m_i2c_buf[25]>>7));

            result.cnts[24]=(0x100&(m_i2c_buf[28]<<8)) | (0xFF&(m_i2c_buf[27]>>0));
            result.cnts[25]=(0x180&(m_i2c_buf[29]<<7)) | (0x7F&(m_i2c_buf[28]>>1));
            result.cnts[26]=(0x1C0&(m_i2c_buf[30]<<6)) | (0x3F&(m_i2c_buf[29]>>2));
            result.cnts[27]=(0x1E0&(m_i2c_buf[31]<<5)) | (0x1F&(m_i2c_buf[30]>>3));
            result.cnts[28]=(0x1F0&(m_i2c_buf[32]<<4)) | (0x0F&(m_i2c_buf[31]>>4));
            result.cnts[29]=(0x1F8&(m_i2c_buf[33]<<3)) | (0x07&(m_i2c_buf[32]>>5));
            result.cnts[30]=(0x1FC&(m_i2c_buf[34]<<2)) | (0x03&(m_i2c_buf[33]>>6));
            result.cnts[31]=(0x1FE&(m_i2c_buf[35]<<1)) | (0x01&(m_i2c_buf[34]>>7));

            result.cnts[32]=(0x100&(m_i2c_buf[37]<<8)) | (0xFF&(m_i2c_buf[36]>>0));
            result.cnts[33]=(0x180&(m_i2c_buf[38]<<7)) | (0x7F&(m_i2c_buf[37]>>1));
            result.cnts[34]=(0x1C0&(m_i2c_buf[39]<<6)) | (0x3F&(m_i2c_buf[38]>>2));
            result.cnts[35]=(0x1E0&(m_i2c_buf[40]<<5)) | (0x1F&(m_i2c_buf[39]>>3));
        }

	return constants::CH_NUM;
}

// Read block of data without addressing
int klaus_i2c_iface::block_read(int length)
{
	int ln=0;
	int pos=0;
	int len;
	if ( length > (MAX_BLK_SIZE*100000) ) {
		fprintf(stderr,"Error: Total size too large:\n"); 
	}	

	while (pos<length){
		unsigned char* buffer=&m_i2c_buf[pos];
		if(pos+MAX_BLK_SIZE < length){
			len = MAX_BLK_SIZE;
		}else{
			len = length-pos;
		}
		//printf("file discriptor=%d, Reading %d of %d bytes \n",m_fd,len,length);
		ln = read(m_fd, buffer,len);
		pos+=ln;
		if (ln < len) {
			fprintf(stderr,"Error: Read block transaction failed or finished after %d bytes read: %s\n",ln,strerror(errno)); 
			return pos;
		}
	}
	return len;
}



int klaus_i2c_iface::block_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, int length)
{
	struct i2c_msg msg[2];
	struct i2c_ioctl_rdwr_data 
	{
		struct i2c_msg *msgs;  /* ptr to array of simple messages */              
		int nmsgs;             /* number of messages to exchange */ 
	} msgst;

	if ( length > (MAX_BLK_SIZE) ) 
	{
		fprintf(stderr,"Error: Block too large:\n"); 
	}

	msg[0].addr = slave_addr;
	msg[0].flags = 0;
	msg[0].len = REG_ADDR_LEN;
	msg[0].buf = &reg_addr;

	msg[1].addr = slave_addr;
	msg[1].flags = I2C_M_NOSTART;
	msg[1].len = length;
	msg[1].buf = buf;

	msgst.msgs = msg;	
	msgst.nmsgs = 2;
	if (ioctl(m_fd,I2C_RDWR,&msgst) < 0)
	{
		fprintf(stderr,"Error: Write block transaction failed: %s\n",strerror(errno)); 
		return 1;
	}
	return 0;
}

// Read block of data with specified register address
int klaus_i2c_iface::block_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, int length)
{
	int ln=0;
	struct i2c_msg msg[2];
	struct i2c_ioctl_rdwr_data {
		struct i2c_msg *msgs;  // ptr to array of simple messages               
		int nmsgs;             // number of messages to exchange  
	} msgst;

	if ( length > (MAX_BLK_SIZE) ) {
		fprintf(stderr,"Error: Block too large:\n"); 
	}	
	
	//printf("Reading from block address %d\n",0xFF&buff[0]);
	//unsigned char blk_addr=(addr & 0xFF);
	msg[0].addr = slave_addr;
	msg[0].flags = 0;
	msg[0].len = REG_ADDR_LEN;
	msg[0].buf = &reg_addr;
	
	msg[1].addr = slave_addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = length;
	msg[1].buf = m_i2c_buf;

	msgst.msgs = msg;	
	msgst.nmsgs = 2;
	if ((ln = ioctl(m_fd, I2C_RDWR, &msgst)) < 0) {
		fprintf(stderr,"Error: Write block transaction failed: %s\n",strerror(errno)); 
		return ln;
	}
	return ln;
}

