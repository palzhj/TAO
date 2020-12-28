

#include "Klaus4Config.h"
#include "IBConfig.h"
#include "VCRemoteClient.h"



int main (int argc, char *argv[])
{
	if (argc < 3){
		std::cerr << "Usage: save_current <host> <basename>\n";
		return 1;
	}


	printf("******* STARTING CONFIG SERVER PROBE ******\n");
	printf("Probing config-server @ %s\n",argv[1]);
	//connect to config server, try binding to handle id 1++ until the connection fails.
	//for succeeded connections, store the I2C address of this ASIC
	printf("Getting list of ASICs from config-server @ %s\n",argv[1]);
	int handle=1;
	while(1){

		VCRemoteClient<TKLauS4Config>* config_tb=new VCRemoteClient<TKLauS4Config>(argv[1],handle);
		if(config_tb->Socket(false)>0){
			long long unsigned int addr;
			config_tb->GetParValueWR("digital/i2c_address",addr);
			printf("HANDLE %d -> ADDR 0x%x\n",handle,addr);
			char name[255]; sprintf(name,"%s%d.txt",argv[2],handle-1);
			printf("Saving to file : %s\n",name);
    			config_tb->Print(1,name);
			
		}else{
			printf("HANDLE %d NOT VALID, STOPPING\n",handle);
			delete config_tb;
			break;
		}
		handle++;
	}
	printf("******* FINISHED CONFIG SERVER PROBE ******\n");
	printf("*******        ASICS FOUND: %d       *******\n",handle-1);
	return 0;
}
