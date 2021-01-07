import sys
from lib import EDM
from ROOT import TFile
from ROOT import TTree
from ROOT import TBranch
from array import array

from time import sleep
import sys
import lib
from lib import rbcp
from lib import i2c
from lib import mux
from lib import sysmon
from lib import ucd90xxx
from lib import gpio
from lib import spi
from lib import klaus6

LPC_PORTA_BASE_ADDR = 0x200
LPC_PORTB_BASE_ADDR = 0x300
LPC_PMBUS_BASE_ADDR = 0x400
HPC_PORTA_BASE_ADDR = 0x500
HPC_PORTB_BASE_ADDR = 0x600
HPC_PMBUS_BASE_ADDR = 0x700

def printf(format, *args):
    sys.stdout.write(format % args)

device_addr_offset = 0
#################################################################
# KLauS6
KLAUS_ADDR = (0x40+device_addr_offset) << 1
klaus6 = klaus6.klaus6(KLAUS_ADDR, LPC_PORTA_BASE_ADDR)

# 1. read a test binary file and get the 1st event
file = open("Klaus6_bitflow_test.txt", "rb")
#binary_file = file.read()
binary_file = klaus6.readEvents()
file.close()

bytes_first_event = binary_file[0:60]

# 2. feed the first event to EDM class
first_event = EDM.EDM(bytes_first_event)
first_event.printHeader()
first_event.print() # the output is the same as that from the c++ daq-i2c class 

# 3. try to read 10 events
nevent = 10

output = TFile("output.root", "recreate")
tree = TTree("dump", "dumptree")

channel = array('i',[0])
groupID = array('i',[0])
channelID = array('i',[0])
gainsel_evt = array('i',[0])
ADC_10b = array('i',[0])
ADC_6b = array('i',[0])
ADC_PIPE = array('i',[0])
T_CC = array('i',[0])
T_MC = array('i',[0])
T_FC = array('i',[0])
tree.Branch("channel",channel,"channel/I")
tree.Branch("groupID",groupID,"groupID/I")
tree.Branch("channelID",channelID,"channelID/I")
tree.Branch("gainsel_evt",gainsel_evt,"gainsel_evt/I")
tree.Branch("ADC_10b",ADC_10b,"ADC_10b/I")
tree.Branch("ADC_6b",ADC_6b,"ADC_6b/I")
tree.Branch("ADC_PIPE",ADC_PIPE,"ADC_PIPE/I")
tree.Branch("T_CC",T_CC,"T_CC/I")
tree.Branch("T_MC",T_MC,"T_MC/I")
tree.Branch("T_FC",T_FC,"T_FC/I")

for i in range(nevent):
	bytes_i_event = binary_file[60*i:60*(i+1)]
	i_event = EDM.EDM(bytes_i_event)
	i_event.print()

	channel    [0]= i_event.channel   
	groupID    [0]= i_event.groupID
	channelID  [0]= i_event.channelID
	gainsel_evt[0]= i_event.gainsel_evt
	ADC_10b    [0]= i_event.ADC_10b
	ADC_6b     [0]= i_event.ADC_6b
	ADC_PIPE   [0]= i_event.ADC_PIPE
	T_CC       [0]= i_event.T_CC
	T_MC       [0]= i_event.T_MC
	T_FC       [0]= i_event.T_FC
	tree.Fill()
	
tree.Scan("channel:groupID:channelID:gainsel_evt:ADC_10b:ADC_6b:ADC_PIPE:T_CC:T_MC:T_FC")
output.Write()
output.Close()

# To integrate with klaus6.py:
# 1. Replace the source of the binary file from test file to the output of klaus6.py: 
#	 binary_file = klaus6.readEvents()
# 2. Repeat step 2 and 3
