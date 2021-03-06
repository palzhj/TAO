#!/usr/bin/python
# -*- coding: UTF-8 -*-
# author: zhj@ihep.ac.cn

import tkinter as tk
import tkinter.ttk as ttk
import tkinter.filedialog
import tkinter.messagebox

import lib
from lib import klaus6config
from lib import interface
from lib import EDM

class ToolTip(object):
    def __init__(self,widget):
        self.widget = widget
        self.tip_window = None
 
    def show_tip(self,tip_text):
        "Display text in a tooltip window"
        if self.tip_window or not tip_text:
            return
        x, y, _cx, cy = self.widget.bbox("insert")      
        # get size of widget
        x = x + self.widget.winfo_rootx() + 25          
        # calculate to display tooptip
        y = y + cy + self.widget.winfo_rooty() + 25     
        # below and to the right
        self.tip_window = tw = tk.Toplevel(self.widget) 
        # create new tooltip window
        tw.wm_overrideredirect(True)                    
        # remove all Window Manager (wm)
        tw.wm_geometry("+%d+%d" %(x, y))                
        # create window size
 
        label = tk.Label(tw, text=tip_text, justify=tk.LEFT,
                         background="#ffffe0", relief=tk.SOLID,
                         borderwidth=1,font=("tahoma", "8", "normal"))
        label.pack(ipadx=1)
 
    def hide_tip(self):
        tw = self.tip_window
        self.tip_window = None
        if tw:
            tw.destroy()
 
def create_ToolTip(widget, text):
    tooltip = ToolTip(widget)
    def enter(event):
        tooltip.show_tip(text)
    def leave(event):
        tooltip.hide_tip()
    widget.bind('<Enter>', enter)
    widget.bind('<Leave>', leave)

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
        self.offline_mode = 0

        self.app = tk.Tk()
        self.app.title('KLaus6 Config GUI')
        notebook = ttk.Notebook(self.app)

        #####################################################################
        tabCD = tk.Frame()
        notebook.add(tabCD, text='Common Digital')
        tabCD_setting = tk.LabelFrame(tabCD, font="bold", text='DEVICE ADDRESS')
        self.link_setting = tk.IntVar(tabCD_setting)
        self.link_setting.set(0)
        link_list = ["FMC LPC0", "FMC LPC1", "FMC HPC0", "FMC HPC1"]
        self.link_setting_radiobutton = []
        for i in range(4):
            self.link_setting_radiobutton.append(tk.Radiobutton(tabCD_setting, variable=self.link_setting, text=link_list[i], value=i))
            self.link_setting_radiobutton[i].grid(row = 0, column = i)
        tabCD_setting.grid_columnconfigure(4, minsize=100)
        tk.Label(tabCD_setting, text='Device offset(0~3):').grid(row=0, column = 5)
        self.device_setting = tk.StringVar(tabCD_setting)
        self.device_setting_spinbox = tk.Spinbox(tabCD_setting, textvariable = self.device_setting, width=4, from_=0, to=7)
        self.device_setting_spinbox.grid(row = 0, column = 6)
        create_ToolTip(self.device_setting_spinbox, "Set by the switch on the board")

        tabCD_setting.pack(fill='both', expand=True)
        tabCD_flags = tk.LabelFrame(tabCD, font="bold", text='Flags')
        tabCD_flags.grid_columnconfigure(2, minsize=100)
        tabCD_flags.pack(fill='both', expand=True)
        tabCD_readout = tk.LabelFrame(tabCD, font="bold", text='Readout links configuration')
        tabCD_readout.grid_columnconfigure(2, minsize=100)
        tabCD_readout.pack(fill='both', expand=True)
        tabCD_lvds = tk.LabelFrame(tabCD, font="bold", text='LVDS configuration')
        tabCD_lvds.grid_columnconfigure(2, minsize=100)
        tabCD_lvds.pack(fill='both', expand=True)
        tabCD_pll = tk.LabelFrame(tabCD, font="bold", text='PLL & TDC configuration')
        tabCD_pll.grid_columnconfigure(2, minsize=100)
        tabCD_pll.pack(fill='both', expand=True)
       
        #####################################################################
        tabPC = tk.Frame()
        notebook.add(tabPC, text='Power Gating/Coincidence')
        tabPC_pgc = tk.LabelFrame(tabPC, font="bold", text='Power Gating Control')
        tabPC_pgc.pack(fill='both', expand=True)
        tabPC_pgc0 = tk.Frame(tabPC_pgc)
        tabPC_pgc0.pack(fill='both', expand=True)
        tabPC_pgc1 = tk.Frame(tabPC_pgc)
        tabPC_pgc1.pack(fill='both', expand=True)
        tabPC_coin = tk.LabelFrame(tabPC, font="bold", text='Coincidence')
        tabPC_coin.pack(fill='both', expand=True)
        tabPC_coin_l0 = tk.LabelFrame(tabPC_coin, text='L0 FIFO validation', labelanchor = 'n')
        tabPC_coin_l0.pack(fill='both', expand=True, side = 'left', padx = 5)
        tabPC_coin_l1 = tk.LabelFrame(tabPC_coin, text='L1 FIFO validation', labelanchor = 'n')
        tabPC_coin_l1.pack(fill='both', expand=True, side = 'right', padx = 5)
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
        tabGB_left = tk.Frame(tabGB)
        tabGB_left.pack(fill='both', expand=True, side='left', padx = 5)
        tabGB_right = tk.Frame(tabGB)
        tabGB_right.pack(fill='both', expand=True, side='right', padx = 5)
        tabGB_input0 = tk.LabelFrame(tabGB_left, font="bold", text='Input Stage Bias', width =600)
        tabGB_input0.pack(fill='both', expand=True)
        tabGB_input1 = tk.LabelFrame(tabGB_right, font="bold", text='Input Stage Triode Tuning(Advanced)')
        tabGB_input1.pack(fill='both', expand=True)
        tabGB_shap = tk.LabelFrame(tabGB_left, font="bold", text='Shapers')
        tabGB_shap.pack(fill='both', expand=True)
        tabGB_feed = tk.LabelFrame(tabGB_right, font="bold", text='Low-frequency feedback')
        tabGB_feed.pack(fill='both', expand=True)
        tabGB_comp = tk.LabelFrame(tabGB_left, font="bold", text='Comparators')
        tabGB_comp.pack(fill='both', expand=True)
        tabGB_delay = tk.LabelFrame(tabGB_right, font="bold", text='Hold Delay')
        tabGB_delay.pack(fill='both', expand=True)
        tabGB_resistor = tk.LabelFrame(tabGB_left, font="bold", text='Bias Resistor Tuning')
        tabGB_resistor.pack(fill='both', expand=True)
        tabGB_monitor = tk.LabelFrame(tabGB_right, font="bold", text='Analog Monitor Bias')
        tabGB_monitor.pack(fill='both', expand=True)

 
        #####################################################################
        tabCH = tk.Frame()
        notebook.add(tabCH, text='Channels')

        tk.Label(tabCH, font="bold", text='Channel Selection:', anchor="e").grid(row=0, column=0)
        self.cur_channel = tk.StringVar()
        self.cur_channel.set('0')
        combobox_ch = ttk.Combobox(tabCH, state='readonly', textvariable=self.cur_channel, width = 5,
            values=['0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '10', '11', '12', '13', '14', '15', '16', '17', \
                    '18', '19', '20', '21', '22', '23', '24', '25', '26', '27', '28', '29', '30', '31', '32', '33', '34', '35'])
        combobox_ch.bind('<<ComboboxSelected>>', self.update_channel)
        combobox_ch.grid(row=0, column = 1, sticky='e')
        self.pre_channel = tk.StringVar(tabCH)
        self.pre_channel.set('0')

        tabCH.grid_columnconfigure(0, minsize=100)
        tabCH.grid_columnconfigure(4, minsize=100)
        tabCH.grid_columnconfigure(9, minsize=80)
        CHcolumnspan = 10
        tk.Label(tabCH, font="bold", text='Channel Flags').grid(row=1, column=0, columnspan=CHcolumnspan//2)
        tk.Label(tabCH, font="bold", text='Output & Dynamic range selection').grid(row=1, column=CHcolumnspan//2, columnspan=CHcolumnspan//2)
        # tk.Label(tabCH, anchor="w", text='0) Auto HG/LG').grid(row=2, column=CHcolumnspan-1)
        # tk.Label(tabCH, anchor="w", text='1) External').grid(row=3, column=CHcolumnspan-1)
        # tk.Label(tabCH, anchor="w", text='2) LG force').grid(row=4, column=CHcolumnspan-1)
        # tk.Label(tabCH, anchor="w", text='3) HG force').grid(row=5, column=CHcolumnspan-1)
        ttk.Separator(tabCH, orient='horizontal').grid(row=6,  sticky='e'+'w', columnspan=CHcolumnspan)
        tk.Label(tabCH, font="bold", text='DACs').grid(row=7, column=0, columnspan=CHcolumnspan)
        ttk.Separator(tabCH, orient='horizontal').grid(row=11,  sticky='e'+'w', columnspan=CHcolumnspan)
        tk.Label(tabCH, font="bold", text='Pipelined ADC configuration').grid(row=12, column=0, columnspan=CHcolumnspan//2)
        tk.Label(tabCH, font="bold", text='Advanced/Debug').grid(row=12, column=CHcolumnspan//2, columnspan=CHcolumnspan//2)
        ttk.Separator(tabCH, orient='horizontal').grid(row=16,  sticky='e'+'w', columnspan=CHcolumnspan)
        self.button_channel_copy = tk.Button(tabCH, text='Copy to channel:', width=15, command=self.channel_copy)
        self.button_channel_copy.grid(row=17, column=0)  
        self.copy_channel = tk.StringVar()
        self.copy_channel.set('1')
        combobox_toch = ttk.Combobox(tabCH, state='readonly', textvariable=self.copy_channel, width = 5,
            values=['0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '10', '11', '12', '13', '14', '15', '16', '17', \
                    '18', '19', '20', '21', '22', '23', '24', '25', '26', '27', '28', '29', '30', '31', '32', '33', '34', '35'])
        combobox_toch.grid(row=17, column = 1, sticky='e')
        self.button_channel_all_copy = tk.Button(tabCH, text='Copy to all channels', width=20, command=self.channel_all_copy)
        self.button_channel_all_copy.grid(row=17, column=CHcolumnspan//2)  

        #####################################################################        
        # tabData = tk.Frame()
        # notebook.add(tabData, text='Read Data')

        #####################################################################
        self.elements = []
        for config in self.configuration.parameters:
            name = config.name.replace("_", " ").split("/", 1)
            if len(name)>1:
                self.elements.append(_element(text = name[1][0].upper()+name[1][1:]))
            else:
                self.elements.append(_element(text = '__filler'))

        self.elements[0].master  = tabCD_lvds # digital/clk_div_sel
        self.elements[1].master  = tabPC_pgc1 # coincidence/powergate_time_1
        self.elements[2].master  = tabPC_pgc1 # coincidence/powergate_time_2
        self.elements[3].master  = tabPC_pgc1 # coincidence/powergate_time_3
        self.elements[4].master  = tabPC_pgc1 # coincidence/powergate_time_4
        self.elements[5].master  = tabPC_pgc1 # coincidence/powergate_time_5
        self.elements[6].master  = tabPC_pgc0 # coincidence/powergate_do_gate
        self.elements[7].master  = tabPC_pgc0 # coincidence/powergate_enable
        self.elements[8].master  = tabCD_readout # digital/i2c_address
        self.elements[9].master  = tabCD_flags # digital/debug_pad_enable
        self.elements[10].master = tabCD_readout # digital/i2c_readout_enable
        self.elements[11].master = tabCD_lvds # digital/LVDS_serdata_tx_iDAC
        self.elements[12].master = tabCD_lvds # digital/LVDS_serdata_tx_vcmDAC
        self.elements[13].master = tabCD_lvds # digital/LVDS_serdata_tx_enable
        self.elements[14].master = tabCD_readout # digital/generate_8b10b_idle
        self.elements[15].master = tabCD_pll # digital/TDC_factory_mode
        self.elements[16].master = tabCD_pll # digital/TDC_offset_middle
        self.elements[17].master = tabCD_pll # digital/TDC_offset_fine
        self.elements[18].master = tabCD_pll # digital/TDC_offset_enable
        self.elements[19].master = tabGB_resistor # bias/ADC_rvb
        self.elements[20].master = tabCD_flags # digital/T0channel_mask
        self.elements[21].master = tabPC_coin_l1 # coincidence/L1_trigger_pretime
        self.elements[22].master = tabPC_coin_l1 # coincidence/L1_trigger_pretime_sign
        self.elements[23].master = tabPC_coin_l1 # coincidence/L1_trigger_posttime
        self.elements[24].master = tabPC_coin_l1 # coincidence/L1_trigger_mode
        self.elements[25].master = tabCD_pll # digital/PLL_resistor
        self.elements[26].master = tabCD_pll # digital/PLL_Icp
        self.elements[27].master = tabCD_pll # digital/PLL_Ibuf
        self.elements[28].master = tabGB_monitor # bias/amon_DAC
        self.elements[29].master = tabGB_monitor # bias/amon_ena
        self.elements[30].master = tabGB_shap # bias/shap_LG_DAC
        self.elements[31].master = tabGB_shap # bias/shap_ena
        self.elements[32].master = tabGB_shap # bias/shap_HG_DAC
        self.elements[33].master = tabGB_delay # bias/delay_ena
        self.elements[34].master = tabGB_feed # bias/lfota_HG_DAC
        self.elements[35].master = tabGB_feed # bias/lfota_LG_DAC
        self.elements[36].master = tabGB_resistor # bias/subthbias_Rtrim
        self.elements[37].master = tabGB_input1 # bias/isgtriode_trim_ena_n
        self.elements[38].master = tabGB_input1 # bias/isgtriode_trim_DAC
        self.elements[39].master = tabGB_input0 # bias/isg_sdcomp_DAC
        self.elements[40].master = tabGB_input0 # bias/isg_sdbias_DAC
        self.elements[41].master = tabGB_comp # bias/comparator_th_gs
        self.elements[42].master = tabCD # __filler
        self.elements[43].master = tabGB_input0 # bias/isg_biastune
        self.elements[44].master = tabCD # __filler
        self.elements[45].master = tabGB_comp # bias/comparator_th_trig
        self.elements[46].master = tabGB_delay # bias/delay_DAC
        self.elements[47].master = tabPC_coin_l0 # coincidence/coincidence_window
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
        self.elements[84].master = tabCD_flags # digital/trigger_or_enable
        self.elements[85].master = tabCD_flags # digital/powerdown_pipe
        self.elements[86].master = tabCD_flags # digital/powerdown_sar
        self.elements[87].master = tabCD_flags # digital/pipemode_enable
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

        self.button_channel_all = []
        # location and identity
        # digital/clk_div_sel
        self.elements[0].identity  = tk.Scale(self.elements[0].master, variable = self.elements[0].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[0].row = 0
        self.elements[0].column = 1
        # coincidence/powergate_time_1
        self.elements[1].identity  = tk.Spinbox(self.elements[1].master, textvariable = self.elements[1].value, width=4, from_=0, to=7) 
        self.elements[1].row = 0
        self.elements[1].column = 1
        # coincidence/powergate_time_2
        self.elements[2].identity  = tk.Spinbox(self.elements[2].master, textvariable = self.elements[2].value, width=4, from_=0, to=1023)  
        self.elements[2].row = 0
        self.elements[2].column = 3
        # coincidence/powergate_time_3
        self.elements[3].identity  = tk.Spinbox(self.elements[3].master, textvariable = self.elements[3].value, width=4, from_=0, to=1023)  
        self.elements[3].row = 0
        self.elements[3].column = 5
        # coincidence/powergate_time_4
        self.elements[4].identity  = tk.Spinbox(self.elements[4].master, textvariable = self.elements[4].value, width=4, from_=0, to=1023) 
        self.elements[4].row = 1
        self.elements[4].column = 1
        # coincidence/powergate_time_5
        self.elements[5].identity  = tk.Spinbox(self.elements[5].master, textvariable = self.elements[5].value, width=4, from_=0, to=1023)  
        self.elements[5].row = 1
        self.elements[5].column = 3
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
        self.elements[6].row = 1
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
        self.elements[7].row = 0
        self.elements[7].column = 1
        # digital/i2c_address
        self.elements[8].identity  = tk.Spinbox(self.elements[8].master, textvariable = self.elements[8].value, width=4, from_=0, to=127) 
        self.elements[8].row = 1
        self.elements[8].column = 1
        # digital/debug_pad_enable
        self.elements[9].identity  = tk.Scale(self.elements[9].master, variable = self.elements[9].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[9].row = 2
        self.elements[9].column = 1
        # digital/i2c_readout_enable
        self.elements[10].identity = tk.Scale(self.elements[10].master, variable = self.elements[10].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[10].row = 0
        self.elements[10].column = 4
        # digital/LVDS_serdata_tx_iDAC
        self.elements[11].identity = tk.Spinbox(self.elements[11].master, textvariable = self.elements[11].value, width=4, from_=0, to=63) 
        self.elements[11].row = 1
        self.elements[11].column = 4
        # digital/LVDS_serdata_tx_vcmDAC
        self.elements[12].identity = tk.Spinbox(self.elements[12].master, textvariable = self.elements[12].value, width=4, from_=0, to=63) 
        self.elements[12].row = 1
        self.elements[12].column = 1
        # digital/LVDS_serdata_tx_enable
        self.elements[13].identity = tk.Scale(self.elements[13].master, variable = self.elements[13].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[13].row = 0
        self.elements[13].column = 4
        # digital/generate_8b10b_idle
        self.elements[14].identity = tk.Scale(self.elements[14].master, variable = self.elements[14].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[14].row = 0
        self.elements[14].column = 1
        # digital/TDC_factory_mode
        self.elements[15].identity = tk.Scale(self.elements[15].master, variable = self.elements[15].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[15].row = 2
        self.elements[15].column = 1
        # digital/TDC_offset_middle
        self.elements[16].identity = tk.Spinbox(self.elements[16].master, textvariable = self.elements[16].value, width=4, from_=0, to=7) 
        self.elements[16].row = 3
        self.elements[16].column = 1
        # digital/TDC_offset_fine
        self.elements[17].identity = tk.Spinbox(self.elements[17].master, textvariable = self.elements[17].value, width=4, from_=0, to=31) 
        self.elements[17].row = 3
        self.elements[17].column = 4
        # digital/TDC_offset_enable
        self.elements[18].identity = tk.Scale(self.elements[18].master, variable = self.elements[18].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[18].row = 2
        self.elements[18].column = 4
        # bias/ADC_rvb
        self.elements[19].identity = tk.Spinbox(self.elements[19].master, textvariable = self.elements[19].value, width=4, value=('0', '1', '3')) 
        self.elements[19].row = 1
        self.elements[19].column = 1 
        # digital/T0channel_mask
        self.elements[20].identity = tk.Scale(self.elements[20].master, variable = self.elements[20].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[20].row = 0
        self.elements[20].column = 1
        # coincidence/L1_trigger_pretime
        self.elements[21].identity = tk.Spinbox(self.elements[21].master, textvariable = self.elements[21].value, width=4, from_=0, to=15)  
        self.elements[21].row = 2
        self.elements[21].column = 1
        # coincidence/L1_trigger_pretime_sign
        self.elements[22].identity = tk.Scale(self.elements[22].master, variable = self.elements[22].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)  
        self.elements[22].row = 1
        self.elements[22].column = 1
        # coincidence/L1_trigger_posttime
        self.elements[23].identity = tk.Spinbox(self.elements[21].master, textvariable = self.elements[21].value, width=4, from_=0, to=15) 
        self.elements[23].row = 3
        self.elements[23].column = 1
        # coincidence/L1_trigger_mode
        self.elements[24].identity = tk.Scale(self.elements[24].master, variable = self.elements[24].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[24].row = 0
        self.elements[24].column = 1
        # digital/PLL_resistor
        self.elements[25].identity = tk.Spinbox(self.elements[25].master, textvariable = self.elements[25].value, width=4, from_=0, to=3) 
        self.elements[25].row = 0
        self.elements[25].column = 1
        # digital/PLL_Icp
        self.elements[26].identity = tk.Spinbox(self.elements[26].master, textvariable = self.elements[26].value, width=4, from_=0, to=15) 
        self.elements[26].row = 1
        self.elements[26].column = 1
        # digital/PLL_Ibuf
        self.elements[27].identity = tk.Spinbox(self.elements[27].master, textvariable = self.elements[27].value, width=4, from_=0, to=15) 
        self.elements[27].row = 0
        self.elements[27].column = 4
        # bias/amon_DAC
        self.elements[28].identity = tk.Spinbox(self.elements[28].master, textvariable = self.elements[28].value, width=4, from_=0, to=63) 
        self.elements[28].row = 2
        self.elements[28].column = 4 
        # bias/amon_ena
        self.elements[29].identity = tk.Scale(self.elements[29].master, variable = self.elements[29].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[29].row = 1
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
        self.elements[47].row = 0
        self.elements[47].column = 1
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
        self.elements[84].row = 0
        self.elements[84].column = 4
        # digital/powerdown_pipe
        self.elements[85].identity = tk.Scale(self.elements[85].master, variable = self.elements[85].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[85].row = 2
        self.elements[85].column = 4
        # digital/powerdown_sar
        self.elements[86].identity = tk.Scale(self.elements[86].master, variable = self.elements[86].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[86].row = 1
        self.elements[86].column = 4
        # digital/pipemode_enable
        self.elements[87].identity = tk.Scale(self.elements[87].master, variable = self.elements[87].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True) 
        self.elements[87].row = 1
        self.elements[87].column = 1
        # Channel: 22 elements
        # channel pattern definition, includes fe/adc/channel-digital. Total 53 bits
        # For the FE, bit 18,19 was added in KLaus5
        # channel/mask
        self.elements[88].identity  = tk.Scale(self.elements[88].master, variable = self.elements[88].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[88].row = 2
        self.elements[88].column = 1
        self.button_channel_all.append(tk.Button(self.elements[88].master, text='To All', width=6, command=lambda: self.apply_to_all(88)))
        # channel/ADC_PIPEamp_comp_ena
        self.elements[89].identity  = tk.Scale(self.elements[89].master, variable = self.elements[89].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[89].row = 13
        self.elements[89].column = 1 
        self.button_channel_all.append(tk.Button(self.elements[89].master, text='To All', width=6, command=lambda: self.apply_to_all(89))) 
        # channel/ADC_PIPEamp_comp_dac
        self.elements[90].identity  = tk.Spinbox(self.elements[90].master, textvariable = self.elements[90].value, width=4, from_=0, to=15)  
        self.elements[90].row = 14
        self.elements[90].column = 1  
        self.button_channel_all.append(tk.Button(self.elements[90].master, text='To All', width=6, command=lambda: self.apply_to_all(90))) 
        # __filler
        self.elements[91].identity  = 0 
        self.button_channel_all.append(tk.Button(self.elements[91].master, text='To All', width=6, command=lambda: self.apply_to_all(91)))
        # __filler
        self.elements[92].identity  = 0 
        self.button_channel_all.append(tk.Button(self.elements[92].master, text='To All', width=6, command=lambda: self.apply_to_all(92)))
        # channel/ADC_captrim
        self.elements[93].identity  = tk.Spinbox(self.elements[93].master, textvariable = self.elements[93].value, width=4, from_=0, to=15)  
        self.elements[93].row = 15
        self.elements[93].column = 1  
        self.button_channel_all.append(tk.Button(self.elements[93].master, text='To All', width=6, command=lambda: self.apply_to_all(93))) 
        # channel/trigger_th_finetune
        self.elements[94].identity  = tk.Spinbox(self.elements[94].master, textvariable = self.elements[94].value, width=4, from_=0, to=15)  
        self.elements[94].row = 9
        self.elements[94].column = 6  
        self.button_channel_all.append(tk.Button(self.elements[94].master, text='To All', width=6, command=lambda: self.apply_to_all(94)))
        # channel/gainsel_busy_enable
        self.elements[95].identity  = tk.Scale(self.elements[95].master, variable = self.elements[95].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[95].row = 14
        self.elements[95].column = 6   
        self.button_channel_all.append(tk.Button(self.elements[95].master, text='To All', width=6, command=lambda: self.apply_to_all(95)))
        # channel/hold_delay_finetune
        self.elements[96].identity  = tk.Spinbox(self.elements[96].master, textvariable = self.elements[96].value, width=4, from_=0, to=15)  
        self.elements[96].row = 10
        self.elements[96].column = 6  
        self.button_channel_all.append(tk.Button(self.elements[96].master, text='To All', width=6, command=lambda: self.apply_to_all(96))) 
        # channel/branch_sel_config
        self.elements[97].identity  = tk.Spinbox(self.elements[97].master, textvariable = self.elements[97].value, width=4, from_=0, to=3)  
        self.elements[97].row = 2
        self.elements[97].column = 6  
        self.button_channel_all.append(tk.Button(self.elements[97].master, text='To All', width=6, command=lambda: self.apply_to_all(97))) 
        # channel/ext_trigger_select
        self.elements[98].identity = tk.Scale(self.elements[98].master, variable = self.elements[98].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[98].row = 3
        self.elements[98].column = 1
        self.button_channel_all.append(tk.Button(self.elements[98].master, text='To All', width=6, command=lambda: self.apply_to_all(98))) 
        # __filler
        self.elements[99].identity = 0 
        self.button_channel_all.append(tk.Button(self.elements[99].master, text='To All', width=6, command=lambda: self.apply_to_all(99)))
        # channel/HG_scale_select
        self.elements[100].identity = tk.Scale(self.elements[100].master, variable = self.elements[100].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[100].row = 4
        self.elements[100].column = 6 
        self.button_channel_all.append(tk.Button(self.elements[100].master, text='To All', width=6, command=lambda: self.apply_to_all(100))) 
        # channel/ADC_in_n_select
        self.elements[101].identity = tk.Scale(self.elements[101].master, variable = self.elements[101].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[101].row = 3
        self.elements[101].column = 6 
        self.button_channel_all.append(tk.Button(self.elements[101].master, text='To All', width=6, command=lambda: self.apply_to_all(101))) 
        # channel/lfota_disable
        self.elements[102].identity = tk.Scale(self.elements[102].master, variable = self.elements[102].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[102].row = 15
        self.elements[102].column = 6   
        self.button_channel_all.append(tk.Button(self.elements[102].master, text='To All', width=6, command=lambda: self.apply_to_all(102)))
        # channel/monitor_HG_disable
        self.elements[103].identity = tk.Scale(self.elements[103].master, variable = self.elements[103].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[103].row = 4
        self.elements[103].column = 1  
        self.button_channel_all.append(tk.Button(self.elements[103].master, text='To All', width=6, command=lambda: self.apply_to_all(103)))
        # channel/monitor_LG_disable
        self.elements[104].identity = tk.Scale(self.elements[104].master, variable = self.elements[104].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[104].row = 5
        self.elements[104].column = 1 
        self.button_channel_all.append(tk.Button(self.elements[104].master, text='To All', width=6, command=lambda: self.apply_to_all(104))) 
        # channel/trigger_LSB_scale
        self.elements[105].identity = tk.Scale(self.elements[105].master, variable = self.elements[105].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[105].row = 8
        self.elements[105].column = 6  
        self.button_channel_all.append(tk.Button(self.elements[105].master, text='To All', width=6, command=lambda: self.apply_to_all(105)))
        # channel/LG_scale_select
        self.elements[106].identity = tk.Scale(self.elements[106].master, variable = self.elements[106].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[106].row = 5
        self.elements[106].column = 6 
        self.button_channel_all.append(tk.Button(self.elements[106].master, text='To All', width=6, command=lambda: self.apply_to_all(106))) 
        # __filler
        self.elements[107].identity = 0 
        self.button_channel_all.append(tk.Button(self.elements[107].master, text='To All', width=6, command=lambda: self.apply_to_all(107)))
        # channel/sw_triodesel
        self.elements[108].identity = tk.Scale(self.elements[108].master, variable = self.elements[108].value, from_=0,to=1,orient='horizontal',length=60, showvalue=True)
        self.elements[108].row = 13
        self.elements[108].column = 6 
        self.button_channel_all.append(tk.Button(self.elements[108].master, text='To All', width=6, command=lambda: self.apply_to_all(108)))  
        # channel/vDAC_SiPM
        self.elements[109].identity = tk.Spinbox(self.elements[109].master, textvariable = self.elements[109].value, width=4, from_=0, to=255)  
        self.elements[109].row = 8
        self.elements[109].column = 1 
        self.button_channel_all.append(tk.Button(self.elements[109].master, text='To All', width=6, command=lambda: self.apply_to_all(109)))

        self.elements[10].text = "LVDS readout enable"
        for index in range(len(self.elements)):
            if(self.elements[index].identity):
                tk.Label(self.elements[index].master, text = self.elements[index].text, anchor='e', width = 24).grid(row=self.elements[index].row, column=self.elements[index].column-1, sticky='s'+'e')
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

        for index in range(88,110):
            if self.elements[index].identity:
                self.button_channel_all[index-88].grid(row=self.elements[index].row, column=self.elements[index].column+1, sticky=self.elements[index].sticky) 

        self.load_config_file("config_default.txt")

        # Tooltip
        for index in range(len(self.elements)):
            if(self.elements[index].identity):
                if(type(self.elements[index].identity)==list):
                    for j in range(len(self.elements[index].identity)):
                        create_ToolTip(self.elements[index].identity[j], self.configuration.parameters[index].description)
                else:
                    create_ToolTip(self.elements[index].identity, self.configuration.parameters[index].description)

        #####################################################################
        notebook.grid(row=0, column=0, columnspan=5)

        # post-process
        self.elements[8].identity["state"] = "disabled"

        #####################################################################
        self.button_load = tk.Button(self.app, text='Load Config', width=15, command=self.load_config_file)
        self.button_load.grid(row=1, column=0, padx=1, pady=5)  

        self.button_save = tk.Button(self.app, text='Save Config', width=15, command=self.save_config_file)
        self.button_save.grid(row=1, column=1, padx=1, pady=5)  

        self.button_config = tk.Button(self.app, text='Connect ASIC', width=15, command=self.update_asic) 
        self.button_config.grid(row=1, column=2, padx=1, pady=5)  

        self.button_data = tk.Button(self.app, text='Read ASIC', width=15, command=self.read_asic)
        self.button_data.grid(row=1, column=3, padx=1, pady=5) 
        self.button_data["state"] = "disabled"

        self.button_close = tk.Button(self.app, text='Disconnect', width=15, command=self.close_asic)
        self.button_close.grid(row=1, column=4, padx=1, pady=5) 
        self.button_close["state"] = "disabled"

    def channel_copy(self):
        self.app.update()
        copy_channel_num = eval(self.copy_channel.get())
        # Channel: 22 elements
        for index in range(88, 110):
            if type(self.elements[index].value.get()) == str :
                value = eval(self.elements[index].value.get())
            else:
                value = self.elements[index].value.get()
            self.configuration.SetInPattern(index + 22 * copy_channel_num, value)

    def channel_all_copy(self):
        for index in range(88, 110):
            if type(self.elements[index].value.get()) == str :
                value = eval(self.elements[index].value.get())
            else:
                value = self.elements[index].value.get()
            for i in range(36):
                self.configuration.SetInPattern(index + 22 * i, value)

    def apply_to_all(self, index):
        self.app.update()
        if type(self.elements[index].value.get()) == str :
            value = eval(self.elements[index].value.get())
        else:
            value = self.elements[index].value.get()
        for ch in range(36):
            self.configuration.SetInPattern(index + 22 * ch, value)

    def update_channel(self, event):
        # Channel: 22 elements
        pre_channel_num = eval(self.pre_channel.get())
        for index in range(88, 110):
            if type(self.elements[index].value.get()) == str :
                value = eval(self.elements[index].value.get())
            else:
                value = self.elements[index].value.get()
            self.configuration.SetInPattern(index + 22 * pre_channel_num, value)
        cur_channel_num = eval(self.cur_channel.get())
        for index in range(88, 110):
            value = self.configuration.GetFromPattern(index + 22 * cur_channel_num)
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
                        self.elements[index].value.set(str(self.configuration.GetFromPattern(index)))
                    elif type(self.elements[index].identity) == tk.Scale:
                        self.elements[index].value.set(self.configuration.GetFromPattern(index))
                    else: # tkinter.Checkbutton array
                        temp = self.configuration.GetFromPattern(index)
                        for jndex in range(len(self.elements[index].identity)):
                            self.elements[index].value[jndex].set((temp>>jndex)&0x1)
                else:
                    cur_channel_num = eval(self.cur_channel.get())
                    if type(self.elements[index].identity) == tk.Spinbox:
                        self.elements[index].value.set(str(self.configuration.GetFromPattern(index + 22 * cur_channel_num)))
                    elif type(self.elements[index].identity) == tk.Scale:
                        self.elements[index].value.set(self.configuration.GetFromPattern(index + 22 * cur_channel_num))
                    else: # tkinter.Checkbutton array
                        temp = self.configuration.GetFromPattern(index + 22 * cur_channel_num)
                        for jndex in range(len(self.elements[index].identity)):
                            self.elements[index].value[jndex].set((temp>>jndex)&0x1)

    def save_config_file(self, filename = ''):
        self.update_bitcode()
        filename = tk.filedialog.asksaveasfilename(filetypes=[('TXT', '*.txt'), ('All Files', '*')], 
            defaultextension = '.txt', initialfile='config')
        if not filename.endswith('.txt'):
            filename += '.txt'
        self.configuration.SaveToFile(filename)

    def update_bitcode(self):
        for index in range(88):
            if(self.elements[index].identity):
                if type(self.elements[index].identity) == tk.Spinbox:
                    value = eval(self.elements[index].value.get())
                elif type(self.elements[index].identity) == tk.Scale:
                    value = self.elements[index].value.get()
                else:
                    value = 0
                    for jndex in range(len(self.elements[index].identity)):
                        value |= self.elements[index].value[jndex].get() << jndex
                self.configuration.SetInPattern(index, value)
        # channels
        cur_channel_num = eval(self.cur_channel.get())
        for index in range(88, 110):
            if(self.elements[index].identity):
                if type(self.elements[index].value.get()) == str :
                    value = eval(self.elements[index].value.get())
                else:
                    value = self.elements[index].value.get()
                self.configuration.SetInPattern(index + 22 * cur_channel_num, value)

    def update_asic(self):
        self.app.update()
        linkNo = self.link_setting.get()
        deviceNo = eval(self.device_setting.get())
        if(self.ifac.status == 0):
            try:
                self.ifac.init(linkNo = linkNo, deviceNo = deviceNo)
                print ("linkNo: "+str(linkNo)+"; deviceNo: "+str(deviceNo))
                self.device_setting_spinbox["state"] = "disabled"
                for radiobutton in self.link_setting_radiobutton:
                    radiobutton["state"] = "disabled"
            except Exception:
                tk.messagebox.showinfo("Error", "Device is not connected")
                return
            self.button_config['text'] = 'Re-config ASIC'
            self.button_data["state"] = "normal"
            self.button_close["state"] = "normal"
            self.ifac.status = 1
        # else:
        #     # reset digital
        #     self.ifac.reset()
        self.update_bitcode()
        bits = self.configuration.Writebitcode()  
        try:      
            error = self.ifac.configCheck(bits)
        except Exception:
            tk.messagebox.showinfo("Error", "Device is not connected")
            self.close_asic()      
            return            
        if(error!=0):
            print('Found '+str(error)+' errors during config')
        else:
            print("Config done")
        # reset digital
        # self.ifac.reset()

    def read_asic(self):
        try:
            temp = self.ifac.klaus6.readEvent()
        except Exception:
            tk.messagebox.showinfo("Error", "Device is not connected")
            self.close_asic()
            return
        print ("0x%02x%02x_%02x%02x_%02x%02x" % (temp[0],temp[1],temp[2],temp[3],temp[4],temp[5]))
        if(temp[0]!=0x3F):
            event = EDM.EDM(temp)
            event.printHeader()
            event.print()

    def close_asic(self):
        self.button_config['text'] = 'Connect ASIC'
        self.button_data["state"] = "disabled"
        self.button_close["state"] = "disabled"
        self.ifac.status = 0
        self.device_setting_spinbox["state"] = "normal"
        for radiobutton in self.link_setting_radiobutton:
            radiobutton["state"] = "normal"