/*
* GUI class to handle the signals from the main window, changing the underlying TVirtualConfig parameter set.
* it can control several VirtualConfig instances, by using a "Chip-Channel-Scope" to adress the different asics.
*
* Also includes some tools to copy configuration data from one channel to another, and communication with a parent-like window 
* (e.g. a System overview window for a combined ctrl&daq framework)
*
* Basically written in gtk+, although the class is derived from a gtkmm window widget.
* 
* Author: kbriggl
* Date:   Feb/13
* Updates Sep 2016: Gui for KLauS4, a bit more generic
*
*/




#ifndef  IBGUI_H__
#define  IBGUI_H__

#include <gtk/gtk.h>
#include <map>
#include <sstream>
//configuration class
#include "IBConfig.h"
#include <string.h>
class TIBGui
{

public:
	TIBGui(int nIB);

	virtual ~TIBGui();

	static void connection_mapper (GtkBuilder *builder, GObject *object,
		const gchar *signal_name, const gchar *handler_name,
		GObject *connect_object, GConnectFlags flags, gpointer user_data);

	

	void SetConfiguration(int ID, TIBConfig* conf){
		if(ID>=0 && ID<nIBs)
			config[ID]=conf;
		UpdateScope();
	}

	/* Update the labels and settings in the newly selected scope. Called by on_scope_changed or parent */
	virtual void ChangeScope(int ib);

//signal handlers
	virtual void on_btn_clicked (GObject *object);
	virtual void on_switch_changed (GObject *object);
	virtual void on_val_changed (GObject *object);
	virtual void on_val_changed2 (GObject *object, gboolean state ){on_val_changed(object);};
	virtual void on_val_changed3 (GObject *object, GParamSpec* ){on_val_changed(object);};
	virtual void on_scope_changed (GObject *object);
	virtual void on_window_close(GObject *object);

	
	GtkWidget* GetWindow(){ return GTK_WIDGET(window);};

protected:

	void SetWidgetValue(GObject* obj,unsigned long long value);
	void SetOldValueLabel(GObject* obj,unsigned long long value);
	virtual void UpdateWR(); //Update written values
	virtual void UpdateRD(); //Update read back values
	virtual void UpdateScope(); //Update everything (RD&WR)


	TIBConfig** config;


	GtkBuilder      *builder; 
	GtkWidget	*window;
	struct{
		int IB;
		std::string widget_name(const char* type_prefix,const char* parname,int bit=-1){
			std::string s=parname;
			//s.erase(0,prefix_param().length());
			//s.insert(0,prefix_widget(type_prefix));
			s.insert(0,type_prefix);
			if(bit>=0){
				std::stringstream ss;
				ss<<s<<"["<<bit<<"]";
				return ss.str();
			}
			//printf("Param: %s --> %s\n",parname,s.c_str());
			return s;
		}
		std::string param_name(const char* type_prefix,const char* widget_name,int& bit){
			std::string s=widget_name;
			//check if change happened in current scope, otherwise ignore
			/*
			if(prefix_widget(type_prefix).compare(\
				0,prefix_widget(type_prefix).length(),s,\
				0,prefix_widget(type_prefix).length()\
			)!=0){
				printf("Widget: %s : ignored in scope of %s\n",widget_name,prefix_widget(type_prefix).c_str());
				return "";
			}
			*/

			s.erase(0,strlen(type_prefix));
			//s.insert(0,prefix_param());
			if(s[s.length()-1]==']'){ //is a bitwise widget, extract bit number
				sscanf(&(s[s.find_last_of('[')]),"[%d]",&bit);
				s.erase(s.find_last_of('['));
			}else bit=-1;

			//printf("Widget: %s --> %s bit %d\n",widget_name,s.c_str(),bit);
			return s;
		}
	} scope;
	int nIBs;
	bool block_val_changed;
};




#endif // STIC3GUI_H__
