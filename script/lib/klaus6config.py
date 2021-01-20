#!/usr/bin/python
# -*- coding: UTF-8 -*-
import math

class _parameter:
    def __init__(self, name, offset, endianess, description):
        self.name = name
        self.offset = offset
        self.endianess = endianess # 0: little endian, 1: big endian
        self.description = description

class TVirtualConfig:
    def __init__(self):
        # name, offset, endianess, description
        # 0: little endian, 1: big endian
        self.parameters = []
        self.parameters.append(_parameter("digital/clk_div_sel",                     0,      0, "Clk division selection. 0) Ndiv=4; 1). Ndiv=1"))
        self.parameters.append(_parameter("coincidence/powergate_time_1",            1+0,    0, "Clock gating time 1 (ISG precharge stop time)"))
        self.parameters.append(_parameter("coincidence/powergate_time_2",            1+3,    0, "Clock gating time 2 (DC feedback precharge stop time)"))
        self.parameters.append(_parameter("coincidence/powergate_time_3",            1+13,   0, "Clock gating time 3 (Shaper & Delay bias enable time)"))
        self.parameters.append(_parameter("coincidence/powergate_time_4",            1+23,   0, "Clock gating time 4 (Channel unmask time)"))
        self.parameters.append(_parameter("coincidence/powergate_time_5",            1+33,   0, "Clock gating time 5 (TDC start. LSB=50ns)"))
        self.parameters.append(_parameter("coincidence/powergate_do_gate",           44+0,   0, "Gated block gating flags (if gated)"))
        self.parameters.append(_parameter("coincidence/powergate_enable",            44+11,  0, "Gated blocks enable flags (if enabled at all)"))
        self.parameters.append(_parameter("digital/i2c_address",                     66,     0, "I2C slave address (7b)"))
        self.parameters.append(_parameter("digital/debug_pad_enable",                66+7,   0, "Enable digital debug pad (Not Connected)"))
        self.parameters.append(_parameter("digital/i2c_readout_enable",              66+8,   0, "Enable I2C readout mode"))
        self.parameters.append(_parameter("digital/LVDS_serdata_tx_iDAC",            75,     0, "LVDS output pad current setting. 8-bits"))
        self.parameters.append(_parameter("digital/LVDS_serdata_tx_vcmDAC",          83,     0, "LVDS output pad common mode voltage setting. 6-bits"))
        self.parameters.append(_parameter("digital/LVDS_serdata_tx_enable",          89,     0, "Enable LVDS output pad (LVDS data)"))
        self.parameters.append(_parameter("digital/generate_8b10b_idle",             90,     0, "Enable sending of K28 filler words in stream"))
        self.parameters.append(_parameter("digital/TDC_factory_mode",                91,     0, "No internal TDC adjustment."))
        self.parameters.append(_parameter("digital/TDC_offset_middle",               92,     0, "TDC offset compensation for MC"))
        self.parameters.append(_parameter("digital/TDC_offset_fine",                 95,     0, "TDC offset compensation for CC"))
        self.parameters.append(_parameter("digital/TDC_offset_enable",               100,    0, "TDC offset compensation for CC"))
        self.parameters.append(_parameter("bias/ADC_rvb",                            101,    0, "ADC RVB bias. Under i_pgate[5]=1: 0). I; 1). 1.5I; 3). 2I"))
        self.parameters.append(_parameter("digital/T0channel_mask",                  101+2,  0, "T0 channel masking"))
        self.parameters.append(_parameter("coincidence/L1_trigger_pretime",          101+3,  0, "L1 fifo trigger match window before trigger"))
        self.parameters.append(_parameter("coincidence/L1_trigger_pretime_sign",     101+7,  0, "L1 fifo trigger match window before trigger (sign)"))
        self.parameters.append(_parameter("coincidence/L1_trigger_posttime",         101+8,  0, "L1 fifo trigger match window after trigger"))
        self.parameters.append(_parameter("coincidence/L1_trigger_mode",             101+12, 0, "L1 fifo trigger mode"))
        self.parameters.append(_parameter("digital/PLL_resistor",                    114,    0, "PLL resistor. default"))
        self.parameters.append(_parameter("digital/PLL_Icp",                         116,    0, "PLL charge pump current, default"))
        self.parameters.append(_parameter("digital/PLL_Ibuf",                        120,    0, "PLL clock buffer current, default"))
        self.parameters.append(_parameter("bias/amon_DAC",                           124+0 , 0, "Analog monitor OTA bias. Nominal=8"))
        self.parameters.append(_parameter("bias/amon_ena",                           124+6 , 0, "Analog monitor OTA enable, positive logic"))
        self.parameters.append(_parameter("bias/shap_LG_DAC",                        124+7 , 0, "LG shaper OTA bias. Nominal=4"))
        self.parameters.append(_parameter("bias/shap_ena",                           124+11, 0, "Shaper OTA bias enable (LG&HG)"))
        self.parameters.append(_parameter("bias/shap_HG_DAC",                        124+12, 0, "HG shaper OTA bias. Nominal=4"))
        self.parameters.append(_parameter("bias/delay_ena",                          124+16, 0, "Delay cell bias enable"))
        self.parameters.append(_parameter("bias/lfota_HG_DAC",                       124+17, 0, "High gain Low frequency feedback bias current. Nominal=2.\n 0 is maximum current"))
        self.parameters.append(_parameter("bias/lfota_LG_DAC",                       124+20, 0, "Low gain Low frequency feedback bias current. Nominal=2.\n 0 is maximum current"))
        self.parameters.append(_parameter("bias/subthbias_Rtrim",                    124+23, 0, "Subthreshold generator tuning, 2 taps. Nominal=00.\nValues 00,01,11. 11 gives steepest SiPM-DAC voltage ramp & highest current. States 01,10 are equivalent."))
        self.parameters.append(_parameter("bias/isgtriode_trim_ena_n",               124+25, 0, "Disable resistor DAC for triode tuning.\n Set to 1,DAC value 255 for minimum power consumption"))
        self.parameters.append(_parameter("bias/isgtriode_trim_DAC",                 124+26, 0, "Volta    ge DAC for input stage triode tuning"))
        self.parameters.append(_parameter("bias/isg_sdcomp_DAC",                     124+34, 0, "Input stage shutdown compensation current.\nHigher values: lower current\nTune together with sdbias and isgbias to minimize voltage offset between on and off states."))
        self.parameters.append(_parameter("bias/isg_sdbias_DAC",                     124+37, 0, "Input stage shutdown bias current.\nHigher values: lower current\nTune together with sdcomp and isgbias to minimize voltage offset between on and off states.\n Also changes input impedance in low power state."))
        self.parameters.append(_parameter("bias/comparator_th_gs",                   124+40, 0, "Gain selection threshold. Higher value, higher threshold."))
        self.parameters.append(_parameter("__filler",                                124+46, 0, ""))
        self.parameters.append(_parameter("bias/isg_biastune",                       124+51, 0, "Input stage bias current tuning."))
        self.parameters.append(_parameter("__filler",                                124+53, 0, ""))
        self.parameters.append(_parameter("bias/comparator_th_trig",                 124+54, 0, "Timing comparator threshold. Higher value, higher threshold.\nNoise should be around ~3-5; Fine tuning with channelwise DACs"))
        self.parameters.append(_parameter("bias/delay_DAC",                          124+60, 1,"Hold delay tuning DAC. Higher values gives larger current and smaller delay.\nDelay vs. current is hyperbolic."))
        self.parameters.append(_parameter("coincidence/coincidence_window",          188,    0, "Coincidence window configuration"))
        self.parameters.append(_parameter("coincidence/channel0/coincidence_mask",   191+0,  0, "Channel0 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel1/coincidence_mask",   191+10, 0, "Channel1 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel2/coincidence_mask",   191+20, 0, "Channel2 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel3/coincidence_mask",   191+30, 0, "Channel3 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel4/coincidence_mask",   191+40, 0, "Channel4 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel5/coincidence_mask",   191+50, 0, "Channel5 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel6/coincidence_mask",   191+60, 0, "Channel6 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel7/coincidence_mask",   191+70, 0, "Channel7 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel8/coincidence_mask",   191+80, 0, "Channel8 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel9/coincidence_mask",   281+ 0, 0, "Channel9 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel10/coincidence_mask",  281+10, 0, "Channel10 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel11/coincidence_mask",  281+20, 0, "Channel11 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel12/coincidence_mask",  281+30, 0, "Channel12 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel13/coincidence_mask",  281+40, 0, "Channel13 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel14/coincidence_mask",  281+50, 0, "Channel14 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel15/coincidence_mask",  281+60, 0, "Channel15 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel16/coincidence_mask",  281+70, 0, "Channel16 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel17/coincidence_mask",  281+80, 0, "Channel17 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel18/coincidence_mask",  371+ 0, 0, "Channel18 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel19/coincidence_mask",  371+10, 0, "Channel19 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel20/coincidence_mask",  371+20, 0, "Channel20 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel21/coincidence_mask",  371+30, 0, "Channel21 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel22/coincidence_mask",  371+40, 0, "Channel22 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel23/coincidence_mask",  371+50, 0, "Channel23 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel24/coincidence_mask",  371+60, 0, "Channel24 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel25/coincidence_mask",  371+70, 0, "Channel25 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel26/coincidence_mask",  371+80, 0, "Channel26 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel27/coincidence_mask",  461+ 0, 0, "Channel27 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel28/coincidence_mask",  461+10, 0, "Channel28 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel29/coincidence_mask",  461+20, 0, "Channel29 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel30/coincidence_mask",  461+30, 0, "Channel30 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel31/coincidence_mask",  461+40, 0, "Channel31 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel32/coincidence_mask",  461+50, 0, "Channel32 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel33/coincidence_mask",  461+60, 0, "Channel33 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel34/coincidence_mask",  461+70, 0, "Channel34 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("coincidence/channel35/coincidence_mask",  461+80, 0, "Channel35 coincidence mask, bit 0 is ignored"))
        self.parameters.append(_parameter("digital/trigger_or_enable",               551,    0, "Enable or-ed trigger output"))
        self.parameters.append(_parameter("digital/powerdown_pipe",                  552,    0, "Power down ADC bias when idle"))
        self.parameters.append(_parameter("digital/powerdown_sar",                   553,    0, "Power down ADC bias when idle"))
        self.parameters.append(_parameter("digital/pipemode_enable",                 554,    0, "Enable 12b conversion mode"))
        # Channel: 22 parameters
        # channel pattern definition, includes fe/adc/channel-digital. Total 53 bits
        # For the FE, bit 18,19 was added in KLaus5
        for i in range(36):
            self.parameters.append(_parameter("channel"+str(i)+"/mask",                  555+53*i+0,     0, "Channel mask"))
            self.parameters.append(_parameter("channel"+str(i)+"/ADC_PIPEamp_comp_ena",  555+53*i+1+0,   0, "Enable pipeline amplifier mismatch compensation current"))
            self.parameters.append(_parameter("channel"+str(i)+"/ADC_PIPEamp_comp_dac",  555+53*i+1+1,   0, "Pipeline amplifier compensation DAC"))
            self.parameters.append(_parameter("__filler",                                555+53*i+1+5,   0, ""))
            self.parameters.append(_parameter("__filler",                                555+53*i+1+10,  0, ""))
            self.parameters.append(_parameter("channel"+str(i)+"/ADC_captrim",           555+53*i+1+16,  0, "Pipeline feedback capacitor tuning DAC"))
            self.parameters.append(_parameter("channel"+str(i)+"/trigger_th_finetune",   555+53*i+20+1,  0, "Comparator threshold finetuning.\n Higher values => Higher threshold."))
            self.parameters.append(_parameter("channel"+str(i)+"/gainsel_busy_enable",   555+53*i+20+5,  0, "1) Disable output of gain selection result while ADC is busy.\nNominal 0"))
            self.parameters.append(_parameter("channel"+str(i)+"/hold_delay_finetune",   555+53*i+20+6,  0, "Hold delay finetuning DAC. Nominal: 0x01"))
            self.parameters.append(_parameter("channel"+str(i)+"/branch_sel_config",     555+53*i+20+10, 0, "Branch selection configuration\n 0) Auto HG/LG ; 1)External source ; 2) LG force ; 3) HG force"))
            self.parameters.append(_parameter("channel"+str(i)+"/ext_trigger_select",    555+53*i+20+12, 0, "Enable external trigger for ADC conversion. Set to 1 for external trigger, need to put global threshold to 0"))
            self.parameters.append(_parameter("__filler",                                555+53*i+20+13, 0, ""))
            self.parameters.append(_parameter("channel"+str(i)+"/HG_scale_select",       555+53*i+20+14, 0, "High gain scaling. 1) 1:1; 0) 1:7"))
            self.parameters.append(_parameter("channel"+str(i)+"/ADC_in_n_select",       555+53*i+20+15, 0, "ADCn selection.\n 1) Internal GNDA ; 0) From Pad ADCn"))
            self.parameters.append(_parameter("channel"+str(i)+"/lfota_disable",         555+53*i+20+16, 0, "Disables Low Frequency feedback amplifier bias"))
            self.parameters.append(_parameter("channel"+str(i)+"/monitor_HG_disable",    555+53*i+20+17, 0, "Disables Analog monitor feedthrough from HG branch"))
            self.parameters.append(_parameter("channel"+str(i)+"/monitor_LG_disable",    555+53*i+20+18, 0, "Disables Analog monitor feedthrough from LG branch"))
            self.parameters.append(_parameter("channel"+str(i)+"/trigger_LSB_scale",     555+53*i+20+19, 0, "Scale the trigger comparator's LSB by 0 (in KLauS4 = 1)"))
            self.parameters.append(_parameter("channel"+str(i)+"/LG_scale_select",       555+53*i+20+20, 0, "Low gain scaling. 1) 1:100?; 0). 1:40 (KLauS4)"))
            self.parameters.append(_parameter("__filler",                                555+53*i+20+21, 0, ""))
            self.parameters.append(_parameter("channel"+str(i)+"/sw_triodesel",          555+53*i+20+24, 0, "Select input stage triode bias.\n0) Internal selfbias (default);1) Control by DAC"))
            self.parameters.append(_parameter("channel"+str(i)+"/vDAC_SiPM",             555+53*i+20+25, 0, "Input terminal voltage tuning DAC"))
        # None
        self.parameters.append(_parameter("", 2463, 0, ""))
        self.bitpattern_write = bytearray(self.GetPatternByteLength())

    def __str__(self):
        string = str()
        for element in self.parameters:
            string += '(%s, %d, %d, %s)\n' % (element.name, element.offset, element.endianess, element.description)
        return string

    def GetPatternBitLength(self):
        # get length of pattern in bits
        return self.parameters[len(self.parameters)-1].offset

    def GetPatternByteLength(self):
        # Length of the bit pattern in bytes
        return math.ceil(self.parameters[len(self.parameters)-1].offset / 8)

    def GetParID(self, name):
        ID=-1;
        npar = 0
        for element in self.parameters:
            if(element.name == name):
                ID=npar
                break
            else:
                npar += 1
        return ID

    def GetParName(self, index):
        if (index>=0)&(index<len(self.parameters)):
            return self.parameters[index].name
        else:    
            return 0

    def GetParDescription(self, index):
        if (index>=0)&(index<len(self.parameters)):
            return self.parameters[index].description
        else:    
            return 0

    def ParIsFiller(self, index):
        # checks if the current parameter ID refers to a filler with no connections in the ASIC.
        # returns true if first character is a '_'
        return self.GetParName(index)[0]=='_'

    def GetFromPattern(self, index):
        temp=0 #temporary value read from pattern
        pos=self.parameters[index].offset
        while (pos<self.parameters[index+1].offset): #loop over bits in _parameter
            if ((self.bitpattern_write[pos//8] & ( 1<< (pos%8))) !=0 ):
                if (self.parameters[index].endianess==0):
                    temp+=(1<<(pos-self.parameters[index].offset))
                else:
                    temp+=(1<<(self.parameters[index+1].offset-1-pos))
            pos +=1
        return temp

    def SetInPattern(self, index, value):
        mask=0x01;
        if (self.parameters[index].endianess==0):
            pos=self.parameters[index].offset
            while (pos<self.parameters[index+1].offset):
                if ((value & mask)!=0):    # bit is set in _parameter
                    self.bitpattern_write[pos//8]|= (0x01<<(pos%8))
                else:
                    self.bitpattern_write[pos//8]&=~(0x01<<(pos%8))
                pos += 1
                mask<<=1
        else:
            pos=self.parameters[index+1].offset-1
            while (pos>=self.parameters[index].offset): #loop over bits in _parameter
                if ((value & mask)!=0):    # bit is set in _parameter
                    self.bitpattern_write[pos//8]|= (0x01<<(pos%8))
                else:
                    self.bitpattern_write[pos//8]&=~(0x01<<(pos%8))
                pos -= 1;
                mask<<=1;

    def GetParValue(self, name):
        npar = self.GetParID(name)
        if (npar<0) | (npar>=len(self.parameters)):
            return -1
        return self.GetFromPattern(npar)

    def SetParValue(self, name, value):
        npar = self.GetParID(name)
        if (npar<0) | (npar>=len(self.parameters)):
            return -1
        self.SetInPattern(npar,value)
        return npar

    def ReadFromFile(self, filename = "config_default.txt"):
        try:
            ifile = open(filename, "r")
        except IOError:
            print("Configuration File "+filename+" could not be opened. Leaving Standard Values.")
            return -1
        line = ifile.readline()
        i = 0;
        while (line!=''):
            line = line.replace(" ", "").split("=", 1)
            parname = line[0]
            value = eval(line[1]) 
            if (self.SetParValue(parname,value)<0): 
                print("Parameter \""+parname+"\" not recognized.")
            line = ifile.readline()
            i += 1
        print("Read config from: "+filename+", "+str(i)+" parameters are processed")
        ifile.close()
        return 1

    def SaveToFile(self, filename = "config.txt"):
        try:
            ofile = open(filename, "w")
        except IOError:
            print("Configuration File "+filename+" could not be opened. Leaving Standard Values.")
            return -1
        for npar in range(len(self.parameters)-1):
            if self.ParIsFiller(npar):
                continue
            ofile.write(self.GetParName(npar)+" = "+hex(self.GetFromPattern(npar))+'\n')
        ofile.close()
        return 1

    def Writebitcode(self):
        ptr = len(self.bitpattern_write)
        tx_buffer = bytearray(ptr)
        for byte in self.bitpattern_write:
            tx_buffer[ptr-1] = byte
            ptr -= 1
        return tx_buffer
