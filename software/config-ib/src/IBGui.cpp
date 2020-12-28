/*
* GUI class to handle the signals from the main window, changing the underlying TVirtualConfig parameter set.
* it can control several VirtualConfig instances, by using a "Chip-Channel-Scope" to adress the different asics.
*
* Also includes some tools to copy configuration data from one channel to another, and communication with a parent-like window
* (e.g. a System overview window for a combined ctrl&daq framework)
*
* Author: kbriggl
* Date:   Feb/13
* Updates Sep 2016: GUI for KLauS4
*
*/


#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "IBGui.h"

//#include "resources.h"
#include "HandlerProxies.h"
//using namespace std;



//gui contructor
TIBGui::TIBGui(int nIB):
nIBs(nIB),
block_val_changed(false)
{
	builder = gtk_builder_new ();
	gtk_builder_add_from_resource (builder, "/configuration/glade_xml/ibconfig_window.xml", NULL);
      //gtk_builder_add_from_file(builder, "./glade_xml/config_window.xml", NULL);
	window = 		GTK_WIDGET (	gtk_builder_get_object (builder, "ibctrl_window"));

	gtk_builder_connect_signals_full( builder , TIBGui::connection_mapper, (gpointer)this);

	scope.IB=0;
	config=new TIBConfig* [nIBs];
	for(int i=0;i<nIBs;i++)
		config[i]=NULL;
};


//gui wnd destructor
TIBGui::~TIBGui(){
	delete [] config;
};


/* connect the signals with the class' member function.
 * This method has to be static in order to be callable by a standard c function pointer.
 * The this pointer is passed over the user data parameter
 */
void TIBGui::connection_mapper (GtkBuilder *builder, GObject *object,
        const gchar *signal_name, const gchar *handler_name,
        GObject *connect_object, GConnectFlags flags, gpointer user_data)
{
	gpointer this__=user_data;
	if(g_strcmp0 (handler_name, "on_window_destroy") == 0){
		g_signal_connect(object,signal_name,
    				G_MEMBER_CALLBACK1(TIBGui, on_window_close, GObject*),  this__);
		return;
	}
	if(g_strcmp0 (handler_name, "on_switch_changed") == 0){
		printf("Connecting the signal %s for object %s\n", signal_name, (char*)gtk_buildable_get_name((GtkBuildable*)object));
		g_signal_connect(object,signal_name,
   				G_MEMBER_CALLBACK1(TIBGui, on_switch_changed, GObject*),  this__);
		return;
	}
	if(g_strcmp0 (handler_name, "on_btn_clicked") == 0){
		g_signal_connect(object,signal_name,
    				G_MEMBER_CALLBACK1(TIBGui, on_btn_clicked , GObject*),  this__);
		return;
	}
	if(g_strcmp0 (handler_name, "on_val_changed") == 0){
		g_signal_connect(object,signal_name,
    				G_MEMBER_CALLBACK1(TIBGui, on_val_changed, GObject*),  this__);
		return;
	}
	if(g_strcmp0 (handler_name, "on_val_changed2") == 0){
		int ret;
		ret=g_signal_connect(object,signal_name,
    				G_MEMBER_CALLBACK2(TIBGui, on_val_changed2, GObject*,gboolean),  this__);
		if(ret==0){
			printf("Using alternative handler for signal on_val_changed2, probably due to old gtk version...\n",ret);
			g_signal_connect(object,"notify::active",
					G_MEMBER_CALLBACK2(TIBGui, on_val_changed3, GObject*, GParamSpec*),  this__);
		}
		return;
	}
	if(g_strcmp0 (handler_name, "on_scope_changed") == 0){
		g_signal_connect(object,signal_name,
    				G_MEMBER_CALLBACK1(TIBGui, on_scope_changed , GObject* ),  this__);
		return;
	}
	printf("!! Unhandled signal: %s to %s\n",signal_name, handler_name);
}

/* (GUI-)User wants to select a different board*/
void TIBGui::on_scope_changed (GObject *object){
	std::string obj_name=(char*)gtk_buildable_get_name((GtkBuildable*)object);

	//GtkComboBox *ibscale=(GtkComboBox*)gtk_builder_get_object(builder,"val:ibscope");
	//scope.IB=gtk_combo_box_get_active(ibscale);

	//printf("Selected IB: %d\n",scope.IB);
	//UpdateScope();
}


/* Select a different chip/channel or the general config page, update all parameters
   update all widgets to show the current channel's chip's settings and old values
   after the scope has been changed by the user
*/
void TIBGui::ChangeScope(int IB){
	scope.IB=IB;
	//update selection widgets
	//GtkComboBox *channelscale=(GtkComboBox*)gtk_builder_get_object(builder,"val:channelscope");
	//GtkComboBox *chipscale=(GtkComboBox*)gtk_builder_get_object(builder,"val:chipscope");
	//gtk_combo_box_set_active(chipscale,scope.chip);
	//gtk_combo_box_set_active(channelscale,scope.channel);
	UpdateScope();
}

void TIBGui::SetWidgetValue(GObject* obj,unsigned long long value){
	char* object_type_name;
	//determine object type to set their value
	object_type_name=(char*)gtk_widget_get_name((GtkWidget*)obj);
	//printf("%s\n",object_type_name);
	if (strcmp(object_type_name,"GtkCheckButton")==0){
		gtk_toggle_button_set_active((GtkToggleButton*)obj,value);
	}else
	if (strcmp(object_type_name,"GtkComboBoxText")==0){
		char s[20]; sprintf(s,"%d",value);
		gtk_combo_box_set_active_id((GtkComboBox*)obj,s);
	}else
	if (strcmp(object_type_name,"GtkSpinButton")==0){
		gtk_spin_button_set_value((GtkSpinButton*)obj,value);
	}else
	if (strcmp(object_type_name,"GtkSwitch")==0){
		gtk_switch_set_active (GTK_SWITCH (obj), value);
	}else{
		printf("ERROR: unrecognized type of GtkObject (%s)\n",object_type_name);
	}
}
void TIBGui::SetOldValueLabel(GObject* obj,unsigned long long value){
	std::stringstream s; s<<"("<<value<<")";
	gtk_label_set_text((GtkLabel*) obj,s.str().c_str());
}

void TIBGui::UpdateRD(){
	if(config[scope.IB]==NULL){
		printf("configuration is NULL!");
		return;
	}
	//update page widgets
	GObject* obj=NULL;
	unsigned long long value;

	for (int par=0;par<config[scope.IB]->GetNParameters();par++){
		//set old values
		config[scope.IB]->GetParValueRD(par,value);
		obj=gtk_builder_get_object(builder,scope.widget_name("old:",config[scope.IB]->GetParName(par)).c_str());
		if (obj!=NULL) {
			SetOldValueLabel(obj,value);
		}else{
			printf("ERROR: could not find widget for parameter %s! (%s)\n",config[scope.IB]->GetParName(par),scope.widget_name("old:",config[scope.IB]->GetParName(par)).c_str());
			continue;
		}
	}
	return;
}



void TIBGui::UpdateWR(){
	if(config[scope.IB]==NULL){
		printf("configuration is NULL!");
		return;
	}
	//update page widgets
	GObject* obj=NULL;
	unsigned long long value;

	block_val_changed=true;//block the value_changed signal
	for (int par=0;par<config[scope.IB]->GetNParameters();par++){
		//set current values
		config[scope.IB]->GetParValueWR(par,value);
		//get widget
		obj=gtk_builder_get_object(builder,scope.widget_name("val:",config[scope.IB]->GetParName(par)).c_str());
		if (obj!=NULL){
			SetWidgetValue(obj,value);
			gtk_widget_set_tooltip_text ((GtkWidget*)obj,config[scope.IB]->GetParDescription(par).c_str());
		}else{
			printf("ERROR: could not find widget for parameter %s! (%s)\n",config[scope.IB]->GetParName(par),scope.widget_name("val:",config[scope.IB]->GetParName(par)).c_str());
			continue;
		}
	}
	block_val_changed=false;//unblock the value_changed signal

	return;
}

void TIBGui::UpdateScope(){
	UpdateWR();
	UpdateRD();	
};

//signal handlers
void TIBGui::on_btn_clicked (GObject *object){

	GdkRGBA red = {1.0, .0, .0, 1.0};
	GdkRGBA green = {0.0, 1.0, .0, 1.0};
	int r;
	//printf("TIBGui::on_btn_clicked\n");
	//what button?
	std::string btn_name=(char*)gtk_buildable_get_name((GtkBuildable*)object);

	if (btn_name.compare("btn_open")==0){
	//"OPEN" button
		GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File",
						(GtkWindow*)window,
						GTK_FILE_CHOOSER_ACTION_OPEN,
						("_Cancel"), GTK_RESPONSE_CANCEL,
						("_Open"), GTK_RESPONSE_ACCEPT,
						NULL);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "./");
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
			config[scope.IB]->ReadFromFile(filename);
			g_free (filename);
			UpdateScope();
		}
		gtk_widget_hide (dialog);
	}else
	if (btn_name.compare("btn_save")==0){
	//"SAVE" button
		GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save File",
						(GtkWindow*)this->window,
						GTK_FILE_CHOOSER_ACTION_SAVE,
						("_Cancel"), GTK_RESPONSE_CANCEL,
						("_Save"), GTK_RESPONSE_ACCEPT,
						NULL);
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "./");
		char fname[50]; sprintf(fname,"Config_CHIP%d.txt",scope.IB);
		gtk_file_chooser_set_current_name   (GTK_FILE_CHOOSER (dialog), fname);
		if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		{
			char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
			config[scope.IB]->Print(true,filename);
			g_free (filename);
		}
		gtk_widget_hide (dialog);
	}else
	if (btn_name.compare("btn_rst")==0){
	//"SYSTEM RESET" button
		printf("____Resetting Chips on IB #%d\n",scope.IB);
		config[scope.IB]->ChipReset();
	}else
	if (btn_name.compare("btn_srst")==0){
	//"SLOW CONTROL RESET" button
		printf("____Resetting Slow control registers of IB #%d\n",scope.IB);
		config[scope.IB]->ConfigReset();
	}else
		printf("configuration: unknown btn_clicked sender!\n");

};

void TIBGui::on_switch_changed (GObject *object){
	printf("TIBGui::on_switch_changed (Not Implemented)\n");
	std::string switch_name=(char*)gtk_buildable_get_name((GtkBuildable*)object);
};

void TIBGui::on_val_changed (GObject *object){
	if(block_val_changed)
		return;
	int bit;
	char* object_type_name=(char*)gtk_widget_get_name((GtkWidget*)object);
	unsigned long long value;
	if (strcmp(object_type_name,"GtkCheckButton")==0){
		value=gtk_toggle_button_get_active((GtkToggleButton*)object);
	}else if (strcmp(object_type_name,"GtkSwitch")==0){
		value=gtk_switch_get_active((GtkSwitch*)object);
	}else if (strcmp(object_type_name,"GtkComboBoxText")==0){
		sscanf(gtk_combo_box_get_active_id((GtkComboBox*)object),"%lld",&value);
	}else if ( strcmp(object_type_name,"GtkSpinButton")==0 ){
		value=gtk_spin_button_get_value_as_int((GtkSpinButton*)object);
	}else{
		printf("ERROR: unrecognized type of GtkObject: %s \n",object_type_name);
		return;
	}
	std::string parname=scope.param_name("val:",(char*)gtk_buildable_get_name((GtkBuildable*)object),bit);
	if(parname.length()==0) //ignore this, duplicate handler action from other widget connected to the same adjustment
		return;
	//Update value
	if(bit>=0){
		unsigned long long oldval;
		config[scope.IB]->GetParValueWR(parname.c_str(),oldval);
		if(value==0)
			value=oldval&~(1<<bit);
		else
			value=oldval|(1<<bit);
	}
	printf("new setting of Parameter %s = %llu\n",parname.c_str(),value);
	if (config[scope.IB]->SetParValue(parname.c_str(),value) < 0)
		printf("ERROR: did not find Parameter \"%s\" in configuration\n",parname.c_str());
	//FOR IB GUI: UPDATE THE READBACK VALUES
	UpdateRD();
};


void TIBGui::on_window_close(GObject *object){
	char filename[200];
	for (int c=0;c<nIBs;c++){
		sprintf(filename,".lastquit_ibconfig_BRD%d",c);
		config[c]->Print(true,filename);
	}
	gtk_main_quit();
}

