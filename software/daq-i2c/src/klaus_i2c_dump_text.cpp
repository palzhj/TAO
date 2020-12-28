#include "EventType.h"
#include "klaus_i2c_iface.h"

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;


int main(int argc, char **argv)
{
	
	klaus_event*  current_event;
	std::list<klaus_event> events;
	int nevents;
	if(argc<6)
	{
		printf("Usage: %s [/dev/i2c-x] [dev_addr] [nevents] [voltage] [output_filename]\n", argv[0]);
		return -1;
	}

	// initialize the i2c inteface
	klaus_i2c_iface i2c_iface(argv[1]);
	i2c_iface.SetSlaveAddr(atoi(argv[2]));
	
	
	// if the interface is opened correctly
	
	// get the number of events to read
	nevents = atoi(argv[3]);		
	double voltage;
	sscanf(argv[4],"%le",&voltage);

	char* filename=argv[5];

	printf("Reading %u events @ V=%e, fout=%s\n",nevents,voltage,filename);

	FILE* fout=fopen(filename,"w");
		if(fout<0){
			printf("Error while opening the file\n");
		return -1;
	}

	unsigned int n_total=0;
	unsigned int n_read;
	unsigned int n_cycles=0;

	klaus_event::PrintHeader();
	//klaus_event::PrintHeader(fout);
	while (n_total<nevents)
	{
		events.clear();
		n_read = i2c_iface.ReadEvents(10, events);
		n_cycles++;
		n_total+=n_read;
		if(n_read>0)  klaus_event::PrintHeader();
		for(std::list<klaus_event>::iterator it=events.begin();it!=events.end();++it)
		{
			current_event = &(*it);
			current_event->Print();
			current_event->Print(fout);
		}
		if(n_total%1000 == 1 ) printf("%u Events read\n",n_total);
	}



	fclose(fout);
	printf("Finished reading %u events after %u cycles\n",n_read,n_cycles);
	printf("The END...\n");
	return 0;
}

