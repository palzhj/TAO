#!/usr/bin/python           
# This is server.py file
# author: zhj@ihep.ac.cn
# 2019-06-18 created

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
from lib import klaus6config
from lib import gui
from lib import interface
from lib import sitcp
from lib import EDM
from lib import afg3252c
from ROOT import TFile
from ROOT import TTree
from ROOT import TBranch
from array import array

# command line argument parser
def get_parser():
    import argparse
    parser = argparse.ArgumentParser(description='Run KLauS6 DAQ.')
    parser.add_argument("--nseq", type=int, default=10, help='sequences to be processed')
    parser.add_argument("--test", default=False, help="test mode using Klaus6_bitflow_test.txt, instead of from output of Klaus6")
    parser.add_argument("--quiet", default=False, help="be quiet")
    parser.add_argument("--output", default="test-output.root", help="specify output filename")
    return parser

parser = get_parser()
args = parser.parse_args()
print(args)

def printf(format, *outs):
    sys.stdout.write(format % outs)

output = TFile(args.output, "recreate")
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

########################################
# main
# afg=afg3252c.afg3252c()

# afg.setOutput(on=0)
# afg.setFrequency(10)
# afg.setPluse(10000)
# afg.setVoltage(100)
# afg.setEdgeLeadin(2.5)
# afg.setEdgeTrailing(2.5)
# afg.setOutput(on=1)

# afg.close()

sitcp = sitcp.sitcp()
while (0==sitcp.getLinkStatus()):
    sitcp.resetLink()
    sleep(1)
    print("Relink")

sitcp.open()
sleep(5)
for n in range(args.nseq):
    sleep(1)
    data = sitcp.readEvents()
    length = len(data)
    i = 0
    while i<length:
          #print ("0x%02x%02x_%02x%02x_%02x%02x" % (data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5]))
          if(data[i]!=0x3F):
            bytes_i_event = data[i:(i+6)]
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
    
          i += 6
          if(i+6>=length):
            break
    if(n>args.nseq):
      break

output.Write()
output.Close()

sitcp.close()
