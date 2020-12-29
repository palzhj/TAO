#!/usr/bin/python           
# This is spi.py file
# SC18IS602B
# author: zhj@ihep.ac.cn
# 2020-12-28 created
from time import sleep
import i2c

SPI_SS0_ADDR    = 0x01 # 
SPI_SS1_ADDR    = 0x02 # 
SPI_SS2_ADDR    = 0x04 # 
SPI_SS3_ADDR    = 0x08 #
MAX_BUFFER_LEN  = 200

SPI_CR_ADDR     = 0xF0 # Configure SPI Interface
CR_LSB          = 0b00100000 #If logic 1, the LSB of the data word is transmitted first.
CR_CPOL         = 0b00001000
CR_CPHA         = 0b00000100
# 00 - SPICLK LOW when idle; data clocked in on leading edge (CPOL = 0, CPHA = 0)
# 01 - SPICLK LOW when idle; data clocked in on trailing edge (CPOL = 0, CPHA = 1)
# 10 - SPICLK HIGH when idle; data clocked in on trailing edge (CPOL = 1, CPHA = 0)
# 11 - SPICLK HIGH when idle; data clocked in on leading edge (CPOL = 1, CPHA = 1)
CR_SPEED_58     = 0b00000011 # 58 kHz
CR_SPEED_115    = 0b00000010 # 115 kHz
CR_SPEED_461    = 0b00000001 # 461 kHz
CR_SPEED_1843   = 0b00000000 # 1843 kHz

SPI_CIR_ADDR     = 0xF1 # Clear Interrupt
SPI_IDR_ADDR     = 0xF2 # Idle mode
SPI_GPIOW_ADDR   = 0xF4 # GPIO Write
SPI_GPIOR_ADDR   = 0xF5 # GPIO Read
SPI_GPIOE_ADDR   = 0xF6 # GPIO Enable

SPI_GPIOC_ADDR   = 0xF7 # GPIO Configuration
GPIOC_BIDIR      = 0b00
GPIOC_PUSH_PULL  = 0b01
GPIOC_INPUT      = 0b10
GPIOC_OD         = 0b11


class spi(object):
    def __init__(self, device_address = 0x28 << 1, base_address = 0x200, clk_freq = 120, i2c_freq = 100):
        self._i2c = i2c.i2c(device_address, base_address, clk_freq, i2c_freq)
        self._i2c.write8(CR_SPEED_1843, True, SPI_CR_ADDR)
        # CAN NOT select all SS as GPIO!
        # self._i2c.write8((GPIOC_PUSH_PULL<<6)|(GPIOC_PUSH_PULL<<4)|(GPIOC_PUSH_PULL<<2)|GPIOC_PUSH_PULL, True, SPI_GPIOC_ADDR)
        # self._i2c.write8(SPI_SS0_ADDR|SPI_SS1_ADDR|SPI_SS2_ADDR|SPI_SS3_ADDR, True, SPI_GPIOW_ADDR) # CS high
        # self._i2c.write8(SPI_SS0_ADDR|SPI_SS1_ADDR|SPI_SS2_ADDR|SPI_SS3_ADDR, True, SPI_GPIOE_ADDR)
        self._i2c.write8(GPIOC_PUSH_PULL, True, SPI_GPIOC_ADDR)
        self._i2c.write8(SPI_SS0_ADDR, True, SPI_GPIOW_ADDR) # CS high
        self._i2c.write8(SPI_SS0_ADDR, True, SPI_GPIOE_ADDR)

    def clear_interrupt(self):
        self._i2c.write8(SPI_CIR_ADDR)

    def idle_mode(self):
        self._i2c.write8(SPI_CIR_ADDR)

    def cs_high(self, internal_addr = SPI_SS0_ADDR):
        if (internal_addr == SPI_SS0_ADDR):
            temp = self._i2c.read8(True, SPI_GPIOW_ADDR)
            temp |= SPI_SS0_ADDR
            self._i2c.write8(temp, True, SPI_GPIOW_ADDR)
        elif (internal_addr == SPI_SS1_ADDR):
            temp = self._i2c.read8(True, SPI_GPIOW_ADDR)
            temp |= SPI_SS1_ADDR
            self._i2c.write8(temp, True, SPI_GPIOW_ADDR)
        elif (internal_addr == SPI_SS2_ADDR):
            temp = self._i2c.read8(True, SPI_GPIOW_ADDR)
            temp |= SPI_SS2_ADDR
            self._i2c.write8(temp, True, SPI_GPIOW_ADDR)
        else:
            temp = self._i2c.read8(True, SPI_GPIOW_ADDR)
            temp |= SPI_SS3_ADDR
            self._i2c.write8(temp, True, SPI_GPIOW_ADDR)

    def cs_low(self, internal_addr = SPI_SS0_ADDR):
        if (internal_addr == SPI_SS0_ADDR):
            temp = self._i2c.read8(True, SPI_GPIOW_ADDR)
            temp &= ~SPI_SS0_ADDR
            self._i2c.write8(temp, True, SPI_GPIOW_ADDR)
        elif (internal_addr == SPI_SS1_ADDR):
            temp = self._i2c.read8(True, SPI_GPIOW_ADDR)
            temp &= ~SPI_SS1_ADDR
            self._i2c.write8(temp, True, SPI_GPIOW_ADDR)
        elif (internal_addr == SPI_SS2_ADDR):
            temp = self._i2c.read8(True, SPI_GPIOW_ADDR)
            temp &= ~SPI_SS2_ADDR
            self._i2c.write8(temp, True, SPI_GPIOW_ADDR)
        else:
            temp = self._i2c.read8(True, SPI_GPIOW_ADDR)
            temp &= ~SPI_SS3_ADDR
            self._i2c.write8(temp, True, SPI_GPIOW_ADDR)

    def readBytes(self, length = 1):
        if(length>MAX_BUFFER_LEN):
            raise ValueError("The data buffer is 200 bytes deep!")
        else:
            return self._i2c.readBytes(length)

    def writeBytes(self, data, internal_addr = SPI_SS0_ADDR):
        self.cs_low(internal_addr)
        length = len(data)
        send_len = 0
        while length >= MAX_BUFFER_LEN:
            self._i2c.writeBytes(data[send_len:send_len+MAX_BUFFER_LEN], True, 0x0F)
            # print(data[send_len:send_len+MAX_BUFFER_LEN])
            sleep(0.1)
            send_len += MAX_BUFFER_LEN
            length -= MAX_BUFFER_LEN
        if(length):
            self._i2c.writeBytes(data[send_len:], True, 0x0F)
            # print(data[send_len:])
        sleep(0.1)
        self.cs_high(internal_addr)
