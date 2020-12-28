
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include "VCRemoteServer.h"

#include "Klaus4Config.h"
#include "IBConfig.h"

#include "PatternMerger.h"
#include "LinuxSPIdevInterface.h"
int main(int argc, char* argv[]){
	int n;
	if(argc<2){
		printf("Usage: %s nchips [i2c-startaddr=0x40]\n",argv[0]);
		exit(-1);
	}
	
	n=atoi(argv[1]);
	if(n<1){
		printf("Usage: %s nchips [i2c-startaddr=0x40]\n",argv[0]);
		exit(-1);
	}

	int startaddr=0x40;
	if((argc>2) && (sscanf(argv[2],"%x",&startaddr)!=1)){
		printf("Usage: %s nchips [i2c-startaddr=0x40]\n",argv[0]);
		exit(-1);
	}

	TLinuxSPIdevInterface iface("/dev/spidev0.0", 500000);
	iface.SetTimeout(1,1000000,0);
	TPatternMerger merger(&iface);

	VCRemoteServer s1("1234");
	s1.AddConfiguration(new TIBConfig(NULL)); //ID=0
	((TIBConfig*)s1.GetConfiguration(0))->GPIO_Init();
	((TIBConfig*)s1.GetConfiguration(0))->GPIO_SetDirection("system/aqu_ena","out");

	//Configuration object for each chip, load standard config
	for(int i=0;i<n;i++){
		s1.AddConfiguration(new TKLauS4Config(&merger)); //ID=1+n
		s1.GetConfiguration(i+1)->ReadFromFile("Config_CHIPstd.txt");
	}
	for(int i=0;i<n;i++){
		s1.GetConfiguration(i+1)->SetParValue("digital/i2c_address",0xff&(startaddr+i));
		printf("Changed I2C addres of chip #%d to 0x%2.2x\n",i,0xff&(startaddr+i));
	}
	s1.Run();
	return 0;
}
