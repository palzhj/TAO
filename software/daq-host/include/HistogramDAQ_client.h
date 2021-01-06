//Histogramming DAQ for K4
//Based on ROOT spy.C example
//Author: K.Briggl

#ifndef __HISTOGRAMDAQ_CLIENT_H
#define __HISTOGRAMDAQ_CLIENT_H
#include <stdarg.h>
#include <list>
#include "DAQctrl_client.h"
class HistogrammedResults;

class HistogramDAQ : public DAQctrl{
	private:
		int m_ASIC_bound;
	public:
		// move m_results into public for the accessibility of child class
		HistogrammedResults* m_results;

		HistogramDAQ(std::string host,unsigned short port);
		~HistogramDAQ();
		HistogrammedResults* GetResults(){return m_results;};
		HistogrammedResults** GetResultsPtr(){return &m_results;};

		void BindTo(unsigned char ASIC){m_ASIC_bound=ASIC;};
		unsigned char GetBound(){return m_ASIC_bound;};

		// Update pipe reconstruction parameters;
                // branch=ADC_6b; branchDis: Distance to the branch before
		void UpdateParams_12b(int channel, int branch, int branchDis);

		//Fetch histograms for given ASIC Slave ID; Set ASIC=-1 for the first
		void FetchResults();
		void ResetResults();
	ClassDef(HistogramDAQ,1);
};

#endif
