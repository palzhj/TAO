#!/usr/bin/python           
# This is ucd90160.py file
# author: zhj@ihep.ac.cn
# 2019-06-28 created
import i2c
import math

class ucd90xxx(object):
    def __init__(self, device_address = 0x67 << 1, base_address = 0x200, clk_freq = 120, i2c_freq = 100):
        self._i2c = i2c.i2c(device_address, base_address, clk_freq, i2c_freq)

    def read_voltage(self, channel = 1):
        if (channel < 1) | (channel > 16):
            print("Invalid channel number")
            return 0
        page = channel -1
        self._i2c.write8(page, True, 0) # select page
        #print(self._i2c.read8(True, 0x0))
        a = self._i2c.read8(True, 0x20) # Vout_mode
        #print(a)
        if a > 15:
            a = a - 32
        #print(a)
        b = self._i2c.read16(True, 0x8B) # Vout
        #print(b)
        c = b * math.pow( 2, a )  # Vout 
        #print(c)
        return c

    def read_current(self, channel = 1):
        if (channel < 1) | (channel > 16):
            print("Invalid channel number")
            return 0
        page = channel -1
        self._i2c.write8(page, True, 0) # select page
        a = self._i2c.read16(True, 0x8C) # Iout
        # print(a)
        b = a & 0x07FF
        # print(b)
        a = a >> 11
        if a > 15:
            a = a - 32
        # print(a)
        c = b * math.pow( 2, a )  # Vout 
        # print(c)
        return c

    def read_device_id(self):
        return self._i2c.readBytes(32, True, 0xfd)
