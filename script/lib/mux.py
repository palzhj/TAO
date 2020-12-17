#!/usr/bin/python           
# This is mux.py file
# TCA9548APWR
# author: zhj@ihep.ac.cn
# 2020-12-17 created

import i2c

TCA9548_ADDR = 0x70 << 1

MUX_CR_I2CA   = 0b00000001 # I2CA for KLauS
MUX_CR_I2CB   = 0b00000010 # I2CB for KLauS
MUX_CR_SPI    = 0b00000100 # I2CB for SPI
MUX_CR_GPIO   = 0b00001000 # I2CB for GPIO
MUX_CR_EEPROM = 0b00010000 # I2CB for EEPROM
MUX_CR_DOWN   = 0b00100000 # I2CB for downstream

class mux(object):
    def __init__(self, address = TCA9548_ADDR):
        self._i2c = i2c.i2c(address)

    def enable_i2ca(self):
        temp = self._i2c.read8()
        printf("I2C MUX: 0x%02x\r\n", temp)
        temp |= MUX_CR_I2CA
        self._i2c.write8(temp)
        temp = self._i2c.read8()
        printf("I2C MUX: 0x%02x\r\n", temp)

    def disable_i2ca(self):
        temp = self._i2c.read8()
        printf("I2C MUX: 0x%02x\r\n", temp)
        temp &= ~MUX_CR_I2CA
        self._i2c.write8(temp)
        temp = self._i2c.read8()
        printf("I2C MUX: 0x%02x\r\n", temp)

#################################################################

