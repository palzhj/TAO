

#ifndef MQ_MESSAGE_DICT_H__
#define MQ_MESSAGE_DICT_H__

#define MAX_MQ_MSGLEN 1000

struct udp_msg_header{
	unsigned char cmd;
	unsigned char handleID;
	unsigned short len;
};

struct udp_msg_struct{
	unsigned char cmd;
	unsigned char handleID;
	unsigned short len;
	char msg[MAX_MQ_MSGLEN];		
};


struct udp_daq_data_header{
	unsigned char cmd;
	unsigned char handleID;
	unsigned short len;
	unsigned short packetID;
};
struct udp_daq_data_struct{
	unsigned char cmd;
	unsigned char handleID;
	unsigned short len;
	unsigned short packetID;
	char payload[MAX_MQ_MSGLEN-2];
};
#endif
