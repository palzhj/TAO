#!/usr/bin/python           
# This is server.py file
# author: zhj@ihep.ac.cn
# 2019-06-18 created

from time import sleep
import sys
sys.path.insert(0, "lib")
import rbcp
import i2c
import mux
import sysmon
import ucd90xxx
import gpio
import spi

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

#################################################################
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
print("CH0: ", b,"V, ", c,"A")
b = ucd.read_voltage(2)
c = ucd.read_current(2)
print("CH0: ", b,"V, ", c,"A")

#################################################################
# MUX
device_addr_offset = 0

TCA9548_ADDR = (0x70+device_addr_offset) << 1
tca9548 = mux.mux(TCA9548_ADDR, LPC_PORTA_BASE_ADDR)

tca9548.enable_eeprom()
tca9548.enable_gpio()
tca9548.enable_spi()

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

#################################################################
# SPI
SPI_ADDR = (0x28+device_addr_offset) << 1
spi = spi.spi(SPI_ADDR, LPC_PORTA_BASE_ADDR)

# length = 401
# a = bytearray(length)
# for i in range(length):
# 	if(i<200):
# 		a[i] = 0xF0
# 	elif (i<400):
# 		a[i] = 0x67
# 	else:
# 		a[i] = 0x45
# print(a)
def To_Bytes(s):
	tmp = b''
	for i in range(0, len(data), 8):
		a = int(s[0+i:8+i], 2)
		b = bytes([a])
		tmp += b
		# if len(tmp) >= 200:
		# 	return tmp
	return tmp
with open('test.txt') as file:
	data = file.read()
	a = To_Bytes(data)
	# print(a)
	# print(len(a))

spi.writeBytes(a)
sleep(1)
d = spi.readBytes(200)
print(d)
# if d == a:
# 	print('True')
# else:
# 	print('Error')



