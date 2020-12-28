/*
 * klaus4Config.cpp
 *
   Bitpattern definition for the KLauS4.0 miniASIC interface board
 *  Created on: 7.11.2015
 *      Author: Konrad Briggl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>

#include "IBConfig.h"
#include "VirtualConfig.h"
#include "VirtualInterface.h"
#include "bcm2835.h"

void TIBConfig::GPIO_SetDirection(const char* name, const char* dir){
	//open file from name->npar->gpio map
	int ID=GetParID(name);
	if (ID<0 || ID>9) {printf("TIBConfig::GPIO_SetDirection(): wrong parameter name \"%s\"\n",name); return;}
	if (m_gpio_map[ID]<0) {printf("TIBConfig::GPIO_SetDirection(): will not change unmapped GPIO of \"%s\"\n",name); return;}

	//write direction
	if(strcmp(dir,"out")==0){
		printf("TIBConfig::GPIO_SetDirection(): setting GPIO%d (\"%s\") as OUTPUT\n",m_gpio_map[ID],GetParName(ID));
		bcm2835_gpio_fsel(m_gpio_map[ID],BCM2835_GPIO_FSEL_OUTP);
	}
	if(strcmp(dir,"in")==0){
		printf("TIBConfig::GPIO_SetDirection(): setting GPIO%d (\"%s\") as  INPUT\n",m_gpio_map[ID],GetParName(ID));
		bcm2835_gpio_fsel(m_gpio_map[ID],BCM2835_GPIO_FSEL_INPT);
	}
}

void TIBConfig::GPIO_Init(){
	printf("Initializing GPIOs\n");
	if (!bcm2835_init()){
		printf("TIBConfig::GPIO_Init(): bcm library init failed\n"); return;
	}
	

	//set map to gpio
	m_gpio_map[GetParID("monitoring/fifo_full")	]=4;
	m_gpio_map[GetParID("system/reset")		]=17;
	m_gpio_map[GetParID("system/aqu_ena")		]=27;
	m_gpio_map[GetParID("system/clkenable")		]=22;
	m_gpio_map[GetParID("power/vcca18")		]=14;
	m_gpio_map[GetParID("power/vcca33")		]=15;
	m_gpio_map[GetParID("system/screset")		]=18;
	m_gpio_map[GetParID("power/vccd18")		]=23;
	m_gpio_map[GetParID("power/vccd33")]		=24;
	//Set direction
	GPIO_SetDirection("power/vcca33","out");
	GPIO_SetDirection("power/vcca18","out");
	GPIO_SetDirection("power/vccd33","out");
	GPIO_SetDirection("power/vccd18","out");
	GPIO_SetDirection("system/clkenable","out");
	GPIO_SetDirection("system/aqu_ena","in");
	GPIO_SetDirection("system/reset","out");
	GPIO_SetDirection("system/screset","out");
	GPIO_SetDirection("monitoring/fifo_full","in");

	//Initialize GPIOs
	SetParValue("power/vcca33",0);
	SetParValue("power/vcca18",0);
	SetParValue("power/vccd33",0);
	SetParValue("power/vccd18",0);
	SetParValue("system/clkenable",0);
	SetParValue("system/aqu_ena",0);
	SetParValue("system/reset",0);
	SetParValue("system/screset",0);
}


TIBConfig::TIBConfig(TVirtualInterface*)
    :TVirtualConfig(NULL,par_descriptor,9,2)
{
	printf("TIBConfig::TIBConfig(TVirtualInterface*)\n");
	bitpattern_read =  new char[2];
	bitpattern_write = new char[2];
	//init gpio fds
	for (int i=0;i<9;i++){
		m_gpio_map[i]=-1;
	}
};

TIBConfig::~TIBConfig()
{
	delete [] bitpattern_read;
	delete [] bitpattern_write;
	if (!bcm2835_close()){
		printf("TIBConfig::~TIBConfig(): bcm library close failed\n");
	}
};

void TIBConfig::ChipReset(){
    //generate pulse on chip reset pin (rst)
    SetParValue("system/reset",0);
    SetParValue("system/reset",1);
    SetParValue("system/reset",0);
}


void TIBConfig::ConfigReset(){
    //generate pulse on slow reset pin (srst)
    SetParValue("system/screset",0);
    SetParValue("system/screset",1);
    SetParValue("system/screset",0);
}

int TIBConfig::SetParValue(short int npar,unsigned long long value){
	printf("TIBConfig::SetParValue(%s)=%d\n",GetParName(npar),value);
	if (npar<0 || npar>9) {printf("TIBConfig::SetParValueRD(): wrong parameter \"%d\"\n",npar); return -1;}
	if (m_gpio_map[npar]<0) {printf("TIBConfig::SetParValueRD(): will not access unmapped GPIO \"%d\"\n",npar); return -1;}
	unsigned char val=value;

	//write new value
	if(val==0){
		bcm2835_gpio_clr(m_gpio_map[npar]);
	}else 
	if(val==1){
		bcm2835_gpio_set(m_gpio_map[npar]);
	}else{ 
		printf("TIBConfig::SetParValue(%s)=%d: UNDEFINED VALUE!\n",GetParName(npar),value);
	} 
	return 0;
}
int TIBConfig::GetParValueRD(short int npar,unsigned long long &value){
	printf("TIBConfig::GetParValueRD(%s)\n",GetParName(npar));
	if (npar<0 || npar>9) {printf("TIBConfig::GetParValueRD(): wrong parameter \"%d\"\n",npar); return -1;}
	if (m_gpio_map[npar]<0) {printf("TIBConfig::GetParValueRD(): will not access unmapped GPIO \"%d\"\n",npar); return -1;}

	//read value
	unsigned char c=bcm2835_gpio_lev(m_gpio_map[npar]);
	value = c;
	//close file
	printf("TIBConfig::GetParValueRD(%s)=%d\n",GetParName(npar),value);
	return 0;
};

TVirtualConfig::parameter const TIBConfig::par_descriptor[]=
{
	//gpios
	TVirtualConfig::parameter("power/vcca33",	0,0,"Enable vcca33 LDO"),
	TVirtualConfig::parameter("power/vcca18",	1,0,"Enable vcca18 LDO"),
	TVirtualConfig::parameter("power/vccd33",	2,0,"Enable vccd33 LDO"),
	TVirtualConfig::parameter("power/vccd18",	3,0,"Enable vccd18 LDO"),
	TVirtualConfig::parameter("system/clkenable",	4,0,"Oscillator output"),
	TVirtualConfig::parameter("system/aqu_ena",	5,0,"System Aquisition enable"),
	TVirtualConfig::parameter("system/reset",	6,0,"System reset"),
	TVirtualConfig::parameter("system/screset",	7,0,"Slow control register reset"),
	TVirtualConfig::parameter("monitoring/fifo_full",	8,0,"Fifo full flag (Low active)"),
	TVirtualConfig::parameter("",                    9,	0),
};

