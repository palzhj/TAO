

#include "IBGui.h"
#include "IBConfig.h"

#include <gtk/gtk.h>

#define NIBS 1

int main (int argc, char *argv[])
{

	gtk_init (&argc, &argv);

	TIBGui *gui_wnd = new TIBGui(NIBS);
	TIBConfig *configurations[NIBS];
	char filename[256];
	for (int i=0;i<NIBS;i++){
		configurations[i]=new TIBConfig();

		sprintf(filename,"Config_IB%d.txt",i);
		configurations[i]->ReadFromFile(filename);
		configurations[i]->GPIO_Init();
		gui_wnd->SetConfiguration(i, configurations[i]);
	}
	gtk_widget_show (gui_wnd->GetWindow());
	gtk_main();
	for (int i=0;i<NIBS;i++){
		delete configurations[i];
	}
	return 0;
}
