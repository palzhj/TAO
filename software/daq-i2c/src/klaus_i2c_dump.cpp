//
//
//
//

#include "EventType.h"
#include "klaus_i2c_iface.h"

#include "TApplication.h"
#include "TObject.h"
#include "TH1.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TRandom.h"
#include "TThread.h"
#include "TTree.h"
#include "TFile.h"
#include "TStyle.h"

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;


//SIGINT handler
#include <signal.h>
TFile* fout2;
TTree* tree2;
void handler_sigint(int sig){
	printf("Caught SIGINT!\n");
	tree2->Write();
	fout2->Close();
	TThread::Ps();
	exit(0);
}


int main(int argc, char **argv)
{
	signal(SIGINT,&handler_sigint);
	
	klaus_event*  current_event=NULL;
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

	TFile* fout = new TFile(filename,"recreate");	// open a output file to store the data
	TTree* tree = new TTree("dump","dumptree");
	fout2=fout;
	tree2=tree;


	tree->Branch("klaus_event",&current_event);
	tree->Branch("voltage",&voltage,"voltage/D");

	unsigned int n_total=0;
	unsigned int n_read;
	unsigned int n_cycles=0;

//	klaus_event::PrintHeader();
	while (n_total<nevents)
	{
		events.clear();
		//n_read = i2c_iface.ReadEvents(10, events);
		n_read = i2c_iface.ReadEvents(atoi(argv[2]),10, events,0);
		n_cycles++;
		n_total+=n_read;
		//if(n_read>0)  klaus_event::PrintHeader();
		for(std::list<klaus_event>::iterator it=events.begin();it!=events.end();++it)
		{
			current_event = &(*it);
			current_event->Print();
			tree->Fill();
		}
		if(n_total%1000 == 1 ) printf("%u Events read\n",n_total);
	}
	printf("Finished reading %u events after %u cycles\n",n_read,n_cycles);
	fout->Write();
	fout->Close();
	printf("The END...\n");

	return 0;
}

