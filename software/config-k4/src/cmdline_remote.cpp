

#include "Klaus4Config.h"
#include "VCRemoteClient.h"

//#include <unistd.h>
//#include <iostream>

#define NCHIPS 1

int main (int argc, char *argv[])
{

	if (argc<3){
		std::cerr << "Usage: gui_remote <host> <ConfigBaseFilename>\n";
		return -1;
	}


	TKLauS4Config *configurations[NCHIPS];
	char filename[256];
	for (int i=0;i<NCHIPS;i++){
		configurations[i]=new VCRemoteClient<TKLauS4Config>(argv[1],1);
		sprintf(filename,"%s%d.txt",argv[2],i);
		printf("Reading file %s\n",filename);
		configurations[i]->ReadFromFile(filename);
	}

	configurations[0]->UpdateConfig();
	configurations[0]->UpdateConfig();

	for (int i=0;i<NCHIPS;i++){
		delete configurations[i];
	}
	return 0;
}
