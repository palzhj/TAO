#!/usr/bin/python           
# This is the event data model for the 6 bytes bit stream readout from Klaus6 chip, translated from the c++ daq-common/EventType class
# Usage: event = EDM.EDM(byets_event)
#		 event.printHeader()
#		 event.print()
#		 where "bytes_event" is the 6 bytes output from klaus.readEvent()
# author: liyichen@ihep.ac.cn
# 2021-1-06 created
import sys

def printf(format, *args):
    sys.stdout.write(format % args)

def mapMC(MC):
	MCmap = [6,1,2,5,4,3,0,7]
	return MCmap[MC]

class EDM(object):
	def __init__(self, bytes_event):
		self.foffset = 0
		self.moffset = 1

		self.groupID         =	( bytes_event[0] >> 6 ) & 0x0003     # evt[0]: (7:6)
		self.channelID       =	( bytes_event[0] >> 2 ) & 0x000f     # evt[0]: (5:2)
    	    
    	# This is verfied, for T0 channel: groupID=3, channelID=0
		self.channel = 0xff
		if self.groupID == 3 and self.channelID==15:
			self.channel = 36 # T0 channel
		else:
			if self.channelID < 9:
				self.channel = 9*self.groupID + self.channelID

		self.gainsel_evt	 =	( bytes_event[0] >> 1 ) & 0x0001     # evt[0]: (1:1)
		self.ADC_10b         =	((bytes_event[0] << 9 ) & 0x0200) | ((bytes_event[1] << 1) & 0x01fe) | ((bytes_event[2] >> 7) & 0x0001)
		self.ADC_6b          =	((bytes_event[0] << 5 ) & 0x0020) | ((bytes_event[1] >> 3) & 0x001f)
		self.ADC_PIPE        =  ((bytes_event[1] << 5 ) & 0x0007) | ((bytes_event[2] >> 3) & 0x001f)
		self.T_CC            =	((bytes_event[2] <<16 ) &0x70000) | ((bytes_event[3] << 8 )& 0x0ff00) | ((bytes_event[4] << 0)& 0x000ff)
		self.T_MC			 =  ((bytes_event[5] >> 5 ) & 0x0007)
		self.T_FC			 =  ((bytes_event[5] << 0 ) & 0x001f)

		self.ADC_10b         = 	1023    -   self.ADC_10b
		self.ADC_6b          =	63      -   self.ADC_6b
		self.ADC_PIPE        = 	255     -   self.ADC_PIPE
		self.T_FC		 	 = 	31	-	self.T_FC # correct
		self.T_MC			 = 	mapMC(self.T_MC) # correct

	def printHeader(self):
		printf("CHANNEL\tGS\tADC_10b\tADC_6b\tPIPE\tT_CC\tT_MC\tT_FC\tTime\n")

	def print(self):
		printf("%2.2u\t%1.1u\t%3.3u\t%2.2u\t%3.3u\t%d\t%d\t%d\t%d\n",
		self.channel,
		self.gainsel_evt,
		self.ADC_10b,
		self.ADC_6b,
		self.ADC_PIPE,
		self.T_CC,
        self.T_MC,
		self.T_FC,
		self.getTime())

	def getTime(self):
		FC = (self.T_FC + self.foffset)	& 0x1f # offset correction
		MC = (self.T_MC + self.moffset) & 0x07 # offset correction
		mctemp = MC>>1 & 0x03 # align MC according to FC
		cctemp = (self.T_CC%(1<<19)) >> 1 # align CC according to aligned-MC
		if MC%2==1 and FC/8==0:
			mctemp = int((MC/2) + 1)     & 0x03
		if MC%2==0 and FC/8==3:
			mctemp = int((MC/2) - 1 + 4) & 0x03

		if self.T_CC%2==1 and mctemp==0:
			cctemp = (cctemp + 1) & 0x1ffff
		if self.T_CC%2==0 and mctemp==3:
			cctemp = (cctemp + (1<<18) - 1) & 0x1ffff

		return (cctemp<<7)+(mctemp<<5)+FC
