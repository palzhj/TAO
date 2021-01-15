#include "TApplication.h"
#include "TGClient.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TRandom.h"
#include "TGButton.h"
#include "TGButtonGroup.h"
#include "TRootEmbeddedCanvas.h"
#include "DAQ_Main_Frame_local.h"
#include "TFile.h"
#include "HistogrammedResults.h"
//#include "HistogramDAQ_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "TCanvas.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TFile.h"
#include "TString.h"
#include "TH1F.h"

float xi[]={2., 5., 0.5, 3.5, 6.5, 2., 5.};
float xf[]={5., 8., 3.5, 6.5, 9.5, 5 , 8.};
float yi[]={1., 1 , 4. , 4. , 4. , 7., 7.};
float yf[]={4., 4., 7. , 7. , 7. , 10., 10.};
int tileMap[]={0,1,4,3,2,6,5};

void DAQ_Main_Frame_local::Update_Canvas(){
	if(histDAQ==NULL){return;};	
	if(stop==0){
		histDAQ->FetchResults();
		if(histDAQ->GetResults() == NULL) return;
		if(type==1){ 	    // ADC_10b 
			if(channel>constants::CH_NUM-1){std::cout<<"Attention: not so many channels"<<std::endl;return;}
			histDAQ->GetResults()->h_ADC_10b[channel]->Draw();
			if(useDefaultRange==false){ histDAQ->GetResults()->h_ADC_10b[channel]->GetXaxis()->SetRangeUser(xRangeMin,xRangeMax); }
		}
		else if(type==2){   // ADC_12b 
			if(channel>constants::CH_NUM-1){std::cout<<"Attention: not so many channels"<<std::endl;return;}
			histDAQ->GetResults()->h_ADC_12b[channel]->Draw();
			if(useDefaultRange==false){ histDAQ->GetResults()->h_ADC_12b[channel]->GetXaxis()->SetRangeUser(xRangeMin,xRangeMax); }
		}
		else if(type==3){   // TDC
			if(channel>constants::CH_NUM){std::cout<<"Attention: not so many channels"<<std::endl;return;}
			histDAQ->GetResults()->h_CC[channel]->Draw();
			if(useDefaultRange==false){ histDAQ->GetResults()->h_CC[channel]->GetXaxis()->SetRangeUser(xRangeMin,xRangeMax); }
		}
		else if(type==4){   // Channel
			histDAQ->GetResults()->h_channels->Draw();
		}
	//	else if(type==5){   // ADC_PIPE
	//		if(channel>constants::CH_NUM-1){std::cout<<"Attention: not so many channels"<<std::endl;return;}
	//		histDAQ->GetResults()->h_ADC_PIPE[channel]->Draw();
	//		if(useDefaultRange==false){ histDAQ->GetResults()->h_ADC_PIPE[channel]->GetXaxis()->SetRangeUser(xRangeMin,xRangeMax); }
	//	}
	//	else if(type==6){   // ADC_6b
	//		if(channel>constants::CH_NUM-1){std::cout<<"Attention: not so many channels"<<std::endl;return;}
	//		histDAQ->GetResults()->h_ADC_6b[channel]->Draw();
	//		if(useDefaultRange==false){ histDAQ->GetResults()->h_ADC_6b[channel]->GetXaxis()->SetRangeUser(xRangeMin,xRangeMax); }
	//	}
                // TODO: change the hit-map
		else if(type==7){   // Channel MAP 
		 	 h_map->Reset();
		  	for(int ch=0;ch<7;ch++){
		    		for(float x=xi[tileMap[ch]];x<xf[tileMap[ch]];x+=0.5){
		      			for(float y=yi[tileMap[ch]];y<yf[tileMap[ch]];y+=0.5){
						h_map->Fill(x,y,histDAQ->GetResults()->h_channels->GetBinContent(ch+1));
		      			}
		    		}
		  	}
			h_map->Draw("COLZ");
		}
		else if(type==8){ // Channel Multiplicity -> T_MC
			if(channel>constants::CH_NUM){std::cout<<"Attention: not so many channels"<<std::endl;return;}
			histDAQ->GetResults()->h_MC[channel]->Draw();
			if(useDefaultRange==false){ histDAQ->GetResults()->h_MC[channel]->GetXaxis()->SetRangeUser(xRangeMin,xRangeMax); }
		}
		else if(type==9){ // Channel Multiplicity -> T_FC
			if(channel>constants::CH_NUM){std::cout<<"Attention: not so many channels"<<std::endl;return;}
			histDAQ->GetResults()->h_FC[channel]->Draw();
			if(useDefaultRange==false){ histDAQ->GetResults()->h_FC[channel]->GetXaxis()->SetRangeUser(xRangeMin,xRangeMax); }
		}

		(fEcanvas->GetCanvas())->Update();
	}
	else std::cout<<"NOTICE: DAQ is STOPPED!"<<std::endl;

}


void DAQ_Main_Frame_local::Set_HistDAQHost(){
	//histDAQ = new DAQServ(i2c_iface);
	//histDAQ = new HistogramDAQ(server,9090);
	//if(!histDAQ->Good()){ printf("DAQ not connected, exiting.\n");return;}
	histDAQ->AppendASICList(0);
        //histDAQ->AppendASICList(1);
        //histDAQ->AppendASICList(2);
        //histDAQ->AppendASICList(3);
	//histDAQ->FetchResults();
}

void	DAQ_Main_Frame_local::Set_HistDAQASIC(unsigned char ASIC){ 
	histDAQ->BindTo(ASIC);
	chip_id = ASIC; 
}

void DAQ_Main_Frame_local::DoReset(){ histDAQ->ResetResults(); usleep(0); }

DAQ_Main_Frame_local::DAQ_Main_Frame_local(klaus_i2c_iface& i2c_iface, const TGWindow *p, UInt_t w, UInt_t h)
	:TGMainFrame(p,w,h,kHorizontalFrame){

	histDAQ=new DAQServ(i2c_iface);
	channel = 0; type = 4; stop = 0; 
	xRangeMax = 4096; xRangeMin = 0;
	useDefaultRange = true;
	SetCleanup(kDeepCleanup);
	
	// make sure that the DNL_COR.root is stored into the same directory as the exectuable program
	//f_cal = new TFile("DNL_COR.root");

	// TCanvas on which the picture draw
	fEcanvas = new TRootEmbeddedCanvas("Ecanvas",this);
	fEcanvas->Resize(600,400);
	AddFrame(fEcanvas,new TGLayoutHints(kLHintsExpandX|kLHintsExpandY, 10, 10, 10, 1));

	// Controls on right
	TGVerticalFrame *controls = new TGVerticalFrame(this,50,400);
	AddFrame(controls, new TGLayoutHints(kLHintsRight | kLHintsExpandY));
	// controls datatype
	TGButtonGroup *datatype = new TGButtonGroup(controls,"Data Type");
	datatype->SetTitlePos(TGGroupFrame::kCenter);
	new	TGRadioButton(datatype,"ADC_10b",1);
	new	TGRadioButton(datatype,"ADC_12b",2);
	new	TGRadioButton(datatype,"TDC_CC",3);
	new	TGRadioButton(datatype,"Channel",4);
	//new	TGRadioButton(datatype,"ADC_PIPE",5);
	//new	TGRadioButton(datatype,"ADC_6b",6);
	//new	TGRadioButton(datatype,"HIT_MAP",7);
	//new	TGRadioButton(datatype,"Multiplicity",8);
	new	TGRadioButton(datatype,"TDC_MC",8);
	new	TGRadioButton(datatype,"TDC_FC",9);
	datatype->SetButton(kTextCenterX);
	datatype->Connect("Pressed(Int_t)","DAQ_Main_Frame_local",this, "DoDataType(Int_t)");
	controls->AddFrame(datatype,new TGLayoutHints(kLHintsExpandX));

	// channel select
	fNum = new TGNumberEntry(controls,0,7,999,
			TGNumberEntry::kNESInteger,
			TGNumberEntry::kNEANonNegative,
			TGNumberEntry::kNELLimitMinMax,0,constants::CH_NUM);
	fNum->Connect("ValueSet(Long_t)","DAQ_Main_Frame_local",this, "DoSetChannel()");
	(fNum->GetNumberEntry())->Connect("ReturnPressed()","DAQ_Main_Frame_local",this,"DoSetChannel()");
	controls->AddFrame(fNum, new TGLayoutHints(kLHintsExpandX));
	
	// reset button
	TGTextButton *reset = new TGTextButton(controls,"&Reset");
	reset->Connect("Clicked()","DAQ_Main_Frame_local",this,"DoReset()");
	controls->AddFrame(reset, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 5));
	
	//stop button
	TGTextButton *sstop = new TGTextButton(controls,"&Stop");
	sstop->Connect("Clicked()","DAQ_Main_Frame_local",this,"DoStop()");
	controls->AddFrame(sstop, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 5));
	
	// save button
	TGTextButton *save = new TGTextButton(controls,"&Save");
	save->Connect("Clicked()","DAQ_Main_Frame_local",this,"DoSave()");
	controls->AddFrame(save, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 5));

	// text entries to set the RangeUser
	TGHorizontalFrame *rangeSelect = new TGHorizontalFrame(controls);
	// xMin
	fXMin = new TGNumberEntry(rangeSelect,0,7,999, TGNumberEntry::kNESInteger, TGNumberEntry::kNEANonNegative, TGNumberEntry::kNELLimitMinMax,0,1<<16);
	fXMin->Connect("ValueSet(Long_t)","DAQ_Main_Frame_local",this, "DoChangeXMin()");
	(fXMin->GetNumberEntry())->Connect("ReturnPressed()","DAQ_Main_Frame_local",this,"DoChangeXMin()");
	rangeSelect->AddFrame(fXMin, new TGLayoutHints(kLHintsExpandX));
	// xMax
	fXMax = new TGNumberEntry(rangeSelect,4095,7,999, TGNumberEntry::kNESInteger, TGNumberEntry::kNEANonNegative, TGNumberEntry::kNELLimitMinMax,0,1<<16);
	fXMax->Connect("ValueSet(Long_t)","DAQ_Main_Frame_local",this, "DoChangeXMax()");
	(fXMin->GetNumberEntry())->Connect("ReturnPressed()","DAQ_Main_Frame_local",this,"DoChangeXMax()");
	rangeSelect->AddFrame(fXMax, new TGLayoutHints(kLHintsExpandX));

	controls->AddFrame(rangeSelect, new TGLayoutHints(kLHintsExpandX));
	
	// setRangeUser
	TGTextButton *setRange = new TGTextButton(controls,"&SetRangeUser");
	setRange->Connect("Clicked()","DAQ_Main_Frame_local",this,"DoSetRangeUser()");
	controls->AddFrame(setRange, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 5));

	// Set the Pipeline ADC branch displacement
//        useDefaultPipe = true; pipeBranch = 44; pipeBranchDis = 128;
//	TGHorizontalFrame *pipeSet = new TGHorizontalFrame(controls);
	// pipeBranch
//	fpipeBranch = new TGNumberEntry(pipeSet,44,7,999, TGNumberEntry::kNESInteger, TGNumberEntry::kNEANonNegative, TGNumberEntry::kNELLimitMinMax,0,63);
//	fpipeBranch->Connect("ValueSet(Long_t)","DAQ_Main_Frame_local",this, "DoSetPipeBranch()");
//	(fpipeBranch->GetNumberEntry())->Connect("ReturnPressed()","DAQ_Main_Frame_local",this,"DoSetPipeBranch()");
//	pipeSet->AddFrame(fpipeBranch, new TGLayoutHints(kLHintsExpandX));
	// pipeBranchDistance
//	fpipeBranchDis = new TGNumberEntry(pipeSet,128,7,999, TGNumberEntry::kNESInteger, TGNumberEntry::kNEANonNegative, TGNumberEntry::kNELLimitMinMax,100,150);
//	fpipeBranchDis->Connect("ValueSet(Long_t)","DAQ_Main_Frame_local",this, "DoSetPipeBranchDis()");
//	(fpipeBranchDis->GetNumberEntry())->Connect("ReturnPressed()","DAQ_Main_Frame_local",this,"DoSetPipeBranchDis()");
//	pipeSet->AddFrame(fpipeBranchDis, new TGLayoutHints(kLHintsExpandX));

//	controls->AddFrame(pipeSet, new TGLayoutHints(kLHintsExpandX));
	
	// setRangeUser
//	TGTextButton *setPipe = new TGTextButton(controls,"&SetPipeline");
//	setPipe->Connect("Clicked()","DAQ_Main_Frame_local",this,"DoSetPipe()");
//	controls->AddFrame(setPipe, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 5));


	// exit button
	TGTextButton *exit = new TGTextButton(controls,"&Exit", "gApplication->Terminate(0)");
	controls->AddFrame(exit, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 0, 0, 0, 5));

	SetWindowName("DAQ GUI");
       	MapSubwindows(); Resize(this->GetDefaultSize()); MapWindow();

	//Start the Canvas update timer
	m_CanvasUpdateTimer = new TTimer();
	m_CanvasUpdateTimer->Connect("Timeout()","DAQ_Main_Frame_local",this,"Update_Canvas()");
	m_CanvasUpdateTimer->Start(500,kFALSE);

	TCanvas *fCanvas = fEcanvas->GetCanvas();
	fCanvas->cd();
	fCanvas->Update();

	h_map = new TH2F("hitMap","hitMap;x;y",22,0,11,22,0,11);
}

void DAQ_Main_Frame_local::DoChangeXMax(){ xRangeMax = fXMax->GetNumberEntry()->GetIntNumber(); }
void DAQ_Main_Frame_local::DoChangeXMin(){ xRangeMin = fXMin->GetNumberEntry()->GetIntNumber(); }
void DAQ_Main_Frame_local::DoSetRangeUser(){ useDefaultRange = false; printf("Use Range: xMin = %d, xMax = %d\n",xRangeMin,xRangeMax); }
void DAQ_Main_Frame_local::DoSetPipeBranch(){ pipeBranch = fpipeBranch->GetNumberEntry()->GetIntNumber();}
void DAQ_Main_Frame_local::DoSetPipeBranchDis(){ pipeBranchDis = fpipeBranchDis->GetNumberEntry()->GetIntNumber();}
void DAQ_Main_Frame_local::DoSetPipe(){ 
    useDefaultPipe=false;
    printf("Pipeline ADC: Distance between branch %d and %d is %d",2*(int(pipeBranch/2)),2*(int(pipeBranch)/2)-1,pipeBranchDis);
    // interaction with HigstogrammedResults
    histDAQ->UpdateParams_12b(channel,pipeBranch,pipeBranchDis);
    histDAQ->ResetResults();        // 
}

void DAQ_Main_Frame_local::DoSave(){
	TFile* f = new TFile("dump.root","recreate");
	histDAQ->GetResults()->Write("dump");
	f->Write(); f->Close();	
}

void DAQ_Main_Frame_local::DoSetChannel(){
	channel = fNum->GetNumberEntry()->GetIntNumber();
	std::cout<<"Select the Channel\t"<<channel<<std::endl;
}

void DAQ_Main_Frame_local::DoDataType(Int_t id){
	useDefaultRange = true;
	type = id;
	if(type==1) std::cout<<"Select the Data Type to be\t\t ADC_10b"<<std::endl;
	else if(type==2) std::cout<<"Select the Data Type to be\t\t ADC_12b"<<std::endl;
	else if(type==3) std::cout<<"Select the Data Type to be\t\t TDC_CC"<<std::endl;
	else if(type==4) std::cout<<"Select the Data Type to be\t\t Channel"<<std::endl;
	//else if(type==5) std::cout<<"Select the Data Type to be\t\t ADC_PIPE"<<std::endl;
	//else if(type==6) std::cout<<"Select the Data Type to be\t\t ADC_6b"<<std::endl;
	//else if(type==7) std::cout<<"Select the Data Type to be\t\t HIT_MAP"<<std::endl;
	//else if(type==8) std::cout<<"Select the Data Type to be\t\t Multiplicity"<<std::endl;
	else if(type==8) std::cout<<"Select the Data Type to be\t\t TDC_MC"<<std::endl;
	else if(type==9) std::cout<<"Select the Data Type to be\t\t TDC_FC"<<std::endl;
	else std::cout<<"Invalid!\t id=\t"<<id<<std::endl;
	
}

void DAQ_Main_Frame_local::DoStop(){
	if(stop==0){ stop = 1;std::cout<<"DAQ SWITCH TO OFF"<<std::endl;}
	else {stop = 0;std::cout<<"DAQ SWITCH TO ON"<<std::endl;}
}


DAQ_Main_Frame_local::~DAQ_Main_Frame_local(){
	//f_cal->Close();
       	Cleanup();
}

ClassImp (DAQ_Main_Frame_local);
