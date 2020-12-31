#!/usr/bin/python           
# This is klaus6.py file
# KLauS6
# author: zhj@ihep.ac.cn
# 2020-12-30 created
from time import sleep
import rbcp
import i2c
import spi
import gpio

# Event data
# The single hit information sent through the data interfaces consists of all hit information (channel number, 
# ADC and TDC information, flags). Given the additional TDC fine time information, the event data is 6 bytes in KLauS6
EVENT_LEN       = 6
EMPTY_HEADER    = 0x3F
NONE_HEADER     = 0x0

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
        while ((temp[0] != EMPTY_HEADER)&(temp[0] != NONE_HEADER)):
            temp += self.readEvent()
        print (temp)
        return temp

    def read8(self, with_internal_addr = False, internal_addr = 0):
        return self._i2c.read8(with_internal_addr, internal_addr)
