#!/usr/bin/python           
# This is klaus6.py file
# KLauS6
# author: zhj@ihep.ac.cn
# 2020-12-30 created
import sys
from time import sleep
import lib
from lib import rbcp
from lib import i2c
from lib import spi
from lib import gpio

# Event data
# The single hit information sent through the data interfaces consists of all hit information (channel number, 
# ADC and TDC information, flags). Given the additional TDC fine time information, the event data is 6 bytes in KLauS6
EVENT_LEN       = 6
EMPTY_HEADER    = 0x3F
NONE_HEADER     = 0x0

def printf(format, *args):
    sys.stdout.write(format % args)

class klaus6(object):
    def __init__(self, device_address = 0x40 << 1, base_address = 0x200, clk_freq = 120, i2c_freq = 100):
        self._i2c = i2c.i2c(device_address, base_address, clk_freq, i2c_freq)

    def readEvent(self):
        return self._i2c.readBytes(EVENT_LEN)

    def readEvents(self):
        # To inform the I2C master implemented in the DAQ hardware that there is no more
        # event to read, the ASIC will transmit an "empty event" indicated by an unused
        # channel number. In this case, after reading this empty event, the DAQ may stop
        # reading further events. In KLauS6, the first byte empty event is given by 0x3F.    
        temp = self.readEvent()
        events = bytes()
        while ((temp[0] != EMPTY_HEADER)&(temp[0] != NONE_HEADER)):
            printf ("0x%02x%02x_%02x%02x%02x%02x\r\n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5])
            events += temp
            temp = self.readEvent()
        return events

    def read8(self, with_internal_addr = False, internal_addr = 0):
        return self._i2c.read8(with_internal_addr, internal_addr)
