//
//
//
//

#include "HistogramDAQ_server.h"
#include "EventType.h"
#include "klaus_i2c_iface.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

//#define TEST

//SIGINT handler
#include <signal.h>
void handler_sigint(int sig){
	printf("Caught SIGINT!\n");
	exit(0);
}


int main(int argc, char **argv)
{
	signal(SIGINT,&handler_sigint);
	
	if(argc<2)
	{
		printf("Usage: %s /dev/i2c-x [configHost=localhost]\n", argv[0]);
		return -1;
	}

	// initialize the i2c inteface
	klaus_i2c_iface i2c_iface(argv[1]);
	i2c_iface.SetChunksize(40);
	//daq
	DAQServ histDAQ(i2c_iface);

#ifdef TEST
	histDAQ.AppendASICList(0x20);	
	histDAQ.AppendASICList(0x21);	
	histDAQ.AppendASICList(0x22);	
#else
	if(argc>2)
		histDAQ.AutoFetchASICList(argv[2]);
	else
		histDAQ.AutoFetchASICList("localhost");
#endif
	histDAQ.Run();
	return 0;
}

