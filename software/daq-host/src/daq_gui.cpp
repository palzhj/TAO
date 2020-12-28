

#include "TApplication.h"
#include "TGClient.h"
#include "DAQ_Main_Frame.h"

#include "HistogrammedResults.h"
#include "HistogramDAQ_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "TCanvas.h"
#include "TSystem.h"
#include "TStyle.h"

//SIGINT handler
#include <signal.h>

bool run=true;

void handler_sigint(int sig){
	printf("Caught SIGINT!\n");
	run=false;
}


int main(int argc, char **argv)
{
	TApplication *theApp = new TApplication("theApp", 0, 0);
	//signal(SIGINT,&handler_sigint);	
	if(argc<2)
	{
		printf("Usage: %s host [CHIPID==first]\n", argv[0]);
		return -1;
	}


	DAQ_Main_Frame *gui = new DAQ_Main_Frame(gClient->GetRoot(),200,200);
	gui->Set_HistDAQHost(argv[1]);
	if(argc>2)
		gui->Set_HistDAQASIC(atoi(argv[2]));

	//sleep(2);
	theApp->Run(kTRUE);
	return 0;
}
