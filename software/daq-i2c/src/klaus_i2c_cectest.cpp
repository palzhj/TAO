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



int main(int argc, char **argv)
{
	klaus_event*  current_event=NULL;
	std::list<klaus_event> events;
	int n_read;

	// initialize the i2c inteface
	klaus_i2c_iface i2c_iface;//("/dev/i2c-1");
	i2c_iface.SetSlaveAddr(0x40);
/*
	for(int i=0; i<5 ;i++){
		printf("Reading some events...\n");
		klaus_event::PrintHeader();
		n_read = i2c_iface.ReadEvents(0x40,10,events,0);
		for(auto h:events) h.Print();
		printf("--> %d read\n",n_read);
		events.clear();
	}
*/
	klaus_cec_data data,sum;
	sum.Clear();
//	for(int i=0;i<500;i++){
		i2c_iface.ReadCEC(0x40, data);
		//data.PrintTransposed();
		sum.Add(&data);
		sum.Print();
//		sum.PrintTransposed();
//	}
	return 0;
}

