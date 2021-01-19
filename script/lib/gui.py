#!/usr/bin/python
# -*- coding: UTF-8 -*-
# author: zhj@ihep.ac.cn

import tkinter as tk
import tkinter.ttk as ttk
import tkinter.filedialog

import lib
from lib import klaus6config
from lib import interface

class _element:
    def __init__(self, text, value = 0, master = 0, identity = 0, row = 0, column = 0, rowspan = 1, columnspan = 1, sticky = 's'):
        self.text = text
        self.value = value, 
        self.master = master
        self.identity = identity
        self.row = row
        self.column = column
        self.rowspan = rowspan
        self.columnspan = columnspan
        self.sticky = sticky

class TGui:
    def __init__(self):
        self.configuration = klaus6config.TVirtualConfig() 
        # print(configuration)
        self.ifac = interface.interface()

        self.app = tk.Tk()
        # self.app.geometry('800x600')
        self.app.title('KLaus6 Config GUI')
        notebook = ttk.Notebook(self.app)

        #####################################################################
        tabCD = tk.Frame()
        notebook.add(tabCD, text='Common Digital')
        tabCD.grid_columnconfigure(0, minsize=250)
        tabCD.grid_columnconfigure(2, minsize=200)
        tabCD.grid_columnconfigure(5, minsize=150)
        CDcolumnspan = 6
        tk.Label(tabCD, font="bold", text='Flags').grid(row=0,  columnspan=CDcolumnspan)
        ttk.Separator(tabCD, orient='horizontal').grid(row=4,  sticky='e'+'w', columnspan=CDcolumnspan)
        tk.Label(tabCD, font="bold", text='Readout links configuration').grid(row=5,  columnspan=CDcolumnspan)
        ttk.Separator(tabCD, orient='horizontal').grid(row=8,  sticky='e'+'w', columnspan=CDcolumnspan)
        tk.Label(tabCD, font="bold", text='LVDS configuration').grid(row=9,  columnspan=CDcolumnspan)
        ttk.Separator(tabCD, orient='horizontal').grid(row=12, sticky='e'+'w', columnspan=CDcolumnspan)
        tk.Label(tabCD, font="bold", text='PLL & TDC configuration').grid(row=13, columnspan=CDcolumnspan)
        
        #####################################################################
        tabPC = tk.Frame()
        notebook.add(tabPC, text='Power Gating/Coincidence')
        PCcolumnspan = 12
        tabPC_up = tk.Frame(tabPC)
        tabPC_up.pack(fill='both', expand=True)
        tk.Label(tabPC_up, font="bold", text='Power Gating Control').grid(row=0, columnspan=PCcolumnspan)
        tabPC_middle = tk.Frame(tabPC)
        tabPC_middle.pack(fill='both', expand=True)
        ttk.Separator(tabPC_middle, orient='horizontal').grid(row=5,  sticky='e'+'w', columnspan=PCcolumnspan)
        tk.Label(tabPC_middle, font="bold", text='Coincidence/L0 FIFO validation').grid(row=6,  columnspan=PCcolumnspan//2)
        tk.Label(tabPC_middle, font="bold", text='Coincidence/L1 FIFO validation').grid(row=6, column=PCcolumnspan//2, columnspan=PCcolumnspan//2)
        tabPC_sub = tk.Frame(tabPC)
        tabPC_sub.pack(fill='both', expand=True)
        tabPC_canvas = tk.Canvas(tabPC_sub)
        tabPC_scroll = tk.Scrollbar(tabPC_sub, command=tabPC_canvas.yview)
        tabPC_canvas.config(yscrollcommand=tabPC_scroll.set, scrollregion=(0,0,100,1000))
        tabPC_canvas.pack(side='left', fill='both', expand=True)
        tabPC_scroll.pack(side='right', fill='y')
        tabPC_down = tk.Frame(tabPC_canvas)
        tabPC_down.pack(fill='both', expand=True)
        tabPC_canvas.create_window(400, 480, window=tabPC_down)

        #####################################################################
        tabGB = tk.Frame()
        notebook.add(tabGB, text='Global Bias')
        tabGB.grid_columnconfigure(0, minsize=200)
        tabGB.grid_columnconfigure(2, minsize=200)
        tabGB.grid_columnconfigure(5, minsize=150)
        GBcolumnspan = 6
        tk.Label(tabGB, font="bold", text='Input Stage Bias').grid(row=0, column=0, columnspan=GBcolumnspan//2)
        tk.Label(tabGB, font="bold", text='Input Stage Triode Tuning(Advanced)').grid(row=0, column=GBcolumnspan//2, columnspan=GBcolumnspan//2)
        ttk.Separator(tabGB, orient='horizontal').grid(row=4,  sticky='e'+'w', columnspan=GBcolumnspan)
        tk.Label(tabGB, font="bold", text='Shapers').grid(row=5, column=0, columnspan=GBcolumnspan//2)
        tk.Label(tabGB, font="bold", text='Low-frequency feedback').grid(row=5, column=GBcolumnspan//2, columnspan=GBcolumnspan//2)
        ttk.Separator(tabGB, orient='horizontal').grid(row=9,  sticky='e'+'w', columnspan=GBcolumnspan)
        tk.Label(tabGB, font="bold", text='Comparators').grid(row=10, column=0, columnspan=GBcolumnspan//2)
        tk.Label(tabGB, font="bold", text='Hold Delay').grid(row=10, column=GBcolumnspan//2, columnspan=GBcolumnspan//2)
        ttk.Separator(tabGB, orient='horizontal').grid(row=13, sticky='e'+'w', columnspan=GBcolumnspan)
        tk.Label(tabGB, font="bold", text='Bias Resistor Tuning').grid(row=14, column=0, columnspan=GBcolumnspan//2)
        tk.Label(tabGB, font="bold", text='Analog Monitor Bias').grid(row=14, column=GBcolumnspan//2, columnspan=GBcolumnspan//2)
 
        #####################################################################
        tabCH = tk.Frame()
        notebook.add(tabCH, text='Channel')
        tabCH.grid_columnconfigure(0, minsize=200)
        tabCH.grid_columnconfigure(4, minsize=200)
        tabCH.grid_columnconfigure(9, minsize=150)
        GBcolumnspan = 10
        tk.Label(tabCH, font="bold", text='Channel Flags').grid(row=0, column=0, columnspan=GBcolumnspan//2)
        tk.Label(tabCH, font="bold", text='Output & Dynamic range selection').grid(row=0, column=GBcolumnspan//2, columnspan=GBcolumnspan//2)
        tk.Label(tabCH, anchor="w", text='0) Auto HG/LG').grid(row=1, column=GBcolumnspan-1)
        tk.Label(tabCH, anchor="w", text='1) External').grid(row=2, column=GBcolumnspan-1)
        tk.Label(tabCH, anchor="w", text='2) LG force').grid(row=3, column=GBcolumnspan-1)
        tk.Label(tabCH, anchor="w", text='3) HG force').grid(row=4, column=GBcolumnspan-1)
        ttk.Separator(tabCH, orient='horizontal').grid(row=5,  sticky='e'+'w', columnspan=GBcolumnspan)
        tk.Label(tabCH, font="bold", text='DACs').grid(row=6, column=0, columnspan=GBcolumnspan)
        ttk.Separator(tabCH, orient='horizontal').grid(row=10,  sticky='e'+'w', columnspan=GBcolumnspan)
        tk.Label(tabCH, font="bold", text='Pipelined ADC configuration').grid(row=11, column=0, columnspan=GBcolumnspan//2)
        tk.Label(tabCH, font="bold", text='Advanced/Debug').grid(row=11, column=GBcolumnspan//2, columnspan=GBcolumnspan//2)
        ttk.Separator(tabCH, orient='horizontal').grid(row=15, sticky='e'+'w', columnspan=GBcolumnspan)

        tk.Label(tabCH, font="bold", text='Channel:', anchor="e").grid(row=16, column=0)
        self.cur_channel = tk.StringVar()
        self.cur_channel.set('0')
        combobox_ch = ttk.Combobox(tabCH, state='readonly', textvariable=self.cur_channel, width = 10, height = 10, font="bold",
            values=['0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '10', '11', '12', '13', '14', '15', '16', '17', \
                    '18', '19', '20', '21', '22', '23', '24', '25', '26', '27', '28', '29', '30', '31', '32', '33', '34', '35'])
        combobox_ch.bind('<<ComboboxSelected>>', self.update_channel)
        combobox_ch.grid(row=16, column = 1, sticky='e'+'w')
        self.pre_channel = tk.StringVar(tabCH)
        self.pre_channel.set('0')
        
        #####################################################################
        self.elements = []
        for config in self.configuration.parameters:
            name = config.name.replace("_", " ").split("/", 1)
            if len(name)>1:
                self.elements.append(_element(text = name[1][0].upper()+name[1][1:]))
            else:
                self.elements.append(_element(text = '__filler'))

        self.elements[0].master  = tabCD # digital/clk_div_sel
        self.elements[1].master  = tabPC_middle # coincidence/powergate_time_1
        self.elements[2].master  = tabPC_middle # coincidence/powergate_time_2
        self.elements[3].master  = tabPC_middle # coincidence/powergate_time_3
        self.elements[4].master  = tabPC_middle # coincidence/powergate_time_4
        self.elements[5].master  = tabPC_middle # coincidence/powergate_time_5
        self.elements[6].master  = tabPC_up # coincidence/powergate_do_gate
        self.elements[7].master  = tabPC_up # coincidence/powergate_enable
        self.elements[8].master  = tabCD # digital/i2c_address
        self.elements[9].master  = tabCD # digital/debug_pad_enable
        self.elements[10].master = tabCD # digital/i2c_readout_enable
        self.elements[11].master = tabCD # digital/LVDS_serdata_tx_iDAC
        self.elements[12].master = tabCD # digital/LVDS_serdata_tx_vcmDAC
        self.elements[13].master = tabCD # digital/LVDS_serdata_tx_enable
        self.elements[14].master = tabCD # digital/generate_8b10b_idle
        self.elements[15].master = tabCD # digital/TDC_factory_mode
        self.elements[16].master = tabCD # digital/TDC_offset_middle
        self.elements[17].master = tabCD # digital/TDC_offset_fine
        self.elements[18].master = tabCD # digital/TDC_offset_enable
        self.elements[19].master = tabGB # bias/ADC_rvb
        self.elements[20].master = tabCD # digital/T0channel_mask
        self.elements[21].master = tabPC_middle # coincidence/L1_trigger_pretime
        self.elements[22].master = tabPC_middle # coincidence/L1_trigger_pretime_sign
        self.elements[23].master = tabPC_middle # coincidence/L1_trigger_posttime
        self.elements[24].master = tabPC_middle # coincidence/L1_trigger_mode
        self.elements[25].master = tabCD # digital/PLL_resistor
        self.elements[26].master = tabCD # digital/PLL_Icp
        self.elements[27].master = tabCD # digital/PLL_Ibuf
        self.elements[28].master = tabGB # bias/amon_DAC
        self.elements[29].master = tabGB # bias/amon_ena
        self.elements[30].master = tabGB # bias/shap_LG_DAC
        self.elements[31].master = tabGB # bias/shap_ena
        self.elements[32].master = tabGB # bias/shap_HG_DAC
        self.elements[33].master = tabGB # bias/delay_ena
        self.elements[34].master = tabGB # bias/lfota_HG_DAC
        self.elements[35].master = tabGB # bias/lfota_LG_DAC
        self.elements[36].master = tabGB # bias/subthbias_Rtrim
        self.elements[37].master = tabGB # bias/isgtriode_trim_ena_n
        self.elements[38].master = tabGB # bias/isgtriode_trim_DAC
        self.elements[39].master = tabGB # bias/isg_sdcomp_DAC
        self.elements[40].master = tabGB # bias/isg_sdbias_DAC
        self.elements[41].master = tabGB # bias/comparator_th_gs
        self.elements[42].master = tabCD # __filler
        self.elements[43].master = tabGB # bias/isg_biastune
        self.elements[44].master = tabCD # __filler
        self.elements[45].master = tabGB # bias/comparator_th_trig
        self.elements[46].master = tabGB # bias/delay_DAC
        self.elements[47].master = tabPC_middle # coincidence/coincidence_window
        self.elements[48].master = tabPC_down # coincidence/channel0/coincidence_mask
        self.elements[49].master = tabPC_down # coincidence/channel1/coincidence_mask
        self.elements[50].master = tabPC_down # coincidence/channel2/coincidence_mask
        self.elements[51].master = tabPC_down # coincidence/channel3/coincidence_mask
        self.elements[52].master = tabPC_down # coincidence/channel4/coincidence_mask
        self.elements[53].master = tabPC_down # coincidence/channel5/coincidence_mask
        self.elements[54].master = tabPC_down # coincidence/channel6/coincidence_mask
        self.elements[55].master = tabPC_down # coincidence/channel7/coincidence_mask
        self.elements[56].master = tabPC_down # coincidence/channel8/coincidence_mask
        self.elements[57].master = tabPC_down # coincidence/channel9/coincidence_mask
        self.elements[58].master = tabPC_down # coincidence/channel10/coincidence_mask
        self.elements[59].master = tabPC_down # coincidence/channel11/coincidence_mask
        self.elements[60].master = tabPC_down # coincidence/channel12/coincidence_mask
        self.elements[61].master = tabPC_down # coincidence/channel13/coincidence_mask
        self.elements[62].master = tabPC_down # coincidence/channel14/coincidence_mask
        self.elements[63].master = tabPC_down # coincidence/channel15/coincidence_mask
        self.elements[64].master = tabPC_down # coincidence/channel16/coincidence_mask
        self.elements[65].master = tabPC_down # coincidence/channel17/coincidence_mask
        self.elements[66].master = tabPC_down # coincidence/channel18/coincidence_mask
        self.elements[67].master = tabPC_down # coincidence/channel19/coincidence_mask
        self.elements[68].master = tabPC_down # coincidence/channel20/coincidence_mask
        self.elements[69].master = tabPC_down # coincidence/channel21/coincidence_mask
        self.elements[70].master = tabPC_down # coincidence/channel22/coincidence_mask
        self.elements[71].master = tabPC_down # coincidence/channel23/coincidence_mask
        self.elements[72].master = tabPC_down # coincidence/channel24/coincidence_mask
        self.elements[73].master = tabPC_down # coincidence/channel25/coincidence_mask
        self.elements[74].master = tabPC_down # coincidence/channel26/coincidence_mask
        self.elements[75].master = tabPC_down # coincidence/channel27/coincidence_mask
        self.elements[76].master = tabPC_down # coincidence/channel28/coincidence_mask
        self.elements[77].master = tabPC_down # coincidence/channel29/coincidence_mask
        self.elements[78].master = tabPC_down # coincidence/channel30/coincidence_mask
        self.elements[79].master = tabPC_down # coincidence/channel31/coincidence_mask
        self.elements[80].master = tabPC_down # coincidence/channel32/coincidence_mask
        self.elements[81].master = tabPC_down # coincidence/channel33/coincidence_mask
        self.elements[82].master = tabPC_down # coincidence/channel34/coincidence_mask
        self.elements[83].master = tabPC_down # coincidence/channel35/coincidence_mask
        self.elements[84].master = tabCD # digital/trigger_or_enable
        self.elements[85].master = tabCD # digital/powerdown_pipe
        self.elements[86].master = tabCD # digital/powerdown_sar
        self.elements[87].master = tabCD # digital/pipemode_enable
        # Channel: 22 elements
        self.elements[88+0].master  = tabCH # channel"+str(i)+"/mask
        self.elements[88+1].master  = tabCH # channel"+str(i)+"/ADC_PIPEamp_comp_ena
        self.elements[88+2].master  = tabCH # channel"+str(i)+"/ADC_PIPEamp_comp_dac
        self.elements[88+3].master  = tabCH # __filler
        self.elements[88+4].master  = tabCH # __filler
        self.elements[88+5].master  = tabCH # channel"+str(i)+"/ADC_captrim
        self.elements[88+6].master  = tabCH # channel"+str(i)+"/trigger_th_finetune
        self.elements[88+7].master  = tabCH # channel"+str(i)+"/gainsel_busy_enable
        self.elements[88+8].master  = tabCH # channel"+str(i)+"/hold_delay_finetune
        self.elements[88+9].master  = tabCH # channel"+str(i)+"/branch_sel_config
        self.elements[88+10].master = tabCH # channel"+str(i)+"/ext_trigger_select
        self.elements[88+11].master = tabCH # __filler
        self.elements[88+12].master = tabCH # channel"+str(i)+"/HG_scale_select
        self.elements[88+13].master = tabCH # channel"+str(i)+"/ADC_in_n_select
        self.elements[88+14].master = tabCH # channel"+str(i)+"/lfota_disable
        self.elements[88+15].master = tabCH # channel"+str(i)+"/monitor_HG_disable
        self.elements[88+16].master = tabCH # channel"+str(i)+"/monitor_LG_disable
        self.elements[88+17].master = tabCH # channel"+str(i)+"/trigger_LSB_scale
        self.elements[88+18].master = tabCH # channel"+str(i)+"/LG_scale_select
        self.elements[88+19].master = tabCH # __filler
        self.elements[88+20].master = tabCH # channel"+str(i)+"/sw_triodesel
        self.elements[88+21].master = tabCH # channel"+str(i)+"/vDAC_SiPM

        # value
        self.elements[ 0].value = tk.IntVar(self.elements[ 0].master) # digital/clk_div_sel
        self.elements[ 1].value = tk.StringVar(self.elements[ 1].master) # coincidence/powergate_time_1
        self.elements[ 2].value = tk.StringVar(self.elements[ 2].master) # coincidence/powergate_time_2
        self.elements[ 3].value = tk.StringVar(self.elements[ 3].master) # coincidence/powergate_time_3
        self.elements[ 4].value = tk.StringVar(self.elements[ 4].master) # coincidence/powergate_time_4
        self.elements[ 5].value = tk.StringVar(self.elements[ 5].master) # coincidence/powergate_time_5
        self.elements[ 6].value = [
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master),
            tk.IntVar(self.elements[6].master)
        ]# coincidence/powergate_do_gate
        self.elements[ 7].value = [
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master),
            tk.IntVar(self.elements[7].master)
        ] # coincidence/powergate_enable
        self.elements[ 8].value = tk.StringVar(self.elements[ 8].master) # digital/i2c_address
        self.elements[ 9].value = tk.IntVar(self.elements[ 9].master) # digital/debug_pad_enable
        self.elements[10].value = tk.IntVar(self.elements[10].master) # digital/i2c_readout_enable
        self.elements[11].value = tk.StringVar(self.elements[11].master) # digital/LVDS_serdata_tx_iDAC
        self.elements[12].value = tk.StringVar(self.elements[12].master) # digital/LVDS_serdata_tx_vcmDAC
        self.elements[13].value = tk.IntVar(self.elements[13].master) # digital/LVDS_serdata_tx_enable
        self.elements[14].value = tk.IntVar(self.elements[14].master) # digital/generate_8b10b_idle
        self.elements[15].value = tk.IntVar(self.elements[15].master) # digital/TDC_factory_mode
        self.elements[16].value = tk.StringVar(self.elements[16].master) # digital/TDC_offset_middle
        self.elements[17].value = tk.StringVar(self.elements[17].master) # digital/TDC_offset_fine
        self.elements[18].value = tk.IntVar(self.elements[18].master) # digital/TDC_offset_enable
        self.elements[19].value = tk.StringVar(self.elements[19].master) # bias/ADC_rvb
        self.elements[20].value = tk.IntVar(self.elements[20].master) # digital/T0channel_mask
        self.elements[21].value = tk.StringVar(self.elements[21].master) # coincidence/L1_trigger_pretime
        self.elements[22].value = tk.IntVar(self.elements[22].master) # coincidence/L1_trigger_pretime_sign
        self.elements[23].value = tk.StringVar(self.elements[23].master) # coincidence/L1_trigger_posttime
        self.elements[24].value = tk.IntVar(self.elements[24].master) # coincidence/L1_trigger_mode
        self.elements[25].value = tk.StringVar(self.elements[25].master) # digital/PLL_resistor
        self.elements[26].value = tk.StringVar(self.elements[26].master) # digital/PLL_Icp
        self.elements[27].value = tk.StringVar(self.elements[27].master) # digital/PLL_Ibuf
        self.elements[28].value = tk.StringVar(self.elements[28].master) # bias/amon_DAC
        self.elements[29].value = tk.IntVar(self.elements[29].master) # bias/amon_ena
        self.elements[30].value = tk.StringVar(self.elements[30].master) # bias/shap_LG_DAC
        self.elements[31].value = tk.IntVar(self.elements[31].master) # bias/shap_ena
        self.elements[32].value = tk.StringVar(self.elements[32].master) # bias/shap_HG_DAC
        self.elements[33].value = tk.IntVar(self.elements[33].master) # bias/delay_ena
        self.elements[34].value = tk.StringVar(self.elements[34].master) # bias/lfota_HG_DAC
        self.elements[35].value = tk.StringVar(self.elements[35].master) # bias/lfota_LG_DAC
        self.elements[36].value = tk.StringVar(self.elements[36].master) # bias/subthbias_Rtrim
        self.elements[37].value = tk.IntVar(self.elements[37].master) # bias/isgtriode_trim_ena_n
        self.elements[38].value = tk.StringVar(self.elements[38].master) # bias/isgtriode_trim_DAC
        self.elements[39].value = tk.StringVar(self.elements[39].master) # bias/isg_sdcomp_DAC
        self.elements[40].value = tk.StringVar(self.elements[40].master) # bias/isg_sdbias_DAC
        self.elements[41].value = tk.StringVar(self.elements[41].master) # bias/comparator_th_gs
        self.elements[42].value = tk.IntVar(self.elements[42].master) # __filler
        self.elements[43].value = tk.StringVar(self.elements[43].master) # bias/isg_biastune
        self.elements[44].value = tk.IntVar(self.elements[44].master) # __filler
        self.elements[45].value = tk.StringVar(self.elements[45].master) # bias/comparator_th_trig
        self.elements[46].value = tk.StringVar(self.elements[46].master) # bias/delay_DAC
        self.elements[47].value = tk.StringVar(self.elements[47].master) # coincidence/coincidence_window
        # coincidence/channel0-35/coincidence_mask
        for i in range(36):
            self.elements[48+i].value = [
                tk.IntVar(self.elements[48+i].master),
                tk.IntVar(self.elements[48+i].master),
                tk.IntVar(self.elements[48+i].master),
                tk.IntVar(self.elements[48+i].master),
                tk.IntVar(self.elements[48+i].master),
                tk.IntVar(self.elements[48+i].master),
                tk.IntVar(self.elements[48+i].master),
                tk.IntVar(self.elements[48+i].master),
                tk.IntVar(self.elements[48+i].master),
                tk.IntVar(self.elements[48+i].master)
            ]
        self.elements[84].value = tk.IntVar(self.elements[84].master) # digital/trigger_or_enable
        self.elements[85].value = tk.IntVar(self.elements[85].master) # digital/powerdown_pipe
        self.elements[86].value = tk.IntVar(self.elements[86].master) # digital/powerdown_sar
        self.elements[87].value = tk.IntVar(self.elements[87].master) # digital/pipemode_enable
        # Channel: 22 elements
        self.elements[ 88].value = tk.IntVar(self.elements[ 88].master) # channel/mask
        self.elements[ 89].value = tk.IntVar(self.elements[ 89].master) # channel/ADC_PIPEamp_comp_ena
        self.elements[ 90].value = tk.StringVar(self.elements[ 90].master) # channel/ADC_PIPEamp_comp_dac
        self.elements[ 91].value = tk.IntVar(self.elements[ 91].master) # __filler
        self.elements[ 92].value = tk.IntVar(self.elements[ 92].master) # __filler
        self.elements[ 93].value = tk.StringVar(self.elements[ 93].master) # channel/ADC_captrim
        self.elements[ 94].value = tk.StringVar(self.elements[ 94].master) # channel/trigger_th_finetune
        self.elements[ 95].value = tk.IntVar(self.elements[ 95].master) # channel/gainsel_busy_enable
        self.elements[ 96].value = tk.StringVar(self.elements[ 96].master) # channel/hold_delay_finetune
        self.elements[ 97].value = tk.StringVar(self.elements[ 97].master) # channel/branch_sel_config
        self.elements[ 98].value = tk.IntVar(self.elements[ 98].master) # channel/ext_trigger_select
        self.elements[ 99].value = tk.IntVar(self.elements[ 99].master) # __filler
        self.elements[100].value = tk.IntVar(self.elements[100].master) # channel/HG_scale_select
        self.elements[101].value = tk.IntVar(self.elements[101].master) # channel/ADC_in_n_select
        self.elements[102].value = tk.IntVar(self.elements[102].master) # channel/lfota_disable
        self.elements[103].value = tk.IntVar(self.elements[103].master) # channel/monitor_HG_disable
        self.elements[104].value = tk.IntVar(self.elements[104].master) # channel/monitor_LG_disable
        self.elements[105].value = tk.IntVar(self.elements[105].master) # channel/trigger_LSB_scale
        self.elements[106].value = tk.IntVar(self.elements[106].master) # channel/LG_scale_select
        self.elements[107].value = tk.IntVar(self.elements[107].master) # __filler
        self.elements[108].value = tk.IntVar(self.elements[108].master) # channel/sw_triodesel
        self.elements[109].value = tk.StringVar(self.elements[109].master) # channel/vDAC_SiPM

        # location and identity
        # digital/clk_div_sel
        self.elements[0].identity  = tk.Scale(self.elements[0].master, variable = self.elements[0].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[0].row = 10
        self.elements[0].column = 1
        # coincidence/powergate_time_1
        self.elements[1].identity  = tk.Spinbox(self.elements[1].master, textvariable = self.elements[1].value, width=4, from_=0, to=7) 
        self.elements[1].row = 4
        self.elements[1].column = 1
        # coincidence/powergate_time_2
        self.elements[2].identity  = tk.Spinbox(self.elements[2].master, textvariable = self.elements[2].value, width=4, from_=0, to=1023)  
        self.elements[2].row = 4
        self.elements[2].column = 3
        # coincidence/powergate_time_3
        self.elements[3].identity  = tk.Spinbox(self.elements[3].master, textvariable = self.elements[3].value, width=4, from_=0, to=1023)  
        self.elements[3].row = 4
        self.elements[3].column = 5
        # coincidence/powergate_time_4
        self.elements[4].identity  = tk.Spinbox(self.elements[4].master, textvariable = self.elements[4].value, width=4, from_=0, to=1023) 
        self.elements[4].row = 4
        self.elements[4].column = 7
        # coincidence/powergate_time_5
        self.elements[5].identity  = tk.Spinbox(self.elements[5].master, textvariable = self.elements[5].value, width=4, from_=0, to=1023)  
        self.elements[5].row = 4
        self.elements[5].column = 9
        # coincidence/powergate_do_gate
        self.elements[6].identity  = [
            tk.Checkbutton(self.elements[6].master, text = "CLK",       variable = self.elements[6].value[10], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "CLK RX",    variable = self.elements[6].value[ 9], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "T0 RX",     variable = self.elements[6].value[ 8], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "FIFO RX",   variable = self.elements[6].value[ 7], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "FE RX",     variable = self.elements[6].value[ 6], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "TDC",       variable = self.elements[6].value[ 5], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "Mask",      variable = self.elements[6].value[ 4], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "Idelay",    variable = self.elements[6].value[ 3], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "LFOTA Inj", variable = self.elements[6].value[ 2], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "ISG Inj",   variable = self.elements[6].value[ 1], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[6].master, text = "ISG",       variable = self.elements[6].value[ 0], onvalue = 1, offvalue = 0)
        ] 
        self.elements[6].row = 2
        self.elements[6].column = 1
        # coincidence/powergate_enable
        self.elements[7].identity  = [
            tk.Checkbutton(self.elements[7].master, text = "CLK",       variable = self.elements[7].value[10], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "CLK RX",    variable = self.elements[7].value[ 9], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "T0 RX",     variable = self.elements[7].value[ 8], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "FIFO RX",   variable = self.elements[7].value[ 7], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "FE RX",     variable = self.elements[7].value[ 6], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "TDC",       variable = self.elements[7].value[ 5], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "Mask",      variable = self.elements[7].value[ 4], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "Idelay",    variable = self.elements[7].value[ 3], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "LFOTA Inj", variable = self.elements[7].value[ 2], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "ISG Inj",   variable = self.elements[7].value[ 1], onvalue = 1, offvalue = 0), 
            tk.Checkbutton(self.elements[7].master, text = "ISG",       variable = self.elements[7].value[ 0], onvalue = 1, offvalue = 0)
        ]
        self.elements[7].row = 1
        self.elements[7].column = 1
        # digital/i2c_address
        self.elements[8].identity  = tk.Spinbox(self.elements[8].master, textvariable = self.elements[8].value, width=4, from_=0, to=127) 
        self.elements[8].row = 6
        self.elements[8].column = 1
        # digital/debug_pad_enable
        self.elements[9].identity  = tk.Scale(self.elements[9].master, variable = self.elements[9].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[9].row = 3
        self.elements[9].column = 1
        # digital/i2c_readout_enable
        self.elements[10].identity = tk.Scale(self.elements[10].master, variable = self.elements[10].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[10].row = 7
        self.elements[10].column = 1
        # digital/LVDS_serdata_tx_iDAC
        self.elements[11].identity = tk.Spinbox(self.elements[11].master, textvariable = self.elements[11].value, width=4, from_=0, to=63) 
        self.elements[11].row = 11
        self.elements[11].column = 4
        # digital/LVDS_serdata_tx_vcmDAC
        self.elements[12].identity = tk.Spinbox(self.elements[12].master, textvariable = self.elements[12].value, width=4, from_=0, to=63) 
        self.elements[12].row = 11
        self.elements[12].column = 1
        # digital/LVDS_serdata_tx_enable
        self.elements[13].identity = tk.Scale(self.elements[13].master, variable = self.elements[13].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[13].row = 10
        self.elements[13].column = 4
        # digital/generate_8b10b_idle
        self.elements[14].identity = tk.Scale(self.elements[14].master, variable = self.elements[14].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[14].row = 7
        self.elements[14].column = 4
        # digital/TDC_factory_mode
        self.elements[15].identity = tk.Scale(self.elements[15].master, variable = self.elements[15].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[15].row = 16
        self.elements[15].column = 1
        # digital/TDC_offset_middle
        self.elements[16].identity = tk.Spinbox(self.elements[16].master, textvariable = self.elements[16].value, width=4, from_=0, to=7) 
        self.elements[16].row = 17
        self.elements[16].column = 1
        # digital/TDC_offset_fine
        self.elements[17].identity = tk.Spinbox(self.elements[17].master, textvariable = self.elements[17].value, width=4, from_=0, to=31) 
        self.elements[17].row = 17
        self.elements[17].column = 4
        # digital/TDC_offset_enable
        self.elements[18].identity = tk.Scale(self.elements[18].master, variable = self.elements[18].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[18].row = 16
        self.elements[18].column = 4
        # bias/ADC_rvb
        self.elements[19].identity = tk.Spinbox(self.elements[19].master, textvariable = self.elements[19].value, width=4, value=('0', '1', '3')) 
        self.elements[19].row = 15
        self.elements[19].column = 1 
        # digital/T0channel_mask
        self.elements[20].identity = tk.Scale(self.elements[20].master, variable = self.elements[20].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[20].row = 1
        self.elements[20].column = 1
        # coincidence/L1_trigger_pretime
        self.elements[21].identity = tk.Spinbox(self.elements[21].master, textvariable = self.elements[21].value, width=4, from_=0, to=15)  
        self.elements[21].row = 8
        self.elements[21].column = 9
        # coincidence/L1_trigger_pretime_sign
        self.elements[22].identity = tk.Scale(self.elements[22].master, variable = self.elements[22].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)  
        self.elements[22].row = 8
        self.elements[22].column = 7
        # coincidence/L1_trigger_posttime
        self.elements[23].identity = tk.Spinbox(self.elements[21].master, textvariable = self.elements[21].value, width=4, from_=0, to=15) 
        self.elements[23].row = 9
        self.elements[23].column = 9
        # coincidence/L1_trigger_mode
        self.elements[24].identity = tk.Scale(self.elements[24].master, variable = self.elements[24].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[24].row = 7
        self.elements[24].column = 9
        # digital/PLL_resistor
        self.elements[25].identity = tk.Spinbox(self.elements[25].master, textvariable = self.elements[25].value, width=4, from_=0, to=3) 
        self.elements[25].row = 14
        self.elements[25].column = 1
        # digital/PLL_Icp
        self.elements[26].identity = tk.Spinbox(self.elements[26].master, textvariable = self.elements[26].value, width=4, from_=0, to=15) 
        self.elements[26].row = 15
        self.elements[26].column = 1
        # digital/PLL_Ibuf
        self.elements[27].identity = tk.Spinbox(self.elements[27].master, textvariable = self.elements[27].value, width=4, from_=0, to=15) 
        self.elements[27].row = 14
        self.elements[27].column = 4
        # bias/amon_DAC
        self.elements[28].identity = tk.Spinbox(self.elements[28].master, textvariable = self.elements[28].value, width=4, from_=0, to=63) 
        self.elements[28].row = 16
        self.elements[28].column = 4 
        # bias/amon_ena
        self.elements[29].identity = tk.Scale(self.elements[29].master, variable = self.elements[29].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[29].row = 15
        self.elements[29].column = 4 
        # bias/shap_LG_DAC
        self.elements[30].identity = tk.Spinbox(self.elements[30].master, textvariable = self.elements[30].value, width=4, from_=0, to=15) 
        self.elements[30].row = 8
        self.elements[30].column = 1  
        # bias/shap_ena
        self.elements[31].identity = tk.Scale(self.elements[31].master, variable = self.elements[31].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[31].row = 6
        self.elements[31].column = 1          
        # bias/shap_HG_DAC
        self.elements[32].identity = tk.Spinbox(self.elements[32].master, textvariable = self.elements[32].value, width=4, from_=0, to=15) 
        self.elements[32].row = 7
        self.elements[32].column = 1 
        # bias/delay_ena
        self.elements[33].identity = tk.Scale(self.elements[33].master, variable = self.elements[33].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[33].row = 11
        self.elements[33].column = 4  
        # bias/lfota_HG_DAC
        self.elements[34].identity = tk.Spinbox(self.elements[34].master, textvariable = self.elements[34].value, width=4, from_=0, to=7) 
        self.elements[34].row = 6
        self.elements[34].column = 4         
        # bias/lfota_LG_DAC
        self.elements[35].identity = tk.Spinbox(self.elements[35].master, textvariable = self.elements[35].value, width=4, from_=0, to=7) 
        self.elements[35].row = 7
        self.elements[35].column = 4  
        # bias/subthbias_Rtrim
        self.elements[36].identity = tk.Spinbox(self.elements[36].master, textvariable = self.elements[36].value, width=4, value=('0', '1', '3')) 
        self.elements[36].row = 16
        self.elements[36].column = 1 
        # bias/isgtriode_trim_ena_n
        self.elements[37].identity = tk.Scale(self.elements[37].master, variable = self.elements[37].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[37].row = 1
        self.elements[37].column = 4        
        # bias/isgtriode_trim_DAC
        self.elements[38].identity = tk.Spinbox(self.elements[38].master, textvariable = self.elements[38].value, width=4, from_=0, to=255) 
        self.elements[38].row = 2
        self.elements[38].column = 4 
        # bias/isg_sdcomp_DAC
        self.elements[39].identity = tk.Spinbox(self.elements[39].master, textvariable = self.elements[39].value, width=4, from_=0, to=7) 
        self.elements[39].row = 3
        self.elements[39].column = 1         
        # bias/isg_sdbias_DAC
        self.elements[40].identity = tk.Spinbox(self.elements[40].master, textvariable = self.elements[40].value, width=4, from_=0, to=7) 
        self.elements[40].row = 2
        self.elements[40].column = 1 
        # bias/comparator_th_gs
        self.elements[41].identity = tk.Spinbox(self.elements[41].master, textvariable = self.elements[41].value, width=4, from_=0, to=63) 
        self.elements[41].row = 12
        self.elements[41].column = 1 
        # __filler
        self.elements[42].identity = 0 
        # bias/isg_biastune
        self.elements[43].identity = tk.Spinbox(self.elements[43].master, textvariable = self.elements[43].value, width=4, value=('0', '1', '3')) 
        self.elements[43].row = 1
        self.elements[43].column = 1
        # __filler
        self.elements[44].identity = 0 
        # bias/comparator_th_trig
        self.elements[45].identity = tk.Spinbox(self.elements[45].master, textvariable = self.elements[45].value, width=4, from_=0, to=63) 
        self.elements[45].row = 11
        self.elements[45].column = 1 
        # bias/delay_DAC
        self.elements[46].identity = tk.Spinbox(self.elements[46].master, textvariable = self.elements[46].value, width=4, from_=0, to=15) 
        self.elements[46].row = 12
        self.elements[46].column = 4   
        # coincidence/coincidence_window
        self.elements[47].identity = tk.Spinbox(self.elements[47].master, textvariable = self.elements[47].value, width=4, from_=0, to=7)  
        self.elements[47].row = 7
        self.elements[47].column = 3
        # coincidence/channel0-35/coincidence_mask
        for i in range(35):
            self.elements[48+i].identity = [
                tk.Checkbutton(self.elements[48+i].master, text = "E",    variable = self.elements[48+i].value[ 9], onvalue = 1, offvalue = 0), 
                tk.Checkbutton(self.elements[48+i].master, text = "35",   variable = self.elements[48+i].value[ 8], onvalue = 1, offvalue = 0), 
                tk.Checkbutton(self.elements[48+i].master, text = "34",   variable = self.elements[48+i].value[ 7], onvalue = 1, offvalue = 0), 
                tk.Checkbutton(self.elements[48+i].master, text = "33",   variable = self.elements[48+i].value[ 6], onvalue = 1, offvalue = 0), 
                tk.Checkbutton(self.elements[48+i].master, text = "32",   variable = self.elements[48+i].value[ 5], onvalue = 1, offvalue = 0), 
                tk.Checkbutton(self.elements[48+i].master, text = "31",   variable = self.elements[48+i].value[ 4], onvalue = 1, offvalue = 0), 
                tk.Checkbutton(self.elements[48+i].master, text = "30",   variable = self.elements[48+i].value[ 3], onvalue = 1, offvalue = 0), 
                tk.Checkbutton(self.elements[48+i].master, text = "29",   variable = self.elements[48+i].value[ 2], onvalue = 1, offvalue = 0), 
                tk.Checkbutton(self.elements[48+i].master, text = "28",   variable = self.elements[48+i].value[ 1], onvalue = 1, offvalue = 0), 
                tk.Checkbutton(self.elements[48+i].master, text = "27",   variable = self.elements[48+i].value[ 0], onvalue = 1, offvalue = 0)
            ] 
            self.elements[48+i].row = i
            self.elements[48+i].column = 1
        # digital/trigger_or_enable
        self.elements[84].identity = tk.Scale(self.elements[84].master, variable = self.elements[84].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[84].row = 1
        self.elements[84].column = 4
        # digital/powerdown_pipe
        self.elements[85].identity = tk.Scale(self.elements[85].master, variable = self.elements[85].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[85].row = 3
        self.elements[85].column = 4
        # digital/powerdown_sar
        self.elements[86].identity = tk.Scale(self.elements[86].master, variable = self.elements[86].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[86].row = 2
        self.elements[86].column = 4
        # digital/pipemode_enable
        self.elements[87].identity = tk.Scale(self.elements[87].master, variable = self.elements[87].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[87].row = 2
        self.elements[87].column = 1
        # Channel: 22 elements
        # channel pattern definition, includes fe/adc/channel-digital. Total 53 bits
        # For the FE, bit 18,19 was added in KLaus5
        # channel/mask
        self.elements[88].identity  = tk.Scale(self.elements[88].master, variable = self.elements[88].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[88].row = 1
        self.elements[88].column = 1
        # channel/ADC_PIPEamp_comp_ena
        self.elements[89].identity  = tk.Scale(self.elements[89].master, variable = self.elements[89].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[89].row = 12
        self.elements[89].column = 1  
        # channel/ADC_PIPEamp_comp_dac
        self.elements[90].identity  = tk.Spinbox(self.elements[90].master, textvariable = self.elements[90].value, width=4, from_=0, to=15)  
        self.elements[90].row = 13
        self.elements[90].column = 1   
        # __filler
        self.elements[91].identity  = 0 
        # __filler
        self.elements[92].identity  = 0 
        # channel/ADC_captrim
        self.elements[93].identity  = tk.Spinbox(self.elements[93].master, textvariable = self.elements[93].value, width=4, from_=0, to=15)  
        self.elements[93].row = 14
        self.elements[93].column = 1   
        # channel/trigger_th_finetune
        self.elements[94].identity  = tk.Spinbox(self.elements[94].master, textvariable = self.elements[94].value, width=4, from_=0, to=15)  
        self.elements[94].row = 8
        self.elements[94].column = 6  
        # channel/gainsel_busy_enable
        self.elements[95].identity  = tk.Scale(self.elements[95].master, variable = self.elements[95].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[95].row = 13
        self.elements[95].column = 6   
        # channel/hold_delay_finetune
        self.elements[96].identity  = tk.Spinbox(self.elements[96].master, textvariable = self.elements[96].value, width=4, from_=0, to=15)  
        self.elements[96].row = 9
        self.elements[96].column = 6   
        # channel/branch_sel_config
        self.elements[97].identity  = tk.Spinbox(self.elements[97].master, textvariable = self.elements[97].value, width=14, from_=0, to=3)  
        self.elements[97].row = 1
        self.elements[97].column = 6   
        # channel/ext_trigger_select
        self.elements[98].identity = tk.Scale(self.elements[98].master, variable = self.elements[98].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[98].row = 2
        self.elements[98].column = 1 
        # __filler
        self.elements[99].identity = 0 
        # channel/HG_scale_select
        self.elements[100].identity = tk.Scale(self.elements[100].master, variable = self.elements[100].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[100].row = 3
        self.elements[100].column = 6  
        # channel/ADC_in_n_select
        self.elements[101].identity = tk.Scale(self.elements[101].master, variable = self.elements[101].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[101].row = 2
        self.elements[101].column = 6  
        # channel/lfota_disable
        self.elements[102].identity = tk.Scale(self.elements[102].master, variable = self.elements[102].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[102].row = 14
        self.elements[102].column = 6   
        # channel/monitor_HG_disable
        self.elements[103].identity = tk.Scale(self.elements[103].master, variable = self.elements[103].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[103].row = 3
        self.elements[103].column = 1  
        # channel/monitor_LG_disable
        self.elements[104].identity = tk.Scale(self.elements[104].master, variable = self.elements[104].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[104].row = 4
        self.elements[104].column = 1  
        # channel/trigger_LSB_scale
        self.elements[105].identity = tk.Scale(self.elements[105].master, variable = self.elements[105].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[105].row = 7
        self.elements[105].column = 6  
        # channel/LG_scale_select
        self.elements[106].identity = tk.Scale(self.elements[106].master, variable = self.elements[106].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[106].row = 4
        self.elements[106].column = 6  
        # __filler
        self.elements[107].identity = 0 
        # channel/sw_triodesel
        self.elements[108].identity = tk.Scale(self.elements[108].master, variable = self.elements[108].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[108].row = 12
        self.elements[108].column = 6   
        # channel/vDAC_SiPM
        self.elements[109].identity = tk.Spinbox(self.elements[109].master, textvariable = self.elements[109].value, width=4, from_=0, to=255)  
        self.elements[109].row = 7
        self.elements[109].column = 1 

        for index in range(len(self.elements)):
            if(self.elements[index].identity):
                tk.Label(self.elements[index].master, text = self.elements[index].text, anchor='e').grid(row=self.elements[index].row, column=self.elements[index].column-1, sticky='s'+'e')
                if(index<=87):
                    if type(self.elements[index].identity) == tk.Spinbox:
                        self.elements[index].identity.grid(row=self.elements[index].row, column=self.elements[index].column, sticky=self.elements[index].sticky)
                    elif type(self.elements[index].identity) == tk.Scale:
                        self.elements[index].identity.grid(row=self.elements[index].row, column=self.elements[index].column, sticky=self.elements[index].sticky)
                    else: # tkinter.Checkbutton array
                        for jndex in range(len(self.elements[index].identity)):
                            self.elements[index].identity[jndex].grid(row=self.elements[index].row, column=self.elements[index].column+jndex, sticky=self.elements[index].sticky)
                else:
                    if type(self.elements[index].identity) == tk.Spinbox:
                        self.elements[index].identity.grid(row=self.elements[index].row, column=self.elements[index].column, sticky=self.elements[index].sticky)
                    elif type(self.elements[index].identity) == tk.Scale:
                        self.elements[index].identity.grid(row=self.elements[index].row, column=self.elements[index].column, sticky=self.elements[index].sticky)
                    else: # tkinter.Checkbutton array
                        for jndex in range(len(self.elements[index].identity)):
                            self.elements[index].identity[jndex].grid(row=self.elements[index].row, column=self.elements[index].column+jndex, sticky=self.elements[index].sticky)

        self.load_config_file("config_default.txt")

        #####################################################################
        notebook.grid(row=0, column=0, columnspan=4)

        #####################################################################
        self.button_load = tk.Button(self.app, text='Load Config', width=15, command=self.load_config_file)
        # self.button.pack(padx=10, pady=5)
        self.button_load.grid(row=1, column=0, padx=1, pady=5)  

        self.button_save = tk.Button(self.app, text='Save Config', width=15, command=self.save_config_file)
        # self.button.pack(padx=10, pady=5)
        self.button_save.grid(row=1, column=1, padx=1, pady=5)  

        self.button_config = tk.Button(self.app, text='Connect ASIC', width=15, command=self.update_asic) 
        # self.button.pack(padx=10, pady=5)
        self.button_config.grid(row=1, column=2, padx=1, pady=5)  

        self.button_data = tk.Button(self.app, text='Read ASIC', width=15)
        # self.button.pack(padx=10, pady=5)
        self.button_data.grid(row=1, column=3, padx=1, pady=5)  
        
    def update_channel(self, event):
        # Channel: 22 elements
        pre_channel_num = eval(self.pre_channel.get())
        for index in range(88, 110):
            if type(self.elements[index].value.get()) == str :
                value = eval(self.elements[index].value.get())
            else:
                value = self.elements[index].value.get()
            self.configuration.SetInPatternWR(index + 22 * pre_channel_num, value)
        cur_channel_num = eval(self.cur_channel.get())
        for index in range(88, 110):
            value = self.configuration.GetFromPatternWR(index + 22 * cur_channel_num)
            if type(self.elements[index].value.get()) == str :
                self.elements[index].value.set(str(value))
            else:
                self.elements[index].value.set(value)

        self.pre_channel.set(str(self.cur_channel.get()))

    def load_config_file(self, filename = ''):
        if(filename!="config_default.txt"):
            filename = tk.filedialog.askopenfilename(filetypes=[('TXT', '*.txt'), ('All Files', '*')])
        self.configuration.ReadFromFile(filename)
        # display update
        for index in range(len(self.elements)):
            if(self.elements[index].identity):
                if(index<=87):
                    if type(self.elements[index].identity) == tk.Spinbox:
                        self.elements[index].value.set(str(self.configuration.GetFromPatternWR(index)))
                    elif type(self.elements[index].identity) == tk.Scale:
                        self.elements[index].value.set(self.configuration.GetFromPatternWR(index))
                    else: # tkinter.Checkbutton array
                        temp = self.configuration.GetFromPatternWR(index)
                        for jndex in range(len(self.elements[index].identity)):
                            self.elements[index].value[jndex].set((temp>>jndex)&0x1)
                else:
                    cur_channel_num = eval(self.cur_channel.get())
                    if type(self.elements[index].identity) == tk.Spinbox:
                        self.elements[index].value.set(str(self.configuration.GetFromPatternWR(index + 22 * cur_channel_num)))
                    elif type(self.elements[index].identity) == tk.Scale:
                        self.elements[index].value.set(self.configuration.GetFromPatternWR(index + 22 * cur_channel_num))
                    else: # tkinter.Checkbutton array
                        temp = self.configuration.GetFromPatternWR(index + 22 * cur_channel_num)
                        for jndex in range(len(self.elements[index].identity)):
                            self.elements[index].value[jndex].set((temp>>jndex)&0x1)

    def save_config_file(self, filename = ''):
        filename = tk.filedialog.asksaveasfilename(filetypes=[('TXT', '*.txt'), ('All Files', '*')], 
            defaultextension = '.txt', initialfile='config')
        if not filename.endswith('.txt'):
            filename += '.txt'
        self.configuration.SaveToFile(filename)

    def update_asic(self):
        if(self.ifac.status == 0):
            self.ifac.init(linkNo = 0, deviceNo = 0)
            self.button_config['text'] = 'Config ASIC'
        bits = self.configuration.Writebitcode()
        error = self.ifac.configCheck(bits)
        if(error !=0):
            print('Found '+str(error)+' errors during config')
        else:
            print("Config done")



