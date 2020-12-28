/*
 * klaus4Config.h
 *
   Header to the Bitpattern definition for the KLauS4.0 miniASIC
 *  Created on: 20.09.2016
 *      Author: Konrad Briggl 
 */
#ifndef KLAUS4CONFIG_H__
#define KLAUS4CONFIG_H__


#include "VirtualConfig.h"
#include "VirtualInterface.h"



class TKLauS4Config: public TVirtualConfig
{
private:
//konfiguration
    static parameter const par_descriptor[];



public:
    TKLauS4Config(TVirtualInterface* Interface);
    ~TKLauS4Config();

    void ChipReset();
};


#endif
