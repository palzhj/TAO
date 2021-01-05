// File: CalibrationBox.h
// Desc: Time calibration of mutrig based data:
// 	 Time aligmnent (coarse, full), DNL correction
// 	 TODO: do we want to have alignment in space dimensions also here?
// Author: K. Briggl


#include "TObject.h"
#include <vector>
#include <array>
#include <inttypes.h>
#include <random>
class klaus_event;
class klaus_acquisition;

//ROOT headers for visualization graphs
#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"

#ifndef _CALIBRATIONBOX_H__
#define _CALIBRATIONBOX_H__
class CalibrationBox
: public TObject
{
public:
	//Build empty calibration box constructor
	CalibrationBox();
	//Build new calibration box and reserve space for N channels with a reference frequency of f in Hz.
	CalibrationBox(int nChannels,double f_ref,int nBins);
	~CalibrationBox();
	int GetNChannels(){return m_CC_offsets.size();};
	void SetFref(double f_ref);
	void SetTtick(double period);
	double GetFref(){return 1./m_tick_period;};
	double GetTtick();

	//Update DNL maps including new hit or hits.
	//Important: Recomputation to meaningful histograms is only done after a call to Recompute();
	void AddHit(klaus_event& thehit);
	void AddHits(klaus_acquisition& event);
	void SetHitCounts(CalibrationBox& box);
	int RecomputeDNL();
        bool DNL_Valid() const{return m_DNLcomputed;};
	//Generate ROOT histograms/Graphs for visualization (Note: not stored internally but generated every time) 
	TH1F   GetCountsHist();
	TH1F   GetBinWidthHist(uint16_t channel);
	TH2F   GetBinWidthMap();
	TGraph GetFineshiftGraph();
	TGraph GetFullshiftGraph(); //Full shift including coarse and fine shifts

	//Merge data from other into this, appending information at the end.
	void Append(CalibrationBox& other);
	//Merge data from other into this, combining the count values and averaging the offsets. DNL maps are recomputed
	void Merge(CalibrationBox& other);
	//Reset count histograms - and only these: Not propagated to the results containers, so the class uses the persistent results until the next call of RecomputeDNL().
	void Reset();

	// Set&Get fine time shift values in units of coarse ticks (depending on fRef)
	// This is only applied in the GetTimeXXX() methods, and only if the correstponding bit is set when calling the function
	void SetTimeShift(CalibrationBox& other);
	void SetTimeShift(int channel, float value);
	double GetTimeShift(int channel);
	
	//Compute full time (using hit->GetTime()) based on DNL corrected value, in seconds (or depending on the unit/exponent of the tick_period)
	//Include fine time shift if bit is set
	//Throws std::out_of_range if no calibration data exists
	double GetTimeDNLCorrected_real(const klaus_event& thehit, bool timeshift_correction) const;

	//Compute dithered and binned time (using hit->GetTime()) based on DNL corrected value,
	//in units of remapped fine counter bins (=1./(32*4) CC bins, 12.5ps for a 625MHz reference clock)
	//Include fine time shift if bit is set
	//Throws std::out_of_range if no calibration data exists
	uint32_t GetTimeDNLCorrected_BDITH(const klaus_event& thehit, bool timeshift_correction);

	double GetBinSize_CC(); //Get size of CC bin
	double GetBinSize_BDITH(); //Get bin size of fine bins returned by the BDITH method.
	double GetBinSize_fine();  //Get size of fine counter bins
	int    GetFineBinScaling_BDITH(); //get ratio of coarse counter bin size over fine bin size given by the BDITH method.
protected:
	bool m_DNLcomputed;
	int m_nBins;
	std::vector<int8_t> m_CC_offsets; //units: CC bins
	std::vector<float> m_finetime_offsets; //units: CC bins
	std::vector<uint64_t> m_code_counts_sum;
	std::vector<std::vector<uint32_t> > m_code_counts;//histogram for counts
	std::vector<std::vector<float> > m_code_times; //!absolute (summed from t_0=0) times for each bin for use in GetTimeDNLCorrected_real
	int m_nBits;
	double m_tick_period;

	std::default_random_engine m_randgen; //!
public:
	ClassDef(CalibrationBox,1);
};
#endif
