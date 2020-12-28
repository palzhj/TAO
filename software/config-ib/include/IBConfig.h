/*
 * IBConfig.h
 *
   Header to the Bitpattern definition for the KLauS4.0 interface board control
 *  Created on: 7.11.09.2016
 *      Author: Konrad Briggl 
 */
#ifndef K4IBCONFIG_H__
#define K4IBCONFIG_H__


#include "VirtualConfig.h"
#include "VirtualInterface.h"



class TIBConfig: public TVirtualConfig
{
private:
//konfiguration
    static parameter const par_descriptor[];

    int m_gpio_map[9];

public:
    //CON/D-STRUCTOR:
    TIBConfig(TVirtualInterface* ifa=NULL); //for consistency with other config classes
    ~TIBConfig();
    //Open and preconfigure GPIOs
    void GPIO_Init();

    //LOW LEVEL GPIO CONFIGURATION
    void GPIO_SetDirection(const char* name, const char* dir);
    //GPIO READ/WRITE
    using TVirtualConfig::SetParValue;
    virtual int SetParValue(short int npar,unsigned long long value);
    using TVirtualConfig::GetParValueRD;
    using TVirtualConfig::GetParValueWR;
    virtual int GetParValueRD(short int npar,unsigned long long &value);
    //HIGHER LEVEL COMMANDS
    void ChipReset();
    void ConfigReset();

};


#endif
