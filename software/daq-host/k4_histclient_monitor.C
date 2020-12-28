#include "TCanvas.h"
#include "unistd.h"

void k4_histclient_monitor(int channel=7){
	stop=false;
	//gSystem->Load("HistogramDAQ_client.so");
	//daq
	Spy* histDAQ=new Spy("ropi4k2");
	if(!histDAQ->Good()){ printf("DAQ not connected, exiting.\n");return;}
	histDAQ->ResetResults();
	histDAQ->FetchResults();
	TCanvas* cmon;
	cmon=new TCanvas();
	TFile* file = new TFile("test.root","recreate");
	int n=0;
	//while(!stop){
	while(1){
		histDAQ->ReadChipUntilEmpty(0);
		if(n%2==1){
			histDAQ->FetchResults();
			//histDAQ->GetResults()->h_ADC_PIPE[channel]->Draw();
			histDAQ->GetResults()->h_time[channel]->Draw();
			//histDAQ->GetResults()->h_channels->Draw();
			cmon->Update();
			cmon->Modified();
		}
		n++;
	}
	file->Write();
	file->Close();
	printf("The END...\n");
}

