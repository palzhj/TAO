/*
 *  Interface for configuration, using the linux spi-dev driver.
 *  this interface is predominantly implemented for embedded board such as the rpi
 *  talking to the spi directly, only the 'c' command is implemented
 *
 *  Created on: 4.2016
 *      Author: K Briggl (kbriggl@kip.uni-heidelberg.de)
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <errno.h>

#include "LinuxSPIdevInterface.h"
#include "VirtualConfig.h"

//#define DEBUG

void DbgPrintPatternHex(const char* prefix, char* data,int len,bool inv=false){
	printf("%s",prefix);
	for (int i=len-1;i>=0;i--)
		printf(" %2.2x ",0xff&((const char*)(data))[inv?len-i-1:i]);
	printf("\n");
}
void DbgPrintPatternBin(const char* prefix, char* data,int len,bool inv=false){
	printf("%s",prefix);
	for (int i=len-1;i>=0;i--){
		for (int j=7;j>=0;j--)
			printf("%x",(data[inv?len-i-1:i]&(0xff&(1<<j)))!=0?1:0 );
		printf(":");
	}
	printf("\n");
}


TLinuxSPIdevInterface::TLinuxSPIdevInterface(const char* devname,uint32_t spi_speed):
TVirtualInterface(1),spi_speed(spi_speed),spi_rx_data(NULL)
{
	int ret=0;
	//uint8_t *tx;
	//uint8_t *rx;
	//int size;
	uint32_t mode=0;
//	mode |= SPI_LOOP;
//	mode |= SPI_CPHA;
//	mode |= SPI_CPOL;
//	mode |= SPI_LSB_FIRST;
//	mode |= SPI_CS_HIGH;
//	mode |= SPI_3WIRE;
//	mode |= SPI_NO_CS;
//	mode |= SPI_READY;
//	mode |= SPI_TX_DUAL;
//	mode |= SPI_TX_QUAD;
/*
	if (mode & SPI_LOOP) {
		if (mode & SPI_TX_DUAL)
			mode |= SPI_RX_DUAL;
		if (mode & SPI_TX_QUAD)
			mode |= SPI_RX_QUAD;
	}
*/

	fd = open(devname, O_RDWR);
	if (fd<0) {
		perror("Interface: failed to open spi device");
		return;
	}

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1){
		perror("can't set spi mode");
		return;
	}
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1){
		perror("can't get spi mode");
		return;
	}

	/*
	 * bits per word
	 */
	static uint8_t bits = 8;
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1){
		perror("can't set bits per word");
		return;
	}

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1){
		perror("can't get bits per word");
		return;
	}

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
	if (ret == -1){
		perror("can't set max speed hz");
		return;
	}

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
	if (ret == -1){
		perror("can't get max speed hz");
		return;
	}
#ifdef DEBUG
	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", spi_speed, spi_speed/1000);
#endif
	isReady=1;


}

TLinuxSPIdevInterface::~TLinuxSPIdevInterface(){
	close(fd);
};


//Send configuration pattern. Support only for 'c' command. HandleID is ignored.
//data readback does not work at the moment.
int TLinuxSPIdevInterface::send_cmd(const char cmd, unsigned int ifaceHandID,  int len, char* data, TVirtualConfig* sender){
	int ret;
	if(cmd!='c'){
		fprintf(stderr,"TLinuxSPIdevInterface::send_cmd(): Command '%c' (0x%x) is not implemented.\n",cmd,cmd&0xff);
		return -2;
	}
	if(sender==NULL){
		fprintf(stderr,"TLinuxSPIdevInterface::send_cmd(): Sender is NULL, this parameter is needed for that interface");
		return -2;
	}
	char* spi_tx_data=(char*)malloc(sizeof(char)*sender->GetPatternByteLength());
	char* local_rx_data=(char*)malloc(sizeof(char)*sender->GetPatternByteLength());

	//turn around pattern, byte wise
	for (int i=0;i<sender->GetPatternByteLength();i++){
		spi_tx_data[i]=data[sender->GetPatternByteLength()-i-1];
	}

	struct spi_ioc_transfer tr = {
		tx_buf : (unsigned long) spi_tx_data,
		rx_buf : (unsigned long) local_rx_data,
		len : sender->GetPatternByteLength(),
		speed_hz : spi_speed,
		delay_usecs : spi_delay,
		bits_per_word : 8,
		cs_change : 0
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1){
		perror("Error writing to SPI device.\n");
		return -2;
	}
	//turn around pattern, byte wise
	char byte;
	for (int i=0;i<sender->GetPatternByteLength()/2;i++){
		byte=local_rx_data[i];
		local_rx_data[i]=local_rx_data[sender->GetPatternByteLength()-i-1];
		local_rx_data[sender->GetPatternByteLength()-i-1]=byte;
	}
	//shift back the returned pattern
	spi_rx_data=(char*)realloc(spi_rx_data,sizeof(char)*sender->GetPatternByteLength());
	bzero(spi_rx_data,sender->GetPatternByteLength());
	char shift=sender->GetPatternByteLength()*8-sender->GetPatternBitLength();
	for (int i=0;i<sender->GetPatternByteLength()*8;i++){
		bool bit=local_rx_data[i/8]&(1<<(i%8));
		spi_rx_data[((i-shift)/8)]|= (bit<<((i-shift)%8));
	}

#ifdef DEBUG
	printf("SPI length requested: %d\n",sender->GetPatternByteLength());
	DbgPrintPatternHex("TX:", data,sender->GetPatternByteLength());
	//DbgPrintPatternHex("TL:", spi_tx_data,sender->GetPatternByteLength());
	//DbgPrintPatternHex("RL:", local_rx_data,sender->GetPatternByteLength());
	DbgPrintPatternHex("RX:", spi_rx_data,sender->GetPatternByteLength());
	//DbgPrintPatternBin("TX:", data,sender->GetPatternByteLength());
	//DbgPrintPatternBin("TL:", spi_tx_data,sender->GetPatternByteLength());
	//DbgPrintPatternBin("RL:", local_rx_data,sender->GetPatternByteLength());
	//DbgPrintPatternBin("RX:", spi_rx_data,sender->GetPatternByteLength());
#endif


	free(spi_tx_data);
	free(local_rx_data);
	return 0;
}

    /*read data from the interface.
	returns:
		-2 if reading failed,
		-1 if the reply was not for the given Handle
		0  else.
    */
int TLinuxSPIdevInterface::read_Reply(unsigned int ifaceHandID,int reply_len, char* reply, TVirtualConfig* sender){
	if (spi_rx_data!=NULL){
		memcpy(reply,spi_rx_data,reply_len);
		free(spi_rx_data);
		spi_rx_data=NULL;
		return 0;
	}
	return -2;
}



