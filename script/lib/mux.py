#!/usr/bin/python           
# This is mux.py file
# TCA9548APWR
# author: zhj@ihep.ac.cn
# 2020-12-17 created

import i2c

MUX_CR_I2CA   = 0b00000001 # I2CA for KLauS
MUX_CR_I2CB   = 0b00000010 # I2CB for KLauS
MUX_CR_SPI    = 0b00000100 # I2CB for SPI
MUX_CR_GPIO   = 0b00001000 # I2CB for GPIO
MUX_CR_EEPROM = 0b00010000 # I2CB for EEPROM
MUX_CR_DOWN   = 0b00100000 # I2CB for downstream

class mux(object):
    def __init__(self, device_address = 0x70 << 1, base_address = 0x200, clk_freq = 120, i2c_freq = 100):
        self._i2c = i2c.i2c(device_address, base_address, clk_freq, i2c_freq)

    def enable_all(self):
        self._i2c.write8(0b11111111)

    def disable_all(self):
        self._i2c.write8(0)

    def enable_i2ca(self):
        temp = self._i2c.read8()
        temp |= MUX_CR_I2CA
        self._i2c.write8(temp)

    def disable_i2ca(self):
        temp = self._i2c.read8()
        temp &= ~MUX_CR_I2CA
        self._i2c.write8(temp)

    def enable_i2cb(self):
        temp = self._i2c.read8()
        temp |= MUX_CR_I2CB
        self._i2c.write8(temp)

    def disable_i2cb(self):
        temp = self._i2c.read8()
        temp &= ~MUX_CR_I2CB
        self._i2c.write8(temp)

    def enable_spi(self):
        temp = self._i2c.read8()
        temp |= MUX_CR_SPI
        self._i2c.write8(temp)

    def disable_spi(self):
        temp = self._i2c.read8()
        temp &= ~MUX_CR_SPI
        self._i2c.write8(temp)

    def enable_gpio(self):
        temp = self._i2c.read8()
        temp |= MUX_CR_GPIO
        self._i2c.write8(temp)

    def disable_gpio(self):
        temp = self._i2c.read8()
        temp &= ~MUX_CR_GPIO
        self._i2c.write8(temp)

    def enable_eeprom(self):
        temp = self._i2c.read8()
        temp |= MUX_CR_EEPROM
        self._i2c.write8(temp)

    def disable_eeprom(self):
        temp = self._i2c.read8()
        temp &= ~MUX_CR_EEPROM
        self._i2c.write8(temp)

    def enable_down(self):
        temp = self._i2c.read8()
        temp |= MUX_CR_DOWN
        self._i2c.write8(temp)

    def disable_down(self):
        temp = self._i2c.read8()
        temp &= ~MUX_CR_DOWN
        self._i2c.write8(temp)

#################################################################

