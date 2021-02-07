#!/usr/bin/python           
# This is gpio.py file
# TCA9554
# author: zhj@ihep.ac.cn
# 2020-12-28 created
import socket
import lib
from lib import rbcp

class sitcp(object):
    def __init__(self, tcp_ip = '192.168.10.16', tcp_port = 24):
        self.tcp_ip = tcp_ip
        self.tcp_port = tcp_port
        self.reg = rbcp.Rbcp()
        self.reg.write(0xFFFFFF10, bytes(0x7)) # Keep alive packet,  Fast retrains,  Nagle buffering
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def open(self):
        self.socket.connect((self.tcp_ip, self.tcp_port))

    def getDataLength(self):
        length = self.reg.read(0x30, 4)
        return (length[0]<<24 | length[1]<<16 | length[2]<<8 | length[3])  

    def readEvents(self):
        length = self.getDataLength()
        return self.socket.recv(length)

    def getLinkStatus(self, i=0):
        temp = self.reg.read(0x20, 8)
        status = temp[0]<<56 | temp[1]<<48 | temp[2]<<40 | temp[3]<<32 | temp[4]<<24 | temp[5]<<16 | temp[6]<<8 | temp[7]
        if((1<<i) & status):
            return 1
        else:
            return 0

    def resetLink(self, i=0):
        temp = 1<<i
        # Set
        self.reg.write(0x48, bytes(0xff&(temp>>56)))
        self.reg.write(0x49, bytes(0xff&(temp>>48)))
        self.reg.write(0x4a, bytes(0xff&(temp>>40)))
        self.reg.write(0x4b, bytes(0xff&(temp>>32)))
        self.reg.write(0x4c, bytes(0xff&(temp>>24)))
        self.reg.write(0x4d, bytes(0xff&(temp>>16)))
        self.reg.write(0x4e, bytes(0xff&(temp>>8)))
        self.reg.write(0x4f, bytes(0xff&(temp>>0)))
        # Clear
        self.reg.write(0x48, bytes(0))
        self.reg.write(0x49, bytes(0))
        self.reg.write(0x4a, bytes(0))
        self.reg.write(0x4b, bytes(0))
        self.reg.write(0x4c, bytes(0))
        self.reg.write(0x4d, bytes(0))
        self.reg.write(0x4e, bytes(0))
        self.reg.write(0x4f, bytes(0))

    def close(self):
        self.socket.close()

    # def __del__(self):
    #     self.socket.close()
