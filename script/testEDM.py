# Read output from Klaus and call EDM

import sys

from lib import klaus6
from lib import EDM
from ROOT import TFile
from ROOT import TTree
from ROOT import TBranch
from array import array

# addresses
LPC_PORTA_BASE_ADDR = 0x200
LPC_PORTB_BASE_ADDR = 0x300
LPC_PMBUS_BASE_ADDR = 0x400
HPC_PORTA_BASE_ADDR = 0x500
HPC_PORTB_BASE_ADDR = 0x600
HPC_PMBUS_BASE_ADDR = 0x700

# command line argument parser
def get_parser():
    import argparse
    parser = argparse.ArgumentParser(description='Run Tao Detector Simulation.')
    parser.add_argument("--evtmax", type=int, default=10, help='events to be processed')
    parser.add_argument("--test", default=False, help="test mode using Klaus6_bitflow_test.txt, instead of from output of Klaus6")
    parser.add_argument("--quiet", default=False, help="be quiet")
    return parser

parser = get_parser()
args = parser.parse_args()
print(args)

# In test mode, read a local binary file
if args.test: 
	klaus6.LOCAL_TEST = True

device_addr_offset = 0
KLAUS_ADDR = (0x40+device_addr_offset) << 1
klaus6 = klaus6.klaus6(KLAUS_ADDR, LPC_PORTA_BASE_ADDR)
if args.quiet:
	klaus6.beQuiet()
binary_file = klaus6.readEvents(args.evtmax)
nbytes = len(binary_file)

# 3. try to read n events
nevent = args.evtmax

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

# "*/i": i means unsigned int with 32 bit
tree.Branch("channel",channel,"channel/i")
tree.Branch("groupID",groupID,"groupID/i")
tree.Branch("channelID",channelID,"channelID/i")
tree.Branch("gainsel_evt",gainsel_evt,"gainsel_evt/i")
tree.Branch("ADC_10b",ADC_10b,"ADC_10b/i")
tree.Branch("ADC_6b",ADC_6b,"ADC_6b/i")
tree.Branch("ADC_PIPE",ADC_PIPE,"ADC_PIPE/i")
tree.Branch("T_CC",T_CC,"T_CC/i")
tree.Branch("T_MC",T_MC,"T_MC/i")
tree.Branch("T_FC",T_FC,"T_FC/i")

for i in range(nevent):
	if 6*(i+1) > nbytes-1:
		print("WARN: evtmax =",nevent,"but only",i+1,"events available")
		break

	bytes_i_event = binary_file[6*i:6*(i+1)]
	i_event = EDM.EDM(bytes_i_event)
	if not args.quiet:
		if i == 0:
			i_event.printHeader()
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
	
#tree.Scan("channel:groupID:channelID:gainsel_evt:ADC_10b:ADC_6b:ADC_PIPE:T_CC:T_MC:T_FC")
output.Write()
output.Close()
