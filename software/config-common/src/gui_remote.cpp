

#include "Gui.h"
#include "IBGui.h"
#include "Klaus4Config.h"
#include "IBConfig.h"
#include "VCRemoteClient.h"
#include <gtk/gtk.h>



int main (int argc, char *argv[])
{
	if (argc < 2){
		std::cerr << "Usage: gui_remote <host>\n";
		return 1;
	}

	gtk_init (&argc, &argv);


	TGui*   gui_tb = new TGui;
	TIBGui* gui_ib = new TIBGui(1);
	TIBConfig *config_ib;
	config_ib=new VCRemoteClient<TIBConfig>(argv[1],0);
	gui_ib->SetConfiguration(0, config_ib);


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
			char name[255]; sprintf(name,"I2C: %d",addr);
			gui_tb->AddConfiguration(config_tb,name);

		}else{
			printf("HANDLE %d NOT VALID, STOPPING\n",handle);
			delete config_tb;
			break;
		}
		handle++;
	}
	printf("******* FINISHED CONFIG SERVER PROBE ******\n");
	printf("*******        ASICS FOUND: %d       *******\n",handle-1);


	gtk_widget_show (gui_ib->GetWindow());
	gtk_widget_show (gui_tb->GetWindow());
	gtk_main();
	return 0;
}
