//
//
//
//

#include "EventListDAQ_client.h"
#include "EventType.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "TCanvas.h"
#include "TApplication.h"
#include <TSystem.h>
#include <TStyle.h>

//SIGINT handler
#include <signal.h>

bool run=true;

void handler_sigint(int sig){
	printf("Caught SIGINT!\n");
	run=false;
}

TApplication *theApp = new TApplication("theApp", 0, 0);
int main(int argc, char **argv)
{
	signal(SIGINT,&handler_sigint);	
	if(argc<2)
	{
		printf("Usage: %s [host]\n", argv[0]);
		return -1;
	}
	int channel=7;

	//daq
	EventListDAQ* DAQ=new EventListDAQ(argv[1]);
	if(!DAQ->Good()){ printf("DAQ not connected, exiting.\n");return -1;}
//	TCanvas* cmon;
//	cmon=new TCanvas();
	gStyle->SetOptStat(11111111);
	int n=0;
	DAQ->RegisterQueue(10000,1);
	//DAQ->ReadChipUntilEmpty(1);
	DAQ->ReadChipAsyncStart(0,0);
	while(run){
		TList* li=DAQ->FetchResults();
		//if(li!=NULL && li->GetSize()!=0)
		//	((klaus4_aquisition*)li->First())->Print();
		if(li){
			//li->Dump();
			li->Clear();
		}
		sleep(1);
	}
	DAQ->ReadChipAsyncStop();

	return 0;
}

