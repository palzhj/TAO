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

#define EVT_LEN 6
#define CEC_LEN 41
#define CEC_ADDR 2
#define REG_ADDR_LEN 1
//#define MAX_BLK_SIZE 255
#define MAX_BLK_SIZE 3000
#define MAX_CHUNKS MAX_BLK_SIZE/EVT_LEN
#define DAQLIMIT_AQULEN 1000

//#define ddprintf(args...) printf(args)
#define ddprintf(args...)

//#define dprintf(args...) printf(args)
#define dprintf(args...)

klaus_i2c_iface::klaus_i2c_iface(char *device)
{
	m_current_chipaddr=-1;
	m_chunksize=500;
	m_quiet = true;
	m_i2c_buf=(unsigned char*) malloc(MAX_BLK_SIZE);
	if (device != NULL) {
		m_python_mode = false;
		if ((m_fd = open(device,O_RDWR)) < 0) {
  		 	fprintf(stderr,"Error: Could not open file %s\n", device);             
			return;
		}
	} else {
		m_python_mode = true;

		Py_Initialize();
		PyRun_SimpleString("import sys");
		PyRun_SimpleString("sys.path.insert(0, \"../../script\")");
		PyRun_SimpleString("sys.path.insert(0, \"../../script/lib\")");
		PyObject *pName = PyUnicode_FromString("klaus6");
		PyObject *pModule = PyImport_Import(pName);

		if (pModule == nullptr) {
			PyErr_Print();
			std::cerr << "Failed to import the klaus6 module.\n";
			exit(-1);
		}
		//Py_DECREF(pName);

		PyObject *pDict = PyModule_GetDict(pModule);
		if (pDict == nullptr) {
			PyErr_Print();
			std::cerr << "Failed to get the dictionary.\n";
			exit(-1);
		}
		//Py_DECREF(pModule);

		pClass = PyDict_GetItemString(pDict, "klaus6");
		if (pClass == nullptr) {
			PyErr_Print();
			std::cerr << "Failed to get the Python class.\n";
			exit(-1);
		}
		//Py_DECREF(pDict);
	        //SetSlaveAddr(0);
	}
}

klaus_i2c_iface::~klaus_i2c_iface()
{
	//if (m_python_mode) Py_Finalize();
	//else close(m_fd);
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
	if (!m_python_mode) {
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
	} else {
		if(m_current_chipaddr!=slave_addr){
			PyObject *py_args = PyTuple_New(1);
			unsigned char real_slave_addr;
			if (slave_addr == 0) real_slave_addr = 0x40;
			PyTuple_SetItem(py_args, 0, PyLong_FromLong(real_slave_addr<<1));
			if (PyCallable_Check(pClass)) {
				pClass_inst = PyObject_CallObject(pClass, py_args);
				//Py_DECREF(pClass);
			} else {
				std::cout << "Cannot instantiate the Python class" << std::endl;
				//Py_DECREF(pClass);
				exit(-1);
			}
			m_current_chipaddr = slave_addr;
			return 1;
		}
	}
	return 0;
}

int  klaus_i2c_iface::ReadEvents(unsigned char slave_addr, int nevents, std::list<klaus_event>& events, unsigned short current_blockID)
{
	SetSlaveAddr(slave_addr);

	//read buffer
	int nevt_read = block_read(EVT_LEN*nevents);
	if (nevt_read < 0) return -1;
	
	//convert bytes to event
	int i=0;
	for(int n=0;n<nevt_read;n++)
	{
		if (!m_quiet) {
			for (int j=0; j<EVT_LEN; j++) printf("%2.2x ",m_i2c_buf[n*EVT_LEN+j]);
			printf("\n");
		}

		// The following check has been done in klaus6.py, thus it's commented out
		//if(m_i2c_buf[n*EVT_LEN]==0x3f)
		//{ //remove empty ones
		//	continue;
		//}
		
		unsigned char* event= &(m_i2c_buf[n*EVT_LEN]);
		events.push_back(klaus_event(event,current_blockID));
		i++;
	}
	return i;
}

klaus_acquisition klaus_i2c_iface::ReadEventsUntilEmpty(unsigned char slave_addr, int min_chip, int max_tot){
	std::list<unsigned char> tmp_list;
	tmp_list.push_back(slave_addr);
	return ReadEventsUntilEmpty(tmp_list,min_chip,max_tot);
};

klaus_acquisition klaus_i2c_iface::ReadEventsUntilEmpty(std::list<unsigned char> ASICs, int min_chip, int max_tot){
	int n;
	unsigned short blockID=0;
	klaus_acquisition events;
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
			//if (!m_quiet) std::cout << events.nEvents << " events read so far for chip " << int(*it) << std::endl;
			std::cout << events.nEvents << " events read so far for chip " << int(*it) << std::endl;

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
	klaus_acquisition sink=ReadEventsUntilEmpty(ASICs,min_chip,ASICs.size()*300);
	if(sink.failcnt>0) return -sink.failcnt;
	if(sink.nEvents<ASICs.size()*300) return 0;
	return -sink.nEvents;
}

int klaus_i2c_iface::ReadCEC(unsigned char slave_addr, klaus_cec_data& result){
	result.Now();
	//read register
	if(block_read(slave_addr,CEC_ADDR,m_i2c_buf,CEC_LEN+1)<0) return -1;
	//printf("CEC read: ");
	//for(int i=0;i<CEC_LEN+1;i++){
	//	printf("%2.2X ",m_i2c_buf[i]);
	//}
	//printf("\n");


	//decompose results for each channel
        // TODO: this code should be more generic
        result.cnts[0]=(0x100&(m_i2c_buf[40]<<8)) | (0xFF&(m_i2c_buf[41]>>0));
        result.cnts[1]=(0x180&(m_i2c_buf[39]<<7)) | (0x7F&(m_i2c_buf[40]>>1));
        result.cnts[2]=(0x1C0&(m_i2c_buf[38]<<6)) | (0x3F&(m_i2c_buf[39]>>2));
        result.cnts[3]=(0x1E0&(m_i2c_buf[37]<<5)) | (0x1F&(m_i2c_buf[38]>>3));
        result.cnts[4]=(0x1F0&(m_i2c_buf[36]<<4)) | (0x0F&(m_i2c_buf[37]>>4));
        result.cnts[5]=(0x1F8&(m_i2c_buf[35]<<3)) | (0x07&(m_i2c_buf[36]>>5));
        result.cnts[6]=(0x1FC&(m_i2c_buf[34]<<2)) | (0x03&(m_i2c_buf[35]>>6));
        result.cnts[7]=(0x1FE&(m_i2c_buf[33]<<1)) | (0x01&(m_i2c_buf[34]>>7));

        result.cnts[8] =(0x100&(m_i2c_buf[31]<<8)) | (0xFF&(m_i2c_buf[32]>>0));
        result.cnts[9] =(0x180&(m_i2c_buf[30]<<7)) | (0x7F&(m_i2c_buf[31]>>1));
        result.cnts[10]=(0x1C0&(m_i2c_buf[29]<<6)) | (0x3F&(m_i2c_buf[30]>>2));
        result.cnts[11]=(0x1E0&(m_i2c_buf[28]<<5)) | (0x1F&(m_i2c_buf[29]>>3));
        result.cnts[12]=(0x1F0&(m_i2c_buf[27]<<4)) | (0x0F&(m_i2c_buf[28]>>4));
        result.cnts[13]=(0x1F8&(m_i2c_buf[26]<<3)) | (0x07&(m_i2c_buf[27]>>5));
        result.cnts[14]=(0x1FC&(m_i2c_buf[25]<<2)) | (0x03&(m_i2c_buf[26]>>6));
        result.cnts[15]=(0x1FE&(m_i2c_buf[24]<<1)) | (0x01&(m_i2c_buf[25]>>7));

        result.cnts[16]=(0x100&(m_i2c_buf[22]<<8)) | (0xFF&(m_i2c_buf[23]>>0));
        result.cnts[17]=(0x180&(m_i2c_buf[21]<<7)) | (0x7F&(m_i2c_buf[22]>>1));
        result.cnts[18]=(0x1C0&(m_i2c_buf[20]<<6)) | (0x3F&(m_i2c_buf[21]>>2));
        result.cnts[19]=(0x1E0&(m_i2c_buf[19]<<5)) | (0x1F&(m_i2c_buf[20]>>3));
        result.cnts[20]=(0x1F0&(m_i2c_buf[18]<<4)) | (0x0F&(m_i2c_buf[19]>>4));
        result.cnts[21]=(0x1F8&(m_i2c_buf[17]<<3)) | (0x07&(m_i2c_buf[18]>>5));
        result.cnts[22]=(0x1FC&(m_i2c_buf[16]<<2)) | (0x03&(m_i2c_buf[17]>>6));
        result.cnts[23]=(0x1FE&(m_i2c_buf[15]<<1)) | (0x01&(m_i2c_buf[16]>>7));

        result.cnts[24]=(0x100&(m_i2c_buf[13]<<8)) | (0xFF&(m_i2c_buf[14]>>0));
        result.cnts[25]=(0x180&(m_i2c_buf[12]<<7)) | (0x7F&(m_i2c_buf[13]>>1));
        result.cnts[26]=(0x1C0&(m_i2c_buf[11]<<6)) | (0x3F&(m_i2c_buf[12]>>2));
        result.cnts[27]=(0x1E0&(m_i2c_buf[10]<<5)) | (0x1F&(m_i2c_buf[11]>>3));
        result.cnts[28]=(0x1F0&(m_i2c_buf[9]<<4)) | (0x0F&(m_i2c_buf[10]>>4));
        result.cnts[29]=(0x1F8&(m_i2c_buf[8]<<3)) | (0x07&(m_i2c_buf[9]>>5));
        result.cnts[30]=(0x1FC&(m_i2c_buf[7]<<2)) | (0x03&(m_i2c_buf[8]>>6));
        result.cnts[31]=(0x1FE&(m_i2c_buf[6]<<1)) | (0x01&(m_i2c_buf[7]>>7));

        result.cnts[32]=(0x100&(m_i2c_buf[4]<<8)) | (0xFF&(m_i2c_buf[5]>>0));
        result.cnts[33]=(0x180&(m_i2c_buf[3]<<7)) | (0x7F&(m_i2c_buf[4]>>1));
        result.cnts[34]=(0x1C0&(m_i2c_buf[2]<<6)) | (0x3F&(m_i2c_buf[3]>>2));
        result.cnts[35]=(0x1E0&(m_i2c_buf[1]<<5)) | (0x1F&(m_i2c_buf[2]>>3));

	//cut off overflow
	for(int i=0;i<35;i++)
		if(result.cnts[i]&0x100) result.cnts[i]=0xff;

	return constants::CH_NUM;
}

// Read block of data without addressing
int klaus_i2c_iface::block_read(int length)
{
	if (!m_python_mode) {
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
	} else {
		if ( length > (MAX_BLK_SIZE*100000) ) {
			fprintf(stderr,"Error: Total size too large:\n"); 
		}	

		int nevt_toread = (int) length/EVT_LEN;
		PyObject *value = PyObject_CallMethod(pClass_inst, "readEvents", "(i)", nevt_toread);
    	if (value)
			;//Py_DECREF(value);
    	else
 			PyErr_Print();
		
		PyObject *value2 = PyObject_CallMethod(pClass_inst, "nevtRead", nullptr);
    	if (value2)
			;//Py_DECREF(value2);
    	else
 			PyErr_Print();

		char* signed_bytes = PyBytes_AsString(value);
		int nevt_read = PyLong_AsLong(value2);
		for (int i = 0; i < nevt_read*EVT_LEN; i++) {
			m_i2c_buf[i] = (unsigned char)signed_bytes[i];
		}
	
		if (nevt_read == 0) return -1;
		else return nevt_read;
	}
}

int klaus_i2c_iface::block_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, int length)
{
	/*
	struct i2c_msg msg[2];
	struct i2c_ioctl_rdwr_data 
	{
		struct i2c_msg *msgs;  
		int nmsgs;             
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
	}*/
	return 0;
}

// Read block of data with specified register address
int klaus_i2c_iface::block_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, int length)
{
	/*
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
	return ln; */
	return 0;
}

void klaus_i2c_iface::BeQuiet()
{
	m_quiet = true;
	if (m_python_mode) PyObject_CallMethod(pClass_inst, "beQuiet", nullptr);
}
