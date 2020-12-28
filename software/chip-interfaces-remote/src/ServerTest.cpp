
#include <cstdlib>
#include <iostream>
#include "VCRemoteServer.h"
#include "TestConfig.h"

int main(int argc, char* argv[]){
	try{
		if (argc != 2){
			std::cerr << "Usage: VCRemoteServerTest <port>\n";
			return 1;
		}

		VCRemoteServer s1(argv[1]);
		TestConfig* configuration=new TestConfig(NULL);
		s1.AddConfiguration(configuration); //ID=0

		s1.Run();
	}
	catch (std::exception& e){
		std::cerr << "Exception: " << e.what() << "\n";
	}

return 0;
}
