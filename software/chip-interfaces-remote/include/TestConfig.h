#include "VirtualConfig.h"
#include "VirtualInterface.h"

class TestConfig: public TVirtualConfig
{
private:
static parameter const par_descriptor[];

public:
    TestConfig(TVirtualInterface* Interface)
    :TVirtualConfig(Interface,par_descriptor,2,2)
{
    bitpattern_read =  new char[2];
    bitpattern_write = new char[2];
};
    ~TestConfig(){
    delete [] bitpattern_read;
    delete [] bitpattern_write;
};
};
TVirtualConfig::parameter const TestConfig::par_descriptor[]={
	TVirtualConfig::parameter("O_DAC0",	0,0),\
	TVirtualConfig::parameter("O_DAC1",	8,0),\
	TVirtualConfig::parameter("",          16,0)
};



