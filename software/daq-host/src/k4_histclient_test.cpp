//
//
//
//

#include "HistogrammedResults.h"
#include "HistogramDAQ_client.h"
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
	HistogramDAQ* histDAQ=new HistogramDAQ(argv[1]);
	if(!histDAQ->Good()){ printf("DAQ not connected, exiting.\n");return -1;}
	histDAQ->ResetResults();
	histDAQ->FetchResults();
	TCanvas* cmon;
	cmon=new TCanvas();
	gStyle->SetOptStat(11111111);
	int n=0;
	while(run){
		histDAQ->ReadChipUntilEmpty(1);
		if(n%2==1){
			histDAQ->FetchResults();
			//histDAQ->GetResults()->h_ADC_PIPE[channel]->Draw();
			histDAQ->GetResults()->h_time[channel]->GetXaxis()->SetRangeUser(0,50);
			histDAQ->GetResults()->h_time[channel]->Draw();
			//histDAQ->GetResults()->h_channels->Draw();
			cmon->Update();
			cmon->Modified();
                        gSystem->ProcessEvents();
		}
		if(n%10==1){
			histDAQ->ResetResults();
		}
		n++;
	}
	printf("The END...\n");

	return 0;
}

