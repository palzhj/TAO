

#include "IBGui.h"
#include "IBConfig.h"
#include "VCRemoteClient.h"

#include <gtk/gtk.h>

#define NIBS 1

int main (int argc, char *argv[])
{
	if (argc < 2){
		std::cerr << "Usage: gui_remote <host>\n";
		return 1;
	}

	gtk_init (&argc, &argv);
	TIBGui *gui_wnd = new TIBGui(NIBS);
	TIBConfig *configurations[NIBS];
	char filename[256];
	for (int i=0;i<NIBS;i++){
		configurations[i]=new VCRemoteClient<TIBConfig>(argv[1],0);
		sprintf(filename,"Config_IB%d.txt",i);
		configurations[i]->ReadFromFile(filename);
		gui_wnd->SetConfiguration(i, configurations[i]);
	}
	gtk_widget_show (gui_wnd->GetWindow());
	gtk_main();
	for (int i=0;i<NIBS;i++){
		delete configurations[i];
	}
	return 0;
}
