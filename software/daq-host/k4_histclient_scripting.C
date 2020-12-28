
//before sourcing this script, do: 
// gSystem->Load("HistogramDAQ_client.so");
// or: .L HistogramDAQ_client.so

void k4_histclient_scripting(const char* host="ropi4k1"){
	//daq
	Spy* histDAQ=new Spy(host);

	histDAQ->ResetResults();
	histDAQ->FetchResults();

	printf("The END...\n");
}

