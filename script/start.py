#!/usr/bin/python           
# This is server.py file
# author: zhj@ihep.ac.cn
# 2019-06-18 created

from time import sleep
import sys
sys.path.insert(0, "lib")
import rbcp
import i2c
import sysmon

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

############################################################
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

# TCA9548_ADDR = 0x70 << 1
# tca9548 = i2c.i2c(TCA9548_ADDR)
# # b = tca9548.read8()
# # printf("0x%02x\r\n", b)
# tca9548.write8(1<<7|1<<3|1<<0) # select eeprom, si570 and si5324
# b = tca9548.read8()
# printf("I2C MUX: 0x%02x\r\n", b)

###########################################
# SI5324_ADDR = 0x68 <<1 # clk address
# si5324 = i2c.i2c(SI5324_ADDR)

# b = si5324.read8(True, 134)
# printf("0x%02x\r\n", b)
# b = si5324.read8(True, 135)
# printf("0x%02x\r\n", b)

###########################################
# SI570_ADDR = 0x5D <<1 # clk address
# si570 = i2c.i2c(SI570_ADDR)

# b = si570.read8(True, 7)
# printf("0x%02x\r\n", b)
# b = si570.read8(True, 8)
# printf("0x%02x\r\n", b)

###########################################
# M24C08_ADDR = 0x54 << 1
# m24c08 = i2c.i2c(M24C08_ADDR)
# b = m24c08.read8(True, 0)
# printf("0x%02x\r\n", b)

# b = m24c08.read16(True, 0)
# printf("0x%04x\r\n", b)

# # c = b'Hello World!'
# # m24c08.writeBytes(c, True, 0)

# b = m24c08.readBytes(16, True, 0)
# print(b)

#################################################################

