/*
 * klaus4Config.cpp
 *
   Bitpattern definition for the KLauS4.0 miniASIC
 *  Created on: 20.09.2015
 *      Author: Konrad Briggl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Klaus4Config.h"
#include "VirtualConfig.h"
#include "VirtualInterface.h"

#define NPARS		880
#define PATLEN		308     //319 


TKLauS4Config::TKLauS4Config(TVirtualInterface* Interface)
    :TVirtualConfig(Interface,par_descriptor,NPARS,PATLEN)
{
    bitpattern_read =  new char[PATLEN];
    bitpattern_write = new char[PATLEN];


    memset(bitpattern_write,0x00,sizeof(char) * PATLEN);
    memset(bitpattern_read,0xff,sizeof(char) * PATLEN);
};

TKLauS4Config::~TKLauS4Config()
{
    delete [] bitpattern_read;
    delete [] bitpattern_write;
};

void TKLauS4Config::ChipReset(){
    IssueCommand('R',0, NULL);
}

// 5 parameters
// Not changed in KLauS5
#define EXPAND_ADC( prefix , offset ) \
TVirtualConfig::parameter(#prefix"/ADC_PIPEamp_comp_ena",       (offset+0) ,	0,"Enable pipeline amplifier mismatch compensation current"),\
TVirtualConfig::parameter(#prefix"/ADC_PIPEamp_comp_dac",       (offset+1) ,	0,"Pipeline amplifier compensation DAC"),\
TVirtualConfig::parameter("__filler",			        (offset+5) ,	0,""),\
TVirtualConfig::parameter("__filler",			        (offset+10) ,	0,""),\
TVirtualConfig::parameter(#prefix"/ADC_captrim",	        (offset+16) ,	0,"Pipeline feedback capacitor tuning DAC"),\

// 5+17=22 parameters
//channel pattern definition, includes fe/adc/channel-digital. Total 53 bits
// For the FE, bit 18,19 was added in KLaus5
#define EXPAND_CHANNEL( prefix , offset ) \
	TVirtualConfig::parameter(#prefix"/mask",        	(offset+0) ,	0,"Channel mask"),\
	EXPAND_ADC( prefix , (offset+1) )\
	TVirtualConfig::parameter(#prefix"/trigger_th_finetune",(offset+20+1) ,	0,"Comparator threshold finetuning.\n Higher values => Higher threshold."),\
	TVirtualConfig::parameter(#prefix"/gainsel_busy_enable",(offset+20+5) ,	0,"1) Disable output of gain selection result while ADC is busy.\nNominal 0"),\
	TVirtualConfig::parameter(#prefix"/hold_delay_finetune",(offset+20+6) ,	0,"Hold delay finetuning DAC. Nominal: 0x01"),\
	TVirtualConfig::parameter(#prefix"/branch_sel_config",	(offset+20+10) ,0,"Branch selection configuration\n 0) Auto HG/LG ; 1)External source ; 2) LG force ; 3) HG force"),\
	TVirtualConfig::parameter(#prefix"/ext_trigger_select", (offset+20+12) ,0,"Enable external trigger for ADC conversion. Set to 1 for external trigger, need to put global threshold to 0"),\
	TVirtualConfig::parameter("__filler",			(offset+20+13) ,0,""),\
	TVirtualConfig::parameter(#prefix"/HG_scale_select",	(offset+20+14) ,0,"High gain scaling. 1) 1:1; 0) 1:7"),\
	TVirtualConfig::parameter(#prefix"/ADC_in_n_select",	(offset+20+15) ,0,"ADCn selection.\n 1) Internal GNDA ; 0) From Pad ADCn"),\
	TVirtualConfig::parameter(#prefix"/lfota_disable",	(offset+20+16) ,0,"Disables Low Frequency feedback amplifier bias"),\
	TVirtualConfig::parameter(#prefix"/monitor_HG_disable",	(offset+20+17) ,0,"Disables Analog monitor feedthrough from HG branch"),\
	TVirtualConfig::parameter(#prefix"/monitor_LG_disable",	(offset+20+18) ,0,"Disables Analog monitor feedthrough from LG branch"),\
	TVirtualConfig::parameter(#prefix"/trigger_LSB_scale",	(offset+20+19) ,0,"Scale the trigger comparator's LSB by 0 (in KLauS4 = 1)"),\
	TVirtualConfig::parameter(#prefix"/LG_scale_select",	(offset+20+20) ,0,"Low gain scaling. 1) 1:100?; 0). 1:40 (KLauS4)"),\
	TVirtualConfig::parameter("__filler",			(offset+20+21) ,0,""),\
	TVirtualConfig::parameter(#prefix"/sw_triodesel",	(offset+20+24) ,0,"Select input stage triode bias.\n0) Internal selfbias (default);1) Control by DAC"),\
	TVirtualConfig::parameter(#prefix"/vDAC_SiPM",		(offset+20+25) ,0,"Input terminal voltage tuning DAC"),\

// 19 parameters
//bias pattern definition, 64 bits
#define EXPAND_BIAS( prefix, offset ) \
	TVirtualConfig::parameter(#prefix"/amon_DAC",		(offset+0 ),    0,"Analog monitor OTA bias. Nominal=8"),\
	TVirtualConfig::parameter(#prefix"/amon_ena",		(offset+6 ),    0,"Analog monitor OTA enable, positive logic"),\
	TVirtualConfig::parameter(#prefix"/shap_LG_DAC",	(offset+7 ),    0,"LG shaper OTA bias. Nominal=4"),\
	TVirtualConfig::parameter(#prefix"/shap_ena",		(offset+11),    0,"Shaper OTA bias enable (LG&HG)"),\
	TVirtualConfig::parameter(#prefix"/shap_HG_DAC",	(offset+12),    0,"HG shaper OTA bias. Nominal=4"),\
	TVirtualConfig::parameter(#prefix"/delay_ena",		(offset+16),    0,"Delay cell bias enable"),\
	TVirtualConfig::parameter(#prefix"/lfota_HG_DAC",	(offset+17),    0,"High gain Low frequency feedback bias current. Nominal=2.\n 0 is maximum current"),\
	TVirtualConfig::parameter(#prefix"/lfota_LG_DAC",	(offset+20),    0,"Low gain Low frequency feedback bias current. Nominal=2.\n 0 is maximum current"),\
	TVirtualConfig::parameter(#prefix"/subthbias_Rtrim",	(offset+23),    0,"Subthreshold generator tuning, 2 taps. Nominal=00.\nValues 00,01,11. 11 gives steepest SiPM-DAC voltage ramp & highest current. States 01,10 are equivalent."),\
	TVirtualConfig::parameter(#prefix"/isgtriode_trim_ena_n",(offset+25),   0,"Disable resistor DAC for triode tuning.\n Set to 1,DAC value 255 for minimum power consumption"),\
	TVirtualConfig::parameter(#prefix"/isgtriode_trim_DAC",	(offset+26),    0,"Volta    ge DAC for input stage triode tuning"),\
	TVirtualConfig::parameter(#prefix"/isg_sdcomp_DAC",	(offset+34),    0,"Input stage shutdown compensation current.\nHigher values: lower current\nTune together with sdbias and isgbias to minimize voltage offset between on and off states."),\
	TVirtualConfig::parameter(#prefix"/isg_sdbias_DAC",	(offset+37),    0,"Input stage shutdown bias current.\nHigher values: lower current\nTune together with sdcomp and isgbias to minimize voltage offset between on and off states.\n Also changes input impedance in low power state."),\
	TVirtualConfig::parameter(#prefix"/comparator_th_gs",	(offset+40),    0,"Gain selection threshold. Higher value, higher threshold."),\
	TVirtualConfig::parameter("__filler",			(offset+46),    0),\
	TVirtualConfig::parameter(#prefix"/isg_biastune",	(offset+51),    0,"Input stage bias current tuning."),\
	TVirtualConfig::parameter("__filler",			(offset+53),    0),\
	TVirtualConfig::parameter(#prefix"/comparator_th_trig",	(offset+54),    0,"Timing comparator threshold. Higher value, higher threshold.\nNoise should be around ~3-5; Fine tuning with channelwise DACs"),\
	TVirtualConfig::parameter(#prefix"/delay_DAC",		(offset+60),    1,"Hold delay tuning DAC. Higher values gives larger current and smaller delay.\nDelay vs. current is hyperbolic."),\


// 69 + 19 + 36*22 = 880. The last one is not included.
TVirtualConfig::parameter const TKLauS4Config::par_descriptor[]=
{
        TVirtualConfig::parameter("digital/clk_div_sel",		        0,      0,"Clk division selection. 0) Ndiv=4; 1). Ndiv=1"),
        TVirtualConfig::parameter("coincidence/powergate_time_1",		1+0,    0,"Clock gating time 1 (ISG precharge stop time)"),
        TVirtualConfig::parameter("coincidence/powergate_time_2",		1+3,    0,"Clock gating time 2 (DC feedback precharge stop time)"),
        TVirtualConfig::parameter("coincidence/powergate_time_3",		1+13,   0,"Clock gating time 3 (Shaper & Delay bias enable time)"),
        TVirtualConfig::parameter("coincidence/powergate_time_4",		1+23,   0,"Clock gating time 4 (Channel unmask time)"),
        TVirtualConfig::parameter("coincidence/powergate_time_5",		1+33,   0,"Clock gating time 5 (TDC start. LSB=50ns)"),
        TVirtualConfig::parameter("coincidence/powergate_do_gate",		44+0,   0,"Gated block gating flags (if gated)"),
        TVirtualConfig::parameter("coincidence/powergate_enable",		44+11,  0,"Gated blocks enable flags (if enabled at all)"),
        TVirtualConfig::parameter("digital/i2c_address",		        66,     0,"I2C slave address (7b)"),
        TVirtualConfig::parameter("digital/debug_pad_enable",		        66+7,   0,"Enable digital debug pad (Not Connected)"),
        TVirtualConfig::parameter("digital/i2c_readout_enable",		        66+8,   0,"Enable I2C readout mode"),
        TVirtualConfig::parameter("digital/LVDS_serdata_tx_iDAC",	        75,     0,"LVDS output pad current setting. 8-bits"),
        TVirtualConfig::parameter("digital/LVDS_serdata_tx_vcmDAC",	        83,     0,"LVDS output pad common mode voltage setting. 6-bits"),
        TVirtualConfig::parameter("digital/LVDS_serdata_tx_enable",	        89,     0,"Enable LVDS output pad (LVDS data)"),
        TVirtualConfig::parameter("digital/generate_8b10b_idle",	        90,     0,"Enable sending of K28 filler words in stream"),
        TVirtualConfig::parameter("digital/TDC_factory_mode",		        91,     0,"No internal TDC adjustment."),
        TVirtualConfig::parameter("digital/TDC_offset_middle",		        92,     0,"TDC offset compensation for MC"),
        TVirtualConfig::parameter("digital/TDC_offset_fine",		        95,     0,"TDC offset compensation for CC"),
        TVirtualConfig::parameter("digital/TDC_offset_enable",		        100,    0,"TDC offset compensation for CC"),
        TVirtualConfig::parameter("bias/ADC_rvb",		                101,    0,"ADC RVB bias. Under i_pgate[5]=1: 0). I; 1). 1.5I; 3). 2I"),
        TVirtualConfig::parameter("digital/T0channel_mask",		        101+2,  0,"T0 channel masking"),
        TVirtualConfig::parameter("coincidence/L1_trigger_pretime",		101+3,  0,"L1 fifo trigger match window before trigger"),
        TVirtualConfig::parameter("coincidence/L1_trigger_pretime_sign",	101+7,  0,"L1 fifo trigger match window before trigger (sign)"),
        TVirtualConfig::parameter("coincidence/L1_trigger_posttime",	        101+8,  0,"L1 fifo trigger match window after trigger"),
        TVirtualConfig::parameter("coincidence/L1_trigger_mode",		101+12, 0,"L1 fifo trigger mode"),
        TVirtualConfig::parameter("digital/PLL_resistor",		        114,    0,"PLL resistor. default"),
        TVirtualConfig::parameter("digital/PLL_Icp",		                116,    0,"PLL charge pump current, default"),
        TVirtualConfig::parameter("digital/PLL_Ibuf",		                120,    0,"PLL clock buffer current, default"),
        EXPAND_BIAS( bias , 124 )
        TVirtualConfig::parameter("coincidence/coincidence_window",		188,    0,"Coincidence window configuration"),
        TVirtualConfig::parameter("coincidence/channel0/coincidence_mask",	191+0,  0,"Channel0 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel1/coincidence_mask",	191+10, 0,"Channel1 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel2/coincidence_mask",	191+20, 0,"Channel2 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel3/coincidence_mask",	191+30, 0,"Channel3 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel4/coincidence_mask",	191+40, 0,"Channel4 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel5/coincidence_mask",	191+50, 0,"Channel5 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel6/coincidence_mask",	191+60, 0,"Channel6 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel7/coincidence_mask",	191+70, 0,"Channel7 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel8/coincidence_mask",	191+80, 0,"Channel8 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel9/coincidence_mask",	281+ 0, 0,"Channel9 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel10/coincidence_mask",	281+10, 0,"Channel10 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel11/coincidence_mask",	281+20, 0,"Channel11 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel12/coincidence_mask",	281+30, 0,"Channel12 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel13/coincidence_mask",	281+40, 0,"Channel13 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel14/coincidence_mask",	281+50, 0,"Channel14 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel15/coincidence_mask",	281+60, 0,"Channel15 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel16/coincidence_mask",	281+70, 0,"Channel16 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel17/coincidence_mask",	281+80, 0,"Channel17 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel18/coincidence_mask",	371+ 0, 0,"Channel18 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel19/coincidence_mask",	371+10, 0,"Channel19 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel20/coincidence_mask",	371+20, 0,"Channel20 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel21/coincidence_mask",	371+30, 0,"Channel21 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel22/coincidence_mask",	371+40, 0,"Channel22 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel23/coincidence_mask",	371+50, 0,"Channel23 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel24/coincidence_mask",	371+60, 0,"Channel24 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel25/coincidence_mask",	371+70, 0,"Channel25 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel26/coincidence_mask",	371+80, 0,"Channel26 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel27/coincidence_mask",	461+ 0, 0,"Channel27 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel28/coincidence_mask",	461+10, 0,"Channel28 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel29/coincidence_mask",	461+20, 0,"Channel29 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel30/coincidence_mask",	461+30, 0,"Channel30 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel31/coincidence_mask",	461+40, 0,"Channel31 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel32/coincidence_mask",	461+50, 0,"Channel32 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel33/coincidence_mask",	461+60, 0,"Channel33 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel34/coincidence_mask",	461+70, 0,"Channel34 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("coincidence/channel35/coincidence_mask",	461+80, 0,"Channel35 coincidence mask, bit 0 is ignored"),
        TVirtualConfig::parameter("digital/trigger_or_enable",      		551,    0,"Enable or-ed trigger output"),
        TVirtualConfig::parameter("digital/powerdown_pipe",	        	552,    0,"Power down ADC bias when idle"),
        TVirtualConfig::parameter("digital/powerdown_sar",		        553,    0,"Power down ADC bias when idle"),
        TVirtualConfig::parameter("digital/pipemode_enable",		        554,    0,"Enable 12b conversion mode"),
        EXPAND_CHANNEL( channel0  , 555+53*0)
        EXPAND_CHANNEL( channel1  , 555+53*1)
        EXPAND_CHANNEL( channel2  , 555+53*2)
        EXPAND_CHANNEL( channel3  , 555+53*3)
        EXPAND_CHANNEL( channel4  , 555+53*4)
        EXPAND_CHANNEL( channel5  , 555+53*5)
        EXPAND_CHANNEL( channel6  , 555+53*6)
        EXPAND_CHANNEL( channel7  , 555+53*7)
        EXPAND_CHANNEL( channel8  , 555+53*8)
        EXPAND_CHANNEL( channel9  , 555+53*9)
        EXPAND_CHANNEL( channel10 , 555+53*10)
        EXPAND_CHANNEL( channel11 , 555+53*11)
        EXPAND_CHANNEL( channel12 , 555+53*12)
        EXPAND_CHANNEL( channel13 , 555+53*13)
        EXPAND_CHANNEL( channel14 , 555+53*14)
        EXPAND_CHANNEL( channel15 , 555+53*15)
        EXPAND_CHANNEL( channel16 , 555+53*16)
        EXPAND_CHANNEL( channel17 , 555+53*17)
        EXPAND_CHANNEL( channel18 , 555+53*18)
        EXPAND_CHANNEL( channel19 , 555+53*19)
        EXPAND_CHANNEL( channel20 , 555+53*20)
        EXPAND_CHANNEL( channel21 , 555+53*21)
        EXPAND_CHANNEL( channel22 , 555+53*22)
        EXPAND_CHANNEL( channel23 , 555+53*23)
        EXPAND_CHANNEL( channel24 , 555+53*24)
        EXPAND_CHANNEL( channel25 , 555+53*25)
        EXPAND_CHANNEL( channel26 , 555+53*26)
        EXPAND_CHANNEL( channel27 , 555+53*27)
        EXPAND_CHANNEL( channel28 , 555+53*28)
        EXPAND_CHANNEL( channel29 , 555+53*29)
        EXPAND_CHANNEL( channel30 , 555+53*30)
        EXPAND_CHANNEL( channel31 , 555+53*31)
        EXPAND_CHANNEL( channel32 , 555+53*32)
        EXPAND_CHANNEL( channel33 , 555+53*33)
        EXPAND_CHANNEL( channel34 , 555+53*34)
        EXPAND_CHANNEL( channel35 , 555+53*35)
        TVirtualConfig::parameter("", 2463, 0),
};


