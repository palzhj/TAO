//
//
//
//

#include "EventListDAQ_client.h"
#include "EventType.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "TFile.h"
#include "TTree.h"
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

//TApplication *theApp = new TApplication("theApp", 0, 0);
int main(int argc, char **argv) {
	signal(SIGINT,&handler_sigint);	
	if(argc<3)
	{
		printf("Usage: %s host filename.root [nAquisitions]\n", argv[0]);
		return -1;
	}
	//How many aquisitions before stop
	int nAquDo=-1;
	if(argc>3){
		nAquDo=atoi(argv[3]);
	}

	//output file
	TFile* fout;
	if(!(fout = TFile::Open(argv[2],"RECREATE")) || fout->IsZombie()){
		return -1;
	};
	//output tree
	TTree* tree=new TTree("aqu_dump","Aquisitions");
	klaus_aquisition* curr_aqu=NULL;
	TBranch* br=tree->Branch("aquisitions",&curr_aqu);

	TObjLink* curr_aqu_entry;

	//monitoring
	int nAqu=0;
	int nEventsTot=0;
	float nEventsMean=0;
	int nFailCntTot=0;
	float nFailCntMean=0;
	int lastAquID;


	//daq
	EventListDAQ* DAQ=new EventListDAQ(argv[1]);
	if(!DAQ->Good()){ printf("DAQ not connected, exiting.\n");return -1;}
	gStyle->SetOptStat(11111111);
	int n=0;
	DAQ->RegisterQueue(1000,1);
	//DAQ->ReadChipUntilEmpty(1);
	DAQ->ReadChipAsyncStart(0,0);
	sleep(1);
	while(run){
		TList* li=DAQ->FetchResults();
		if(li!=NULL){
			curr_aqu_entry=li->FirstLink();
			while(curr_aqu_entry){
				curr_aqu=(klaus_aquisition*)curr_aqu_entry->GetObject();
				nAqu++;
				nEventsTot+=curr_aqu->nEvents;
				nEventsMean=nEventsMean*0.99+curr_aqu->nEvents*0.01;
				nFailCntTot+=curr_aqu->failcnt;
				nFailCntMean=nFailCntMean*0.99+curr_aqu->failcnt*0.01;
				tree->Fill();
				curr_aqu_entry=curr_aqu_entry->Next();
			}
			printf("Total aquisitions: %d, Last ID=%d. Total Hits: %d, mean Hits/Aqu: %2.2f. Total failcnt %d, mean %2.3e\n",\
				nAqu,lastAquID,nEventsTot,nEventsMean,nFailCntTot,nFailCntMean);
		}
		sleep(1);
		if(nAqu && nAqu%60==0)
			tree->AutoSave("SaveSelf");
		if((nAquDo>0) && (nAqu>=nAquDo))
			break;
	}
	printf("Stopping thread\n");
	DAQ->ReadChipAsyncStop();
	sleep(1);
	curr_aqu=NULL;
	tree->ResetBranchAddress(br);
	tree->Write();
	fout->Close();
	return 0;
}

