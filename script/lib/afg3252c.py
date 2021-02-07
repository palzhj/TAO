#!/usr/bin/python           
# This is afg3252c.py file
# author: zhj@ihep.ac.cn
# 2019-06-28 created
import pyvisa

class afg3252c():
    def __init__(self, tcpip = '192.168.10.10'):
        self.rm = pyvisa.ResourceManager()
        self.afg = self.rm.open_resource('TCPIP0::192.168.10.10::inst0::INSTR')
        print(self.afg.query("*IDN?")) #Who are you?
        #clear the AFG status
        self.afg.write("*CLS") 

    def setVoltage(self, voltage, ch = 1):
        self.afg.write('SOURce'+str(ch)+':VOlTage:HIGH '+str(voltage)+'mV')
        self.afg.write('SOURce'+str(ch)+':VOlTage:LOW 0mV') 

    def setOutput(self, on = 1, ch = 1):
        if on:
            self.afg.write('OUTPut'+str(ch)+':STATe on')
        else:
            self.afg.write('OUTPut'+str(ch)+':STATe off')

    def setFrequency(self, frequency, ch = 1):
        self.afg.write('SOURce'+str(ch)+':FREQuency:FIXed '+str(frequency)+'Hz')

    def setPolarityINV(self, ch = 1):
        self.afg.write('OUTPut'+str(ch)+':POLarity INV')

    def setPluse(self, width, delay=0, ch=1):
        self.afg.write('SOURce'+str(ch)+':FUNCtion:SHAPe PULSe')
        self.afg.write('SOURce'+str(ch)+':PULSe:WIDTh '+str(width)+'ns')
        self.afg.write('SOURce'+str(ch)+':PULSe:DELay '+str(delay)+'ns')

    def setEdgeLeadin(self, width, ch=1):
        self.afg.write('SOURce'+str(ch)+':PULSe:TRANsition:LEADing '+str(width)+'ns')

    def setEdgeTrailing(self, width, ch=1):
        self.afg.write('SOURce'+str(ch)+':PULSe:TRANsition:TRAiling '+str(width)+'ns')

    def close(self):
        self.rm.close()

    # def __del__(self):
    #     self.rm.close()