

#include "VirtualConfig.h"
#include "VirtualInterface.h"
#include "VCRemoteClient.h"

#include "TestConfig.h"

#define NCHIPS 1

int main (int argc, char *argv[])
{
try{
	if (argc != 3){
		std::cerr << "Usage: chat_client <host> <port>\n";
		return 1;
	}


	using namespace std;
	unsigned long long value;
	int ret;

	//VCRemoteClient<TUDPInterface>* not_derived_from_VC=new VCRemoteClient<TUDPInterface>(io_service,endpoint_iterator,0); //should fail

	VCRemoteClient<TestConfig>* configuration=new VCRemoteClient<TestConfig>(argv[1],0);
/*	ret=configuration->SetParValue("O_DAC0",1);
	printf("SetParValue(1): %d\n",ret);
	ret=configuration->SetParValue("O_DAC0",10);
	printf("SetParValue(2): %d\n",ret);
*/
	ret=configuration->GetParValueRD("O_DAC0",value);
	printf("GetParValueRD(): %d\n",ret);
	ret=configuration->GetParValueWR("O_DAC0",value);
	printf("GetParValueWR(): %d\n",ret);
	ret=configuration->UpdateConfig();
	printf("UpdateConfig(): %d\n",ret);
	delete configuration;
	return 0;

}
catch (std::exception& e)
{
	std::cerr << "Exception: " << e.what() << "\n";
}

	return 0;

}
