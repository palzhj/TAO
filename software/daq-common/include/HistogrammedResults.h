//Histogrammed results collection for K4: Manage histograms filled from klaus_events
//Author: K.Briggl

#ifndef __HISTOGRAMMED_RESULTS_H
#define __HISTOGRAMMED_RESULTS_H

#include <list>
#include "TObject.h"
#include "TCanvas.h"
#include "TH1.h"
#ifndef __CINT__
#include "EventType.h"
#else
class klaus_event;
#endif
#include "klaus_constants.h"

// Histogrammed results collection for ONE KLauS ASIC with ID specified
// All the histograms are filled with klaus_event one by one
// The piplined ADC reconstruction is also included in this class
class HistogrammedResults: public TObject
{
	public:
		HistogrammedResults();
		~HistogrammedResults();
		void SetPrefix(unsigned char ID); //Update histogram name&titles: ASIC IDs
		void Fill(klaus_event&);
		void Fill(std::list<klaus_event>);
		void Reset();

	//channel hit histogram
		TH1F*	h_channels;
		TH1F*	h_multi;
	//histograms for each channel
		TH1F*	h_gainsel_evt[constants::CH_NUM];
		TH1F*	h_gainsel_busy[constants::CH_NUM];
		TH1F*	h_ADC_10b[constants::CH_NUM];
		TH1F*	h_ADC_6b[constants::CH_NUM];
		TH1F*	h_ADC_PIPE[constants::CH_NUM];
		TH1F*	h_ADC_12b[constants::CH_NUM];
		TH1F*	h_time[constants::CH_NUM+1];    // T0 channel constants::CH_NUM + 1	
		
	//show member canvas
		enum resultstype{
			SHOW_CH0=0,     SHOW_CH1=1,     SHOW_CH2=2,     SHOW_CH3=3,     SHOW_CH4=4,     SHOW_CH5=5,
			SHOW_CH6=6,     SHOW_CH7=7,     SHOW_CH8=8,     SHOW_CH9=9,     SHOW_CH10=10,   SHOW_CH11=11,
			SHOW_CH12=12,   SHOW_CH13=13,   SHOW_CH14=14,   SHOW_CH15=15,   SHOW_CH16=16,   SHOW_CH17=17,
			SHOW_CH18=18,   SHOW_CH19=19,   SHOW_CH20=20,   SHOW_CH21=21,   SHOW_CH22=22,   SHOW_CH23=23,
			SHOW_CH24=24,   SHOW_CH25=25,   SHOW_CH26=26,   SHOW_CH27=27,   SHOW_CH28=28,   SHOW_CH29=29,
			SHOW_CH30=30,   SHOW_CH31=31,   SHOW_CH32=32,   SHOW_CH33=33,   SHOW_CH34=34,   SHOW_CH35=35,
			SHOW_CHT0=36,
			SHOW_GAINSEL_EVT=37,
			SHOW_GAINSEL_BUSY=38,
			SHOW_ADC_10B=39,
			SHOW_ADC_6B=40,
			SHOW_ADC_PIPE=41,
			SHOW_ADC_12B=42,
			SHOW_TIME=43,
			SHOW_CHANNELS=44
		};
		//Build a canvas with 7 pads arranged as the PCB
		TCanvas* ChannelPhysCanvas(TCanvas* c=NULL);
		//Show the results histograms
		TCanvas* Show(resultstype what,TCanvas* c_old=NULL);

                //  for pipelined ADC reconstruction
                void    setSubRangeOffset(int channel, unsigned short ADC6b, int off);
                void    setAllRangeOffset(int channel, int* off);
                

	private:
                // Since for every chip, the pipe reconstructure parameters are the same,
                // So they are implemented as a member of the HistogrammedResults.
                //
                // Considering the DNL in the ADC_6b, so a unique gain for every branch is in-sufficient
                // For every ADC_6b branch, restore a offset value, so the final 12bit result is given by
                // Combined 12 bit result = ADC_PIPE + offset[ADC_6b]
                int     offset[constants::CH_NUM][64]; 
                void    setPipe();
                int     getCombinedResults(int channel, unsigned short ADC6b, unsigned short ADCPIPE);
	ClassDef(HistogrammedResults,1);
};

#endif
