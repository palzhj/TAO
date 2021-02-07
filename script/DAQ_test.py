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

def printf(format, *args):
    sys.stdout.write(format % args)

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
data = sitcp.readEvents()
length = len(data)
i = 0
while i<length:
    print ("0x%02x%02x_%02x%02x_%02x%02x" % (data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5]))
    # if(data[i]!=0x3F):
    #     event = EDM.EDM([data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5]])
    #     event.printHeader()
    #     event.print()
    i += 6
    if(i+6>=length):
        break

sitcp.close()
