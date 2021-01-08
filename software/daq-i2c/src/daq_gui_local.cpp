

#include "TApplication.h"
#include "TGClient.h"
#include "DAQ_Main_Frame_local.h"

#include "klaus_i2c_iface.h"
#include "HistogrammedResults.h"
//#include "HistogramDAQ_client.h"
#include "HistogramDAQ_server.h"
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
	if(argc<1)
	{
		printf("Usage: %s [CHIPID==first]\n", argv[0]);
		return -1;
	}

	klaus_i2c_iface i2c_iface;
        i2c_iface.SetChunksize(40);

	DAQ_Main_Frame_local *gui = new DAQ_Main_Frame_local(i2c_iface, gClient->GetRoot(),200,200);

        //daq
        //DAQServ histDAQ(i2c_iface);

	gui->Set_HistDAQHost();
	gui->Set_HistDAQASIC(atoi(argv[1]));

	//sleep(2);
	theApp->Run(kTRUE);
	return 0;
}
