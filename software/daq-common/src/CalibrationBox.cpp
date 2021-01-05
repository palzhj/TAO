
// Desc: Time calibration of mutrig based data:
// 	 Time alignment (coarse on the fly, finetime shifts), DNL correction
// Author: K. Briggl
#include "CalibrationBox.h"
#include <exception>
#include <cmath>
#include "EventType.h"
#include "TMath.h"
#include <cassert>
//empty box
CalibrationBox::CalibrationBox(){
	m_tick_period=1;
};

//Build new calibration box and reserve space for N channels. Added: Possibility to calibrate differnet counters with differnet ammount of bits (FC: 5Bits, MC: 2 Bits, CC: 12)
CalibrationBox::CalibrationBox(int nChannels,double f_ref, int nBits)
{
	m_nBits=nBits;
	m_nBins = 1<<nBits;
	m_DNLcomputed=false;
	m_tick_period=1./f_ref;
	m_CC_offsets.resize(nChannels,0);
	m_finetime_offsets.resize(nChannels,0);
	m_code_counts_sum.resize(nChannels,0);
	m_code_counts.resize(nChannels);
	m_code_times.resize(nChannels);
	for(int i=0;i<nChannels;i++){
		m_code_counts[i].resize(m_nBins);	
		m_code_times[i].resize(m_nBins);	
		for(int j=0;j<m_nBins;j++){
			m_code_counts[i][j]=0;	//empty histograms
			//just make linear as a start, in case user does not wait or initialize before
			m_code_times[i][j]=j*1./m_nBins;
		}
	}
}


CalibrationBox::~CalibrationBox(){}

void CalibrationBox::Reset(){
	for(size_t i=0;i<m_code_counts.size();i++){
		m_code_counts_sum[i]=0;
		for(int j=0;j<m_nBins;j++){
			m_code_counts[i][j]=0;
		}
	}
}


void CalibrationBox::SetFref(double f_ref){m_tick_period=1./f_ref;}
void CalibrationBox::SetTtick(double period){m_tick_period=period;}
double CalibrationBox::GetTtick(){return m_tick_period;}

void CalibrationBox::AddHit(klaus_event& thehit){
	
//	m_code_counts[thehit.channel].resize(m_nBins);
//	printf("Resizing m_code_counts to %i Bins\n",m_nBins);
//	printf("Counts %i \n", thehit.GetTime()%m_nBins);
	m_code_counts[thehit.channel][thehit.GetTime()%m_nBins]++;
//	printf("channel %i\n",thehit.channel);
//	m_code_counts_sum.resize(thehit.channel);
//	printf("Resizing m_code_counts_sum to %i Bins\n",thehit.channel);
	m_code_counts_sum[thehit.channel]++;
//	printf("counts_sum %i \n",m_code_counts_sum[thehit.channel]);
}

void CalibrationBox::AddHits(klaus_acquisition& event){
	for(auto hitlist: event.data)
		for(auto hit: hitlist.second)
			AddHit(hit);
	
	
}

void CalibrationBox::SetHitCounts(CalibrationBox& other){
	if(other.GetNChannels() != this->GetNChannels()){printf("CalibrationBox::SetHitCounts(): Mismatch in channel number, can not load data.\n"); return;}
	if(other.m_tick_period != m_tick_period){printf("CalibrationBox::SetHitCounts(): Mismatch in reference frequencies, can not load data.\n"); return;}
	m_code_counts_sum=other.m_code_counts_sum;
	m_code_counts=other.m_code_counts;
	RecomputeDNL();
}
int CalibrationBox::RecomputeDNL(){
	m_code_times.resize(m_code_counts.size());
	for(size_t ch=0;ch<m_code_times.size();ch++){
		if(m_code_counts_sum[ch]==0) continue; //ignore empty/unused channels
		double time=0; //integrated time of bin upper edge (CC units)
		for(int fbin=0;fbin<m_nBins;fbin++){
			//relative bin width, in units of coarse counts (periods of m_tick_period)
			double codew=m_code_counts[ch][fbin]*1./m_code_counts_sum[ch]*m_nBins;
			//the bin time is given at the centor of the bin, i.e. last bin upper edge + 0.5 * bin width
			m_code_times[ch][fbin]=time+codew;
			//next time
			time+=codew;
			printf("For channel %lu: bin %u: Counts=%d, sum=%lu codew=%2.5lf, time=%2.2lf\n",ch,fbin,m_code_counts[ch][fbin],m_code_counts_sum[ch],codew,time);
		}
	}
	m_DNLcomputed=true;
	return 0;
}

	
//Merge data from other into this, appending information at the end.
void CalibrationBox::Append(CalibrationBox& other){
	m_CC_offsets.insert(m_CC_offsets.begin(),other.m_CC_offsets.begin(),other.m_CC_offsets.end());
	m_finetime_offsets.insert(m_finetime_offsets.begin(),other.m_finetime_offsets.begin(),other.m_finetime_offsets.end());
	m_code_counts_sum.insert(m_code_counts_sum.begin(),other.m_code_counts_sum.begin(),other.m_code_counts_sum.end());
	m_code_counts.insert(m_code_counts.begin(),other.m_code_counts.begin(),other.m_code_counts.end());
	m_code_times.insert(m_code_times.begin(),other.m_code_times.begin(),other.m_code_times.end());
}

//Merge data from other into this, combining the count values. Offset values are not touched. NL maps are recomputed
void CalibrationBox::Merge(CalibrationBox& other){
	//Consistency check
	if(m_CC_offsets.size()!=other.m_CC_offsets.size()){throw std::out_of_range("CalibrationBox::Merge(): size check failed");}

	for(size_t i=0;i<m_CC_offsets.size();i++){
		//m_CC_offsets[i]+=other.m_CC_offsets[i];
		//m_finetime_offsets[i]+=other.m_finetime_offsets[i];
		m_code_counts_sum[i]+=other.m_code_counts_sum[i];
		for(size_t j=0;j<m_nBins;j++)
			m_code_counts[i][j]+=other.m_code_counts[i][j];
	}
	RecomputeDNL();
}
	

// Set&Get time shift values
void CalibrationBox::SetTimeShift(int channel, float value){m_finetime_offsets[channel]=value;}
double CalibrationBox::GetTimeShift(int channel){return m_finetime_offsets[channel];}
void CalibrationBox::SetTimeShift(CalibrationBox& other){
	if(other.GetNChannels() != this->GetNChannels()){printf("CalibrationBox::SetTimeShift(): Mismatch in channel number, can not load data.\n"); return;}
	if(other.m_tick_period != m_tick_period){printf("CalibrationBox::SetTimeShift(): Mismatch in reference frequencies, can not load data.\n"); return;}
	for(int i=0;i<GetNChannels();i++)
		SetTimeShift(i,other.GetTimeShift(i));
}

double CalibrationBox::GetBinSize_CC(){return m_tick_period;};
double CalibrationBox::GetBinSize_fine(){return m_tick_period/m_nBins;};
double CalibrationBox::GetBinSize_BDITH(){return m_tick_period/m_nBins/4;};
int    CalibrationBox::GetFineBinScaling_BDITH(){return m_nBins*4;};

//Compute full time (using hit->GetTime()) based on DNL corrected value, in units of input tick periods (m_tick_period)
//Include fine time shift if bit is set
//Throws std::out_of_range if no calibration data exists
double CalibrationBox::GetTimeDNLCorrected_real(const klaus_event& thehit, bool timeshift_correction) const{
	int finebins= thehit.GetTime()&((1<<m_nBits)-1); //lower bits
	double time=  thehit.GetTime() - finebins;       //upper bits
	//compute bin center from bin edges: (with lower edge of bin 0 == 0, no boundary stored)
	if(finebins==0)
		time+=0.5*m_code_times[thehit.channel][0];
	else
		time+=0.5*(m_code_times[thehit.channel][finebins]+m_code_times[thehit.channel][finebins-1]);
	//do timeshift correction
	if(timeshift_correction) time+=m_finetime_offsets[thehit.channel];
//	printf("GetTimeDNLCorreceted_real: Result: %d \n",time*m_tick_period);
	return time*m_tick_period;
}

//Compute dithered and binned time (using hit->GetTime()) based on DNL corrected value in units of fine bins/4
//Include fine time shift if bit is set
//Throws std::out_of_range if no calibration data exists
uint32_t CalibrationBox::GetTimeDNLCorrected_BDITH(const klaus_event& thehit, bool timeshift_correction){
	uint32_t fbin = thehit.GetTime()&((1<<m_nBits)-1); //lower bits
	uint32_t tick     = (thehit.GetTime() - fbin); //upper bits
	auto channel=thehit.channel;
//	if(timeshift_correction) tick+=(m_finetime_offsets[channel]);

	//calculate dithered mapped bin based on measured fine bin
	//draw a dice in the range of the extends of the fine bin, as mapped.
	//calulate bin extends, in units of mapped bins
	float binlow=0;
	if(fbin!=0) binlow=m_code_times[channel][fbin-1];
	float binhigh=m_code_times[channel][fbin];


	//generate floating point random number within the extends of the bin and remap to new binning
	std::uniform_real_distribution<float> distribution(binlow,binhigh);
	float Dice = distribution(m_randgen);
	uint32_t res = tick*1+int(Dice*1.);
//	printf("For time=%u: tick%u: fbin=%x(%u): low=%f, high=%f. Dice=%2.2f -> %u\n",thehit.GetTime(),tick,fbin,fbin,binlow,binhigh,Dice,res);
	//NOTE: round is biasing the distribution to have the 1st bin represented lower.
	return res;
}

//Visualisation methods
TH1F CalibrationBox::GetCountsHist(){
	TH1F hres("CalibBox_counts","Calibration Box: Hit counts",m_code_counts_sum.size(),0,m_code_counts_sum.size());
//	printf("sum vector size %i\n",m_code_counts.size());
	int sum=0;
	for(size_t i=0;i<m_code_counts_sum.size();i++){
		hres.SetBinContent(i+1,m_code_counts_sum[i]);
		hres.SetBinError(i+1,TMath::Sqrt(m_code_counts_sum[i]));
		sum+=m_code_counts_sum[i];
	}
	hres.SetEntries(sum);
	hres.GetXaxis()->SetTitle("Channel");
	hres.GetYaxis()->SetTitle("# Hits");
	return hres;
};

TH1F CalibrationBox::GetBinWidthHist(uint16_t channel){
	TH1F hres(TString::Format("CalibBox_binwidth_ch%d",channel),TString::Format("Calibration Box: Bin width of channel %d",channel),m_nBins,0,m_nBins);
	if(m_code_counts_sum[channel]!=0)
		for(int i=0;i<m_nBins;i++){
			double val=m_code_counts[channel][i]*1./m_code_counts_sum[channel]*m_nBins;
			double err=TMath::Sqrt(1./m_code_counts[channel][i] + 1./m_code_counts_sum[channel]);
			hres.SetBinContent(i+1,val);
			hres.SetBinError(i+1,err*val);
		}
	return hres;
}

TH2F CalibrationBox::GetBinWidthMap(){
	TH2F hres("CalibBox_binwidthmap","Calibration Box: Bin width map",m_code_counts_sum.size(),0,m_code_counts_sum.size(),32,0,32);
	for(size_t channel=0;channel<m_code_counts_sum.size();channel++){
		if(m_code_counts_sum[channel]!=0)
			for(int i=0;i<31;i++){
				double val=m_code_counts[channel][i]*1./m_code_counts_sum[channel];
				double err=TMath::Sqrt(1./m_code_counts[channel][i] + 1./m_code_counts_sum[channel]);
				hres.SetBinContent(hres.FindBin(channel,i),val);
				hres.SetBinError(hres.FindBin(channel,i),err*val);
			}
	}
	return hres;
}

TGraph CalibrationBox::GetFineshiftGraph(){
	TGraph gres(m_finetime_offsets.size());
	gres.SetNameTitle("Calibbox_Fineshift","Calibration Box: Fine time shifts");
	for(size_t channel=0;channel<m_finetime_offsets.size();channel++){
		gres.SetPoint(channel,channel,m_finetime_offsets[channel]*m_tick_period);
	}
	return gres;
}
	ClassImp(CalibrationBox);

