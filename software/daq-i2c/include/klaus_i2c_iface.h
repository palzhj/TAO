#ifndef K4_I2C_IFACE_H
#define K4_I2C_IFACE_H

#include "EventType.h"
#include <list>
#include <iostream>
#include <python3.6m/Python.h>

class	klaus_i2c_iface
{
	public:

		klaus_i2c_iface(char *device = NULL);
		~klaus_i2c_iface();

		int SetSlaveAddr(unsigned char slave_addr);

		//Read specified number of events from chip in one transaction.
		//Events are appended to list, empty events are not stored.
		int  ReadEvents(unsigned char slave_addr,	int nevents, std::list<klaus_event>& events, unsigned short current_blockID=0);

		//Set block size (in Events) for ReadUntil and Flush methods
		void SetChunksize(int size);

		//Read events in consecutive transactions until less than m_chunksize non-empty events are read (chip is empty)
		//Read at least min_chip non-empty events from each chip
		//Read maximum max_tot non-empty events in total. Max is ignored if set to -1
		//Events are appended to the AquisitionClass, empty events are not stored.
		//Multi-ASIC: balanced reading multiple chips by reading N events at a time (set by SetChunksize()), then trying next ASIC
		klaus_acquisition ReadEventsUntilEmpty(unsigned char slave_addr,	int min_chip=0, int max_tot=-1);
		klaus_acquisition ReadEventsUntilEmpty(std::list<unsigned char> ASICs,	int min_chip=0, int max_tot=-1);

		//Same Transaction as ReadEventsUntilEmpty, but no storage of events. Will read maximum 300 Events per chip. 
		//Return:
		//0 on success
		//-failures: Transmission errors on N ASICs
		//-Nread: More events read as expected, suggests noise triggers
		int  FlushFIFO(unsigned char slave_addr,	int min_chip=0);
		int  FlushFIFO(std::list<unsigned char> ASICs,	int min_chip=0);

		//Read the CEC counter of the given slave
		int ReadCEC(unsigned char slave_addr, klaus_cec_data& result);

		void BeQuiet();

	private:

	// Low level function 
	// The variable of i2c inderface
	// are specified as private

		// Write block of data with specified register address
		int block_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, int length);
		// Read block of data with specified register address
		int block_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char *buf, int length);

		// Read block of data without specified register address
		int block_read(int length);

		int m_fd;
		unsigned char m_current_chipaddr;
		unsigned char* m_i2c_buf;
		int m_chunksize;

		bool m_python_mode;
		bool m_quiet;
		PyObject* pClass;
		PyObject* pClass_inst;
};


#endif
