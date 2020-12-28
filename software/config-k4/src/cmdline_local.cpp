

#include "Klaus4Config.h"
#include "PatternMerger.h"
#include "LinuxSPIdevInterface.h"

//#include <unistd.h>
//#include <iostream>

#define NCHIPS 1

int main (int argc, char *argv[])
{

	if (argc<2){
		printf("Usage: ./cmdline_config <ConfigBaseFilename> [Chip Number]\n");
		return -1;
	}




	TLinuxSPIdevInterface iface("/dev/spidev1.1", 500000);
	iface.SetTimeout(1,1000000,0);


	TPatternMerger merger(&iface);
	
	TKLauS4Config *configurations[NCHIPS];
	char filename[256];
	for (int i=0;i<NCHIPS;i++){
		configurations[i]=new TKLauS4Config(&merger);

		sprintf(filename,"Config_CHIP%d.txt",i);
		configurations[i]->ReadFromFile(filename);
	}






	//The chip that should be configured
	int chip;
	if (argc == 3)
		chip=atoi(argv[2]);
		if ( chip >= NCHIPS ){
			printf("The specified chip does not exist in the current setup!");
			chip=NCHIPS-1;
		}else
	configurations[chip]->UpdateConfig();
	configurations[chip]->UpdateConfig();

	for (int i=0;i<NCHIPS;i++){
		delete configurations[i];
	}
	return 0;
}
