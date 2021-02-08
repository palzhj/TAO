#!/usr/bin/python           
# This is klaus6.py file
# KLauS6
# author: zhj@ihep.ac.cn
# 2020-12-30 created
from time import sleep
import lib
from lib import rbcp
from lib import i2c
from lib import mux
from lib import sysmon
from lib import ucd90xxx
from lib import gpio
from lib import spi
from lib import klaus6

import sys
def printf(format, *args):
    sys.stdout.write(format % args)

# base
LPC_PORTA_BASE_ADDR = 0x200 # link0
LPC_PORTB_BASE_ADDR = 0x300 # link0
LPC_PMBUS_BASE_ADDR = 0x400 # link0
HPC_PORTA_BASE_ADDR = 0x500 # link1
HPC_PORTB_BASE_ADDR = 0x600 # link1
HPC_PMBUS_BASE_ADDR = 0x700 # link1

# device address
UCD_ADDR    = 0x67
MUX_ADDR    = 0x70
EEPROM_ADDR = 0x50
GPIO_ADDR   = 0x38
SPI_ADDR    = 0x28
KLAUS_ADDR  = 0x40

class interface:
    def __init__(self):
        self.status = 0 # uninitialized
        self.linkAddr = 0
        self.deviceOffset = 0
        self.reg = 0
        self.sysmon = 0
        self.ucd = 0
        self.mux = 0
        self.gpio = 0
        self.spi = 0
        self.klaus6 = 0

    def init(self, linkNo = 0, deviceNo = 0):
        if linkNo == 3:
            self.linkAddr = HPC_PORTB_BASE_ADDR
        elif linkNo == 2:
            self.linkAddr = HPC_PORTA_BASE_ADDR
        elif linkNo == 1:
            self.linkAddr = LPC_PORTB_BASE_ADDR
        else:
            self.linkAddr = LPC_PORTA_BASE_ADDR

        self.deviceOffset = deviceNo & 0x03

        self.reg = rbcp.Rbcp()

        self.sysmon = sysmon.sysmon()
        if(linkNo <2):
            pmbus_base = LPC_PMBUS_BASE_ADDR
        else:
            pmbus_base = HPC_PMBUS_BASE_ADDR
        self.ucd = ucd90xxx.ucd90xxx(UCD_ADDR<<1, pmbus_base)
        self.power_on()
        sleep(0.5)

        mux_addr = (MUX_ADDR+self.deviceOffset) << 1
        self.mux = mux.mux(mux_addr, self.linkAddr)

        self.mux.enable_eeprom()
        self.mux.enable_gpio()
        self.mux.enable_spi()
        self.mux.enable_i2ca()
        self.mux.enable_down()

        gpio_addr = (GPIO_ADDR+self.deviceOffset) << 1
        self.gpio = gpio.gpio(gpio_addr, self.linkAddr)

        spi_addr = (SPI_ADDR+self.deviceOffset) << 1
        self.spi = spi.spi(spi_addr, self.linkAddr)

        klaus_addr = (KLAUS_ADDR+self.deviceOffset) << 1
        self.klaus6 = klaus6.klaus6(klaus_addr, self.linkAddr)
        
        self.reset()  
        self.reset_digital()    
        self.status = 1 # initialized

    def power_on(self):
        self.reg.write(0x18, bytes([0xFF]))
        self.reg.write(0x19, bytes([0xFF]))
        self.reg.write(0x1A, bytes([0xFF]))
        self.reg.write(0x1B, bytes([0xFF]))
        self.reg.write(0x1C, bytes([0xFF]))
        self.reg.write(0x1D, bytes([0xFF]))
        self.reg.write(0x1E, bytes([0xFF]))
        self.reg.write(0x1F, bytes([0xFF]))

    def power_off(self):
        self.reg.write(0x18, bytes([0x0]))
        self.reg.write(0x19, bytes([0x0]))
        self.reg.write(0x1A, bytes([0x0]))
        self.reg.write(0x1B, bytes([0x0]))
        self.reg.write(0x1C, bytes([0x0]))
        self.reg.write(0x1D, bytes([0x0]))
        self.reg.write(0x1E, bytes([0x0]))
        self.reg.write(0x1F, bytes([0x0]))

    def reset(self):
        self.gpio.cal_sel_reset() # select INT
        self.gpio.acq_en_reset()
        self.gpio.rst_set()
        sleep(0.2)
        self.gpio.rst_reset()
        sleep(0.2)
    
    def reset_digital(self):
        self.gpio.srst_set()
        sleep(0.2)
        self.gpio.srst_reset()
        sleep(0.2)            

    def print(self):
        # firmware info
        self.read_firmware_info()
        # fpga status
        print("FPGA temperature: ", round(self.sysmon.temperature(),2), "C")
        print("FPGA vccint: ", round(self.sysmon.vccint(),2), "V")
        print("FPGA vccaux: ", round(self.sysmon.vccaux(),2), "V")        
        # power status
        b = self.ucd.read_voltage(1)
        c = self.ucd.read_current(1)
        print("LPC CH_A: ", round(b,2), "V, ", round(c,4), "A")
        b = self.ucd.read_voltage(2)
        c = self.ucd.read_current(2)
        print("LPC CH_B: ", round(b,2), "V, ", round(c,4), "A")

    def read_firmware_info(self): # the date of compiling
        temp = self.reg.read(0, 4)
        year  = "20" + hex(temp[0]).lstrip("0x")
        month = hex(temp[1]).lstrip("0x")
        day   = hex(temp[2]).lstrip("0x")
        hour  = hex(temp[3]).lstrip("0x")
        print("Compiling date: ", year, "-", month, "-", day, ",", hour, ":00")
        temp = self.reg.read(4, 1)
        print("Firmware version: ", temp[0])

    def config(self, buffer):
        printf("Config:\r\n")
        for x in buffer:
            printf("0x%02x ", x)
        printf("\r\n")
        self.spi.writeBytes(buffer)

    def configCheck(self, buffer):
        printf("Config:\r\n")
        for x in buffer:
            printf("0x%02x ", x)
        printf("\r\n")
        tx_buffer = buffer+buffer
        rxbuffer = self.spi.writeRead(tx_buffer)
        # printf("Recv:\r\n")
        # for x in rxbuffer:
        #     printf("0x%02x ", x)
        # printf("\r\n")
        offset = len(buffer)
        i = offset + offset -1
        error = 0
        while i>=offset:
            if tx_buffer[i] != ((rxbuffer[i-1]<<6)|(rxbuffer[i]>>2))&0xFF:
                error += 1
                print("ValidatePattern: Difference is found at byte["+str(i-offset)+"]: "+hex(tx_buffer[i])
                        +" ,recv: "+hex(((rxbuffer[i-1]<<6)|(rxbuffer[i]>>2))&0xFF))
            i = i-1
        return(error)

            

