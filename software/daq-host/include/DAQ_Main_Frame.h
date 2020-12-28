// NO DATA PROCESSING in this GUI, Only show the results of the HigstogrammedResults
// And also change the parameters of the HigstogrammedResults

#ifndef DAQ_MAIN_FRAME
#define DAQ_MAIN_FRAME

#include "TQObject.h"
#include "RQ_OBJECT.h"
#include "TGWindow.h"
#include "TFile.h"
//#include "TGMainFrame.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TRootEmbeddedCanvas.h"
#include "TGNumberEntry.h"
#include "TTimer.h"
#include "TThread.h"
#include "TMutex.h"
#include "TRandom.h"

//#include "HistogrammedResults.h"
#include "HistogramDAQ_client.h"
class HistogrammedResults;

class DAQ_Main_Frame: public TGMainFrame{
	RQ_OBJECT("DAQ_Main_Frame");
	private:
		TGNumberEntry		*fNum;
		TGNumberEntry		*fXMin;
		TGNumberEntry		*fXMax;
		TGNumberEntry		*fpipeBranch;
		TGNumberEntry		*fpipeBranchDis;

		Int_t	type;
		Int_t	channel;
		TTimer*	m_CanvasUpdateTimer;

		int	chip_id;

		int	stop;
		TRandom rnd;
		int	xRangeMin;
		int	xRangeMax;
		bool	useDefaultRange; // true after DoSetRangeUser; false after DoDataType
		TH2F * h_map;

		// Histogram used to do calibration test
		TH1F*	h_cal;
		TFile*	f_cal;

                // for the pipeline ADC reconstruction
                int     pipeBranch;
                int     pipeBranchDis;
                bool    useDefaultPipe;
                void    DoSetPipeBranch();
                void    DoSetPipeBranchDis();
                void    DoSetPipe();

	public:
		DAQ_Main_Frame(const TGWindow *p, UInt_t w, UInt_t h);
		virtual ~DAQ_Main_Frame();
		void	DoReset();
		void	DoSetChannel();
		void	DoDataType(Int_t);
		void	DoStop();
		void	DoSave();
		void	DoSetRangeUser();
		void	DoChangeXMin();
		void	DoChangeXMax();

		TRootEmbeddedCanvas	*fEcanvas;

		HistogramDAQ*	histDAQ;
		void	Set_HistDAQHost(const char* server);
		void	Set_HistDAQASIC(unsigned char ASIC);

//		void 	DoDAQ();
		void	Update_Canvas();

//		static void DoDAQStarter(void* obj){((DAQ_Main_Frame*)obj)->DoDAQ();};
	ClassDef(DAQ_Main_Frame,1);
};

#endif
