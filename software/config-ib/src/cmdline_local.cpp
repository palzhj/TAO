

#include "IBConfig.h"

//#include <unistd.h>
//#include <iostream>

int main (int argc, char **argv)
{
	int arg=0;
	
	if (argc<2){
		printf("Usage: ./cmdline_config [paramname value]+\n");
		return -1;
	}

	TIBConfig config;
	config.GPIO_Init();
	while (arg-argc>=2){
		printf("\"%s\" \"%s\"\n",argv[arg],argv[arg+1]);
		config.SetParValue(argv[arg],atoi(argv[arg+1]));
		arg+=2;
	}
	return 0;
}
