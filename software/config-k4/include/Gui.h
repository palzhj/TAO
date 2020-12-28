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




#ifndef  GUI_H__
#define  GUI_H__

#include <gtk/gtk.h>
#include <map>
#include <sstream>
//configuration class
#include "Klaus4Config.h"
#include "MQInterface.h"
#include "expand.h"
#include <vector>
#include <iostream>
class TGui
{

public:
	TGui();

	virtual ~TGui();

	enum PAGES{
		DIGITAL_PAGE=0,
		PGATING_PAGE=1,
		BIAS_PAGE=2,
		CHANNEL_PAGE=3,
		//TESTCHANNEL_PAGE=4
		
	};
	static void connection_mapper (GtkBuilder *builder, GObject *object,
		const gchar *signal_name, const gchar *handler_name,
		GObject *connect_object, GConnectFlags flags, gpointer user_data);

	

	void AddConfiguration(TKLauS4Config* conf, std::string name_str);

	/* Update the labels and settings in the newly selected scope. Called by on_scope_changed or parent */
	virtual void ChangeScope(int chip, int channel,TGui::PAGES page);

	/*Initialize the interface to configure the FPGA settings*/
	void InitFPGAInterface(TMQInterface *iface);


//signal handlers
	virtual void on_btn_clicked (GObject *object);
	virtual void on_switch_changed (GObject *object);
	virtual void on_val_changed (GObject *object);
	virtual void on_val_changed2 (GObject *object, gboolean state ){on_val_changed(object);};
	virtual void on_scope_changed (GObject *object);
	virtual void on_page_changed (GtkNotebook* notebook, GtkWidget* page, guint page_num);
	virtual void on_window_close(GObject *object);

	
	GtkWidget* GetWindow(){ return GTK_WIDGET(window);};

protected:
	//copy channel configuration data
	void CopyChanInfo(PattExp::copyset &cset);

	// mask and unmask all channels
	void SetAllChannels(const char* pattern, unsigned long long val, bool this_also);

	void SetWidgetValue(GObject* obj,unsigned long long value);
	void SetOldValueLabel(GObject* obj,unsigned long long value);
	virtual void UpdateScope();


	std::vector<TKLauS4Config*> config;


	GtkBuilder      *builder; 
	GtkWidget	*window;
	struct{
		int chip;
		int channel;
		int page;
		std::string prefix_param(){
			std::stringstream s;
			switch(page){
				case DIGITAL_PAGE:{return	"digital/";}
				case PGATING_PAGE:{return	"coincidence/";}
				case BIAS_PAGE:{return		"bias/";}
				case CHANNEL_PAGE:{std::stringstream s; s<<"channel"<<channel<<"/";return s.str();}
				//case TESTCHANNEL_PAGE:{return	"adctc/";}
			};
			return "";
		};
		std::string prefix_widget(const char* type_prefix){
			std::stringstream s;
			switch(page){
				case DIGITAL_PAGE:{return	std::string(type_prefix)+std::string("digital/");}
				case PGATING_PAGE:{return	std::string(type_prefix)+std::string("coincidence/");}
				case BIAS_PAGE:{return		std::string(type_prefix)+std::string("bias/");}
				case CHANNEL_PAGE:{return	std::string(type_prefix)+std::string("channel/");}
				//case TESTCHANNEL_PAGE:{return	std::string(type_prefix)+std::string("adctc/");}
			return s.str();
			};
		};
		std::string widget_name(const char* type_prefix,const char* parname,int bit=-1){
			std::string s=parname;
			s.erase(0,prefix_param().length());
			s.insert(0,prefix_widget(type_prefix));
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
                        std::cout<<s<<std::endl;
			//check if change happened in current scope, otherwise ignore
			if(prefix_widget(type_prefix).compare(\
				0,prefix_widget(type_prefix).length(),s,\
				0,prefix_widget(type_prefix).length()\
			)!=0){
				printf("Widget: %s : ignored in scope of %s\n",widget_name,prefix_widget(type_prefix).c_str());
				return "";
			}

			s.erase(0,prefix_widget(type_prefix).length());
			s.insert(0,prefix_param());

			if(s[s.length()-1]==']'){ //is a bitwise widget, extract bit number
                                
				sscanf(&(s[s.find_last_of('[')]),"[%d]",&bit);
				s.erase(s.find_last_of('['));
			}else bit=-1;

			//printf("Widget: %s --> %s bit %d\n",widget_name,s.c_str(),bit);
			return s;
		}
	} scope;
	bool block_val_changed;
};




#endif // STIC3GUI_H__
