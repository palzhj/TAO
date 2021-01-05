#include "HistogrammedResults.h"
#include "EventType.h"

HistogrammedResults::HistogrammedResults(){
	setPipe();  // initialize the pipelined ADC parameters

	h_channels=new TH1F(TString::Format("h_channels"),TString::Format("Channel histogram"),constants::CH_NUM+1,0,constants::CH_NUM+1);
	h_multi=new TH1F(TString::Format("h_multi"),TString::Format("Multipicity histogram"),300,0,300);
	for (int i=0;i<constants::CH_NUM;i++){
		//h_gainsel_evt[i]=new TH1F(TString::Format("h_gainsel_evt_CH%d",i),TString::Format("Channel %d: gainsel_evt",i), 1<<10,0,1<<10);
		h_ADC_10b[i]=new TH1F(TString::Format("h_ADC_10b_CH%d",i),TString::Format("Channel %d: ADC_10b",i), 1<<10,0,1<<10);
		//h_ADC_6b[i]=new TH1F(TString::Format("h_ADC_6b_CH%d",i),TString::Format("Channel %d: ADC_6b",i), 1<<6,0,1<<6);
		//h_ADC_PIPE[i]=new TH1F(TString::Format("h_ADC_PIPE_CH%d",i),TString::Format("Channel %d: ADC_PIPE",i), 1<<8,0,1<<8);
		h_ADC_12b[i]=new TH1F(TString::Format("h_ADC_12b_CH%d",i),TString::Format("Channel %d: ADC_12b",i), 1<<14,0,1<<14);
		h_CC[i]=new TH1F(TString::Format("h_T_CC_CH%d",i),TString::Format("Channel %d: T_CC",i), 1<<19,0,1<<19);
		h_MC[i]=new TH1F(TString::Format("h_T_MC_CH%d",i),TString::Format("Channel %d: T_MC",i), 1<<3,0,1<<3);
		h_FC[i]=new TH1F(TString::Format("h_T_FC_CH%d",i),TString::Format("Channel %d: T_FC",i), 1<<5,0,1<<5);
	}
	h_CC[constants::CH_NUM]=new TH1F(TString::Format("h_T_CC_T0"),TString::Format("Channel T0: time"), 1<<19,0,1<<19);
}

//Update names of histograms
void HistogrammedResults::SetPrefix(unsigned char ID){
	h_channels->SetNameTitle(TString::Format("h_channels_A%u",ID),TString::Format("Channel histogram ASIC %u",ID));
	h_multi->SetNameTitle(TString::Format("h_multi_A%u",ID),TString::Format("Multiplicity histogram ASIC %u",ID));
	for (int i=0;i<constants::CH_NUM;i++){
		//h_gainsel_evt[i]->SetNameTitle(TString::Format("h_gainsel_evt_A%u_CH%d",ID,i),TString::Format("ASIC %u Channel %d: gainsel_evt",ID,i));
		h_ADC_10b[i]->SetNameTitle(TString::Format("h_ADC_10b_A%u_CH%d",ID,i),TString::Format("ASIC %u Channel %d: ADC_10b",ID,i));
		//h_ADC_6b[i]->SetNameTitle(TString::Format("h_ADC_6b_A%u_CH%d",ID,i),TString::Format("ASIC %u Channel %d: ADC_6b",ID,i));
		//h_ADC_PIPE[i]->SetNameTitle(TString::Format("h_ADC_PIPE_A%u_CH%d",ID,i),TString::Format("ASIC %u Channel %d: ADC_PIPE",ID,i));
		h_ADC_12b[i]->SetNameTitle(TString::Format("h_ADC_12b_A%u_CH%d",ID,i),TString::Format("ASIC %u Channel %d: ADC_12b",ID,i));
		h_CC[i]->SetNameTitle(TString::Format("h_T_CC_A%u_CH%d",ID,i),TString::Format("ASIC %u Channel %d: T_CC",ID,i));
		h_MC[i]->SetNameTitle(TString::Format("h_T_MC_A%u_CH%d",ID,i),TString::Format("ASIC %u Channel %d: T_MC",ID,i));
		h_FC[i]->SetNameTitle(TString::Format("h_T_FC_A%u_CH%d",ID,i),TString::Format("ASIC %u Channel %d: T_FC",ID,i));
	}
	h_CC[constants::CH_NUM]->SetNameTitle(TString::Format("h_T_CC_A%u_T0",ID),TString::Format("ASIC %u Channel T0: time",ID));

}

HistogrammedResults::~HistogrammedResults(){
	for (int i=0;i<constants::CH_NUM;i++){
		//delete h_gainsel_evt[i];
		delete h_ADC_10b[i];
		//delete h_ADC_6b[i];
		//delete h_ADC_PIPE[i];
		delete h_ADC_12b[i];
		delete h_CC[i];
		delete h_MC[i];
		delete h_FC[i];
	}
	delete h_CC[constants::CH_NUM];
	delete h_channels;
	delete h_multi;
}

TCanvas* HistogrammedResults::Show(resultstype what, TCanvas* c){
	if(what<=SHOW_CH6){//channel selected. Show all histograms for this
		if(c==NULL){ c=new TCanvas(); c->Divide(3,3); }
		//c->cd(1);	h_gainsel_evt[what]->Draw();
		c->cd(3);	h_ADC_10b[what]->Draw();
		//c->cd(4);	h_ADC_6b[what]->Draw();
		//c->cd(5);	h_ADC_PIPE[what]->Draw();
		c->cd(6);	h_ADC_12b[what]->Draw();
		c->cd(7);	h_CC[what]->Draw();
		c->cd(8);	h_MC[what]->Draw();
		c->cd(9);	h_FC[what]->Draw();
	}else if(what==SHOW_CHT0){
		if(c==NULL){ c=new TCanvas(); }
		c->cd();
		h_CC[constants::CH_NUM]->Draw();  
	}else if(what<SHOW_CHANNELS){//specific histogram selected. Show all channels for this
		//build pads like channels
		if(c==NULL) c=ChannelPhysCanvas(c);

		for(int i=0;i<constants::CH_NUM;i++){
			c->cd(i+1);
			switch(what){
				//case SHOW_GAINSEL_EVT: h_gainsel_evt[i]->Draw(); break;
				case SHOW_ADC_10B: h_ADC_10b[i]->Draw(); break;
				//case SHOW_ADC_6B: h_ADC_6b[i]->Draw(); break;
				//case SHOW_ADC_PIPE: h_ADC_PIPE[i]->Draw(); break;
				case SHOW_ADC_12B: h_ADC_12b[i]->Draw(); break;
				case SHOW_CC: h_CC[i]->Draw(); break;
				case SHOW_MC: h_MC[i]->Draw(); break;
				case SHOW_FC: h_FC[i]->Draw(); break;
			}
		}
	}else if(what==SHOW_CHANNELS){//channel histo selected.
		if(c==NULL){
			c=new TCanvas();
		}
		c->cd();
		h_channels->Draw();
		
	}
}


//TODO: the hitmap multiplicity needs to be changed
TCanvas* HistogrammedResults::ChannelPhysCanvas(TCanvas* c){
	//		2
	//	5		1
	//		3
	//	6		0
	//		4
	if(c==NULL){c=new TCanvas;};
	double margin=0.01;
	double d=1./3.;
	for (int n=0;n<7;n++){
		double x1,x2,y1,y2;
		x1=margin;
		x2=d;
		y1=d*(1./2+n%2)+margin;
		y2=d*(3./2+n%2);
		if(n<5){
			x1=d+margin;
			x2=d*2;
			y1=d*(2-((n-2)%3))+margin;
			y2=d*(2-((n-2)%3)+1);
		}
		if(n<2){
			x1=d*2+margin;
			x2=d*3;
			y1=d*(1./2+n%2)+margin;
			y2=d*(3./2+n%2);
		}
		TString name(TString::Format("%s_%d",c->GetName(),n));
		c->cd();
		TPad* pad = new TPad(name,name,x1,y1,x2,y2,0);
		pad->SetNumber(1+n);
		pad->Draw();
	}
	return c;
}

void HistogrammedResults::Fill(klaus_event& evt){
	h_channels->Fill(evt.channel);
	if(evt.channel<=constants::CH_NUM){ 
                h_CC[evt.channel]->Fill(evt.T_CC); 
        }

	if(evt.channel>=constants::CH_NUM) return;
	//h_gainsel_evt[evt.channel]->Fill(evt.gainsel_evt);
	h_ADC_10b[evt.channel]->Fill(evt.ADC_10b);
	//h_ADC_6b[evt.channel]->Fill(evt.ADC_6b);
	//h_ADC_PIPE[evt.channel]->Fill(evt.ADC_PIPE);
	h_ADC_12b[evt.channel]->Fill(this->getCombinedResults(evt.channel, evt.ADC_6b, evt.ADC_PIPE));
	h_MC[evt.channel]->Fill(evt.T_MC);
	h_FC[evt.channel]->Fill(evt.T_FC);
}


void HistogrammedResults::Fill(std::list<klaus_event> list){
  for(std::list<klaus_event>::iterator it=list.begin();it!=list.end();it++)
    Fill(*it);
  h_multi->Fill(list.size());
}

void HistogrammedResults::Reset(){
	h_channels->Reset();
	h_multi->Reset();
	for (int i=0;i<constants::CH_NUM;i++){
		//h_gainsel_evt[i]->Reset();
		h_ADC_10b[i]->Reset();
		//h_ADC_6b[i]->Reset();
		//h_ADC_PIPE[i]->Reset();
		h_ADC_12b[i]->Reset();
		h_CC[i]->Reset();
		h_MC[i]->Reset();
		h_FC[i]->Reset();
	}
	h_CC[constants::CH_NUM]->Reset();
}

// set the offset[constants::CH_NUM][64] array
void HistogrammedResults::setPipe(){
    //printf("HistogrammedResults: Initialize the pipelined ADC reconstruction parameters\n");
    for(int i=0;i<constants::CH_NUM;i++){
        offset[i][0] = 0; offset[i][1] = 0;
        for(int j=2;j<64;j+=2){
            offset[i][j] = 256 + offset[i][j-1]; // initial it using the ideal number
            offset[i][j+1] = offset[i][j];
        }
    }
}

int HistogrammedResults::getCombinedResults(int channel, unsigned short ADC6b, unsigned short ADCPIPE){
        if((channel>constants::CH_NUM-1)|(channel<0)|(ADC6b<0)|ADC6b>63) return -1;
    return ADCPIPE + offset[channel][ADC6b];
}


void    HistogrammedResults::setSubRangeOffset(int channel, unsigned short ADC6b, int off){
    if((channel>constants::CH_NUM-1)|(channel<0)|(ADC6b<0)|ADC6b>63) return;
    printf("HistogrammedResults: Set the sub-range offset for range: %d to be %d for channel %d\n",ADC6b,off,channel);
    int start = (int)(ADC6b/2);
    offset[channel][start] = off; offset[channel][start+1] = off;
    for(int j=start+2;j<64;j=j+2){
        offset[channel][j] = offset[channel][j-1] + off;
        offset[channel][j+1] = offset[channel][j];
    }
}

void    HistogrammedResults::setAllRangeOffset(int channel, int* off){
    if((channel>constants::CH_NUM-1)|(channel<0)) return;
    printf("HistogrammedResults: Set the offset for all ranges for channel %d\n",channel);
    offset[channel][0] = 0; offset[channel][1] = 0;
    for(int j=2;j<64;j=j+2){
        offset[channel][j] = offset[channel][j-1] + off[j];
        offset[channel][j+1] = offset[channel][j];
    }
}
