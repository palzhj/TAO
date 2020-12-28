


#include "DAQctrl_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "TSystem.h"



int main(int argc, char **argv)
{
	if(argc<2)
	{
		printf("Usage: %s host \n", argv[0]);
		return -1;
	}

	std::list<unsigned char> ASICs;
	//get list of asics
	ASICs=DAQctrl(argv[1]).GetListOfASICs();

	for(std::list<unsigned char>::iterator it=ASICs.begin();it!=ASICs.end();++it){
		printf("ASIC: %d\n",*it);
	}
	return 0;
}
