

#include "Gui.h"
#include "Klaus4Config.h"
#include "PatternMerger.h"
#include "LinuxSPIdevInterface.h"
#include <cstdlib>
#include <string>
#include <gtk/gtk.h>

//#define NCHIPS 4

int main (int argc, char *argv[])
{

        if(argc != 2) {
          std::cerr << "Please specify a correct chip number. " << std::endl;
          return 0;
        }

        int NCHIPS = std::atoi(argv[1]);

	gtk_init (&argc, &argv);

        TGui*   gui_tb = new TGui;

        TLinuxSPIdevInterface iface("/dev/spidev0.0", 500000);
        iface.SetTimeout(1,1000000,0);
        TPatternMerger merger(&iface);

        int startaddr=0x40;

        TKLauS4Config *configurations[NCHIPS];
        char filename[256];
        for (int i=0;i<NCHIPS;i++){
                long long unsigned int addr;
                configurations[i]=new TKLauS4Config(&merger);
                configurations[i]->SetChipID(i);

                sprintf(filename,"Config_CHIP%d.txt",i);
                configurations[i]->ReadFromFile(filename);
                configurations[i]->SetParValue("digital/i2c_address",0xff&(startaddr+i));
                printf("Changed I2C addres of chip #%d to 0x%2.2x\n",i,0xff&(startaddr+i));
                configurations[i]->GetParValueWR("digital/i2c_address",addr);
                printf("Read I2C addres of chip #%d to 0x%2.2x\n",i,addr);
                char name[255]; sprintf(name,"I2C: %d",addr);
                gui_tb->AddConfiguration(configurations[i],name);
        }

        printf("******* FINISHED CONFIG SERVER PROBE ******\n");
        printf("*******        ASICS FOUND: %d       *******\n", NCHIPS);

        gtk_widget_show (gui_tb->GetWindow());
        gtk_main();

        for (int i=0;i<NCHIPS;i++){
           if(configurations[i]) delete configurations[i]; 
        }
	return 0;
}
