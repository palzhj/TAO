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
LOCAL_TEST      = False

def printf(format, *args):
    sys.stdout.write(format % args)

class klaus6(object):
    def __init__(self, device_address = 0x40 << 1, base_address = 0x200, clk_freq = 160, i2c_freq = 100):
        self.nevt_read = 0
        self.ntimes_read = 0
        self.quiet = True
        if LOCAL_TEST:
            self.file = open("/afs/ihep.ac.cn/users/l/liyichen52/klaus/script/Klaus6_bitflow_test.txt", "rb")
        else:
            self._i2c = i2c.i2c(device_address, base_address, clk_freq, i2c_freq)

    def beQuiet(self):
        self.quiet = True

    def readEvent(self):
        if LOCAL_TEST:
            temp = self.file.read(EVENT_LEN)
            if temp != bytearray():
                return temp
            else:
                return [0x3F]
        else:
            return self._i2c.readBytes(EVENT_LEN)

    def readEvents(self, limit = 1000):
        # To inform the I2C master implemented in the DAQ hardware that there is no more
        # event to read, the ASIC will transmit an "empty event" indicated by an unused
        # channel number. In this case, after reading this empty event, the DAQ may stop
        # reading further events. In KLauS6, the first byte empty event is given by 0x3F.    
        temp = self.readEvent()
        events = bytes()
        cnt = 0
        nsleep = 0
        while (True):
            # previous stop condition: if an empty header appears, stop reading
            # new stop condition: if an empty header appears, sleep for 0.1s and retry, if still empty, then stop
            if temp[0] == EMPTY_HEADER: 
                if nsleep == 0:
                    sleep(0.1)
                    temp = self.readEvent()
                    nsleep = 1
                    continue
                else:
                    break

            if not self.quiet:
                printf ("0x%02x%02x_%02x%02x%02x%02x\r\n",temp[0],temp[1],temp[2],temp[3],temp[4],temp[5])
            events += temp
            cnt += 1

            if cnt >= limit:
                break

            temp = self.readEvent()

        self.nevt_read = cnt
        self.ntimes_read += 1

        if not self.quiet:
            print(self.nevt_read, "events read in the", self.ntimes_read, "call of klaus6.readEvents")

        return events

    def nevtRead(self):
        return self.nevt_read

    def read8(self, with_internal_addr = False, internal_addr = 0):
        return self._i2c.read8(with_internal_addr, internal_addr)
