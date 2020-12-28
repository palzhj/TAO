

#ifndef MQ_MESSAGE_DICT_H__
#define MQ_MESSAGE_DICT_H__

#define MAX_MQ_MSGLEN 2000

struct __attribute__((__packed__)) msg_header{
	unsigned char cmd;
	unsigned char handleID;
	unsigned char port;
	unsigned short len;
};

struct __attribute__((__packed__)) msg_struct{
	unsigned char cmd;
	unsigned char handleID;
	unsigned char port;
	unsigned short len;
	char msg[MAX_MQ_MSGLEN];		
};


struct __attribute__((__packed__)) daq_data_header{
	unsigned char cmd;
	unsigned char handleID;
	unsigned char port;
	unsigned short len;
	unsigned short packetID;
};
struct __attribute__((__packed__)) daq_data_struct{
	unsigned char cmd;
	unsigned char handleID;
	unsigned char port;
	unsigned short len;
	unsigned short packetID;
	char payload[MAX_MQ_MSGLEN-2];
};
#endif
