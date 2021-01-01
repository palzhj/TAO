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

device_addr_offset = 0
#################################################################
# KLauS6
KLAUS_ADDR = (0x40+device_addr_offset) << 1
klaus6 = klaus6.klaus6(KLAUS_ADDR, LPC_PORTA_BASE_ADDR)

# for i in range(32):
# 	c = klaus6.read8(True, i)
# 	printf("0x%02x@%d\r\n",c,i)

c = klaus6.readEvent()
printf ("0x%02x%02x_%02x%02x%02x%02x\r\n",c[0],c[1],c[2],c[3],c[4],c[5])
