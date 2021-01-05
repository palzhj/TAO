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

LPC_PORTA_BASE_ADDR = 0x200
LPC_PORTB_BASE_ADDR = 0x300
LPC_PMBUS_BASE_ADDR = 0x400
HPC_PORTA_BASE_ADDR = 0x500
HPC_PORTB_BASE_ADDR = 0x600
HPC_PMBUS_BASE_ADDR = 0x700

def printf(format, *args):
    sys.stdout.write(format % args)

LED_ADDR = 0x06

def shift_led():
	reg.write(LED_ADDR,'\x80')
	for i in range(8):
		b = reg.read(LED_ADDR, 1)
		b[0] = b[0] >> 1
		if b[0] == 0:
			b[0] = 0x80
		reg.write(LED_ADDR , b)
		sleep(0.5)

def read_info(): # the date of compiling
	temp = reg.read(0, 4)
	year  = "20" + hex(temp[0]).lstrip("0x")
	month = hex(temp[1]).lstrip("0x")
	day   = hex(temp[2]).lstrip("0x")
	hour  = hex(temp[3]).lstrip("0x")
	print("Compiling date: ", year, "-", month, "-", day, ",", hour, ":00")
	temp = reg.read(4, 1)
	print("Firmware version: ", temp[0])

def runSPI(config_bin = "../software/config-k4/Config_bitflow_chip0.txt"):

    CONFIG_BITS = 2463
    file = open(config_bin, "rb")
    bytes_all = file.read()
    file.close()
    # print (bytes_all)
    bytes_len = len(bytes_all)
    if (bytes_len != 2+CONFIG_BITS//8):
        raise ValueError("Invalid config bin file!")
    config_all = bytearray(bytes_len-1)
    # LSB first
    # for i in range(bytes_len-1):
    #   config_all[i] = (bytes_all[bytes_len-2-i]<<1)&0xFF | bytes_all[bytes_len-1-i]>>7 # Align to byte
    # config_all[0] &= 0xFE

    # MSB first
    for i in range(bytes_len-1):
      config_all[i] = bytes_all[i]  
        
    print ("Config KLaus6")
    print (config_all)
    spi.writeBytes(config_all)
  
    sleep(1)
    gpio.acq_en_set()


#################################################################
# main

reg = rbcp.Rbcp()
read_info()
# shift_led()

sysmon = sysmon.sysmon()
print("FPGA temperature: ", round(sysmon.temperature(),2), "C")
print("FPGA vccint: ", round(sysmon.vccint(),2), "V")
print("FPGA vccaux: ", round(sysmon.vccaux(),2), "V")
# print(sysmon.vpvn())
# print(sysmon.vrefp())
# print(sysmon.vrefn())
# print(sysmon.vccbram())
# print(sysmon.temperature_max())
# print(sysmon.vccint_max())
# print(sysmon.vccaux_max())
# print(sysmon.vccbram_max())
# print(sysmon.temperature_min())
# print(sysmon.vccint_min())
# print(sysmon.vccaux_min())
# print(sysmon.vccbram_min())

#################################################################
UCD_ADDR = 0x67 << 1
ucd = ucd90xxx.ucd90xxx(UCD_ADDR, LPC_PMBUS_BASE_ADDR)
# b = ucd.read_device_id()
# print(b)
b = ucd.read_voltage(1)
c = ucd.read_current(1)
print("LPC CH_A: ", round(b,2), "V, ", round(c,4), "A")
b = ucd.read_voltage(2)
c = ucd.read_current(2)
print("LPC CH_B: ", round(b,2), "V, ", round(c,4), "A")

#################################################################
# MUX
device_addr_offset = 0

TCA9548_ADDR = (0x70+device_addr_offset) << 1
tca9548 = mux.mux(TCA9548_ADDR, LPC_PORTA_BASE_ADDR)

tca9548.enable_eeprom()
tca9548.enable_gpio()
tca9548.enable_spi()
tca9548.enable_i2ca()

###########################################
# EEPROM
EEPROM_ADDR = (0x50+device_addr_offset) << 1
eeprom = i2c.i2c(EEPROM_ADDR, LPC_PORTA_BASE_ADDR)

# c = b'Hello World!'
# eeprom.writeBytes(c, True, 0)
# sleep(1)

b = eeprom.readBytes(16, True, 0)
print("EEPROM: ",b)

#################################################################
# GPIO
GPIO_ADDR = (0x38+device_addr_offset) << 1
gpio = gpio.gpio(GPIO_ADDR, LPC_PORTA_BASE_ADDR)
gpio.cal_sel_reset() # select INT
gpio.acq_en_reset()
gpio.srst_set()
gpio.rst_set()
sleep(0.1)
gpio.srst_reset()
gpio.rst_reset()
sleep(0.1)

#################################################################
# SPI
SPI_ADDR = (0x28+device_addr_offset) << 1
spi = spi.spi(SPI_ADDR, LPC_PORTA_BASE_ADDR)

if len(sys.argv) < 2:
  config_bin_file = "Config_bitflow_chip0.txt"
else:
  config_bin_file = sys.argv[1]

CONFIG_BIN_FOLDER = "../software/config-k4/"
config_bin = CONFIG_BIN_FOLDER+config_bin_file

runSPI(config_bin)

#################################################################
# KLauS6
# KLAUS_ADDR = (0x40+device_addr_offset) << 1
# klaus6 = klaus6.klaus6(KLAUS_ADDR, LPC_PORTA_BASE_ADDR)

# for i in range(32):
#   c = klaus6.read8(True, i)
#   printf("0x%02x@%d\r\n",c,i)

# c = klaus6.readEvent()
# print (c)
