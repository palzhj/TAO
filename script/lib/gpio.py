#!/usr/bin/python           
# This is gpio.py file
# TCA9554
# author: zhj@ihep.ac.cn
# 2020-12-28 created

import i2c

GPIO_IPR_ADDR = 0 # Input Port register, R
GPIO_OPR_ADDR = 1 # Output Port register, R/W
GPIO_PIR_ADDR = 2 # Polarity Inversion register, R/W
GPIO_CR_ADDR  = 3 # Configuration register, R/W

PIN_RST       = 0b00000001  # out
PIN_SRST      = 0b00000010  # out
PIN_ACQ_EN    = 0b00000100  # out

PIN_FIFO_FULL = 0b00010000  # in
PIN_OR        = 0b00100000  # in

PIN_CAL_SEL   = 0b10000000  # out

class gpio(object):
    def __init__(self, device_address = 0x38 << 1, base_address = 0x200, clk_freq = 120, i2c_freq = 100):
        self._i2c = i2c.i2c(device_address, base_address, clk_freq, i2c_freq)
        self._i2c.write8(0, True, GPIO_OPR_ADDR)
        self._i2c.write8(0, True, GPIO_PIR_ADDR)
        self._i2c.write8(PIN_FIFO_FULL | PIN_OR, True, GPIO_CR_ADDR) # If a bit in this register is set to 1, the corresponding port pin is enabled as an input with a high-impedance output driver.
  
    def read(self):
        return self._i2c.read8(True, GPIO_IPR_ADDR)

    def write(self, value):
        return self._i2c.write8(value, True, GPIO_OPR_ADDR)

    def rst_read(self):
        if (PIN_RST & self.read()):
            return 1
        else:
            return 0

    def rst_set(self): # set to high
        temp = self._i2c.read8(True, GPIO_OPR_ADDR)
        temp |= PIN_RST
        self.write(temp)

    def rst_reset(self): # reset to low
        temp = self._i2c.read8(True, GPIO_OPR_ADDR)
        temp &= ~PIN_RST
        self.write(temp)

    def srst_read(self):
        if (PIN_SRST & self.read()):
            return 1
        else:
            return 0

    def srst_set(self):
        temp = self._i2c.read8(True, GPIO_OPR_ADDR)
        temp |= PIN_SRST
        self.write(temp)

    def srst_reset(self):
        temp = self._i2c.read8(True, GPIO_OPR_ADDR)
        temp &= ~PIN_SRST
        self.write(temp)

    def acq_en_read(self):
        if (PIN_ACQ_EN & self.read()):
            return 1
        else:
            return 0

    def acq_en_set(self):
        temp = self._i2c.read8(True, GPIO_OPR_ADDR)
        temp |= PIN_ACQ_EN
        self.write(temp)

    def acq_en_reset(self):
        temp = self._i2c.read8(True, GPIO_OPR_ADDR)
        temp &= ~PIN_ACQ_EN
        self.write(temp)

    def fifo_full_read(self):
        if (PIN_FIFO_FULL & self.read()):
            return 1
        else:
            return 0

    def or_read(self):
        if (PIN_OR & self.read()):
            return 1
        else:
            return 0

    def cal_sel_read(self):
        if (PIN_CAL_SEL & self.read()):
            return 1
        else:
            return 0

    def cal_sel_set(self):
        temp = self._i2c.read8(True, GPIO_OPR_ADDR)
        temp |= PIN_CAL_SEL
        self.write(temp)

    def cal_sel_reset(self):
        temp = self._i2c.read8(True, GPIO_OPR_ADDR)
        temp &= ~PIN_CAL_SEL
        self.write(temp)
