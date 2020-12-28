// Generate the repeated coincidence settings
// Copy and Paste the generated files into the config_window.xml
// do:
//      g++ -o genCoincidence genCoincidence.cpp; ./genCoincidence > coincidence.xml


#include <iostream>

using namespace std;

int main(){
    int Group = 4;
    int Channel = 9;
    int Validation = 9;

    for(int iGroup=Group; iGroup>0; iGroup--){

        for(int iChannel=Channel;iChannel>0;iChannel--){
            //int CH = iGroup*12 -12 + iChannel -1;
            int CH = iGroup*Channel -Channel + iChannel -1;
            // GtkLabel 
            cout<<"<child>"<<endl; 
            cout<<"\t<object class=\"GtkLabel\" id=\"lbl_L0val_ch"<<CH<<"\">"<<endl; // id
            cout<<"\t\t<property name=\"visible\">True</property>"<<endl;
            cout<<"\t\t<property name=\"can_focus\">False</property>"<<endl;
            cout<<"\t\t<property name=\"label\" translatable=\"yes\">CH"<<CH<<"</property>"<<endl;
            cout<<"\t</object>"<<endl;
            cout<<"\t<packing>"<<endl;
            cout<<"\t\t<property name=\"left_attach\">0</property>"<<endl;
            cout<<"\t\t<property name=\"top_attach\">"<<Channel*Group-CH-1<<"</property>"<<endl;
            cout<<"\t</packing>"<<endl;
            cout<<"</child>"<<endl;

            // Validation
            for(int iValid=Validation;iValid>=0;iValid--){
                // GtkCheckButton
                cout<<"<child>"<<endl; 
                cout<<"\t<object class=\"GtkCheckButton\" id=\"val:coincidence/channel"<<CH<<"/coincidence_mask["<<iValid<<"]\">"<<endl; // id
                if(iValid==Validation){ cout<<"\t\t<property name=\"label\" translatable=\"yes\">E</property>"<<endl; }             // label for external
                else cout<<"\t\t<property name=\"label\" translatable=\"yes\">"<<(iGroup-1)*Channel+iValid<<"</property>"<<endl;     // label for channel
                cout<<"\t\t<property name=\"visible\">True</property>"<<endl;
                cout<<"\t\t<property name=\"can_focus\">True</property>"<<endl;
                cout<<"\t\t<property name=\"receives_default\">False</property>"<<endl;
                cout<<"\t\t<property name=\"draw_indicator\">True</property>"<<endl;
                cout<<"\t\t<signal name=\"toggled\" handler=\"on_val_changed\" swapped=\"no\"/>"<<endl;

                cout<<"\t</object>"<<endl;
                cout<<"\t<packing>"<<endl;
                cout<<"\t\t<property name=\"left_attach\">"<<2*(Validation-iValid)+1<<"</property>"<<endl;
                cout<<"\t\t<property name=\"top_attach\">"<<Channel*Group-1-CH<<"</property>"<<endl;
                cout<<"\t</packing>"<<endl;
                cout<<"</child>"<<endl;

                // GtkLabel to store the old value
                cout<<"<child>"<<endl; 
                cout<<"\t<object class=\"GtkLabel\" id=\"old:coincidence/channel"<<CH<<"/coincidence_mask["<<iValid<<"]\">"<<endl; // id
                cout<<"\t\t<property name=\"visible\">True</property>"<<endl;
                cout<<"\t\t<property name=\"can_focus\">False</property>"<<endl;
                cout<<"\t\t<property name=\"label\" translatable=\"yes\">(?)</property>"<<endl;
                cout<<"\t</object>"<<endl;
                cout<<"\t<packing>"<<endl;
                cout<<"\t\t<property name=\"left_attach\">"<<2*(Validation-iValid)+2<<"</property>"<<endl;
                cout<<"\t\t<property name=\"top_attach\">"<<Channel*Group-1-CH<<"</property>"<<endl;
                cout<<"\t</packing>"<<endl;
                cout<<"</child>"<<endl;
            }

        }

    }
}
