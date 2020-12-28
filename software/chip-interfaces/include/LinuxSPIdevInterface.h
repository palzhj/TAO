/*
 *  Interface for configuration, using the linux spi-dev driver.
 *  this interface is predominantly implemented for embedded board such as the rpi
 *  talking to the spi directly, only the 'c' command is implemented
 *
 *  Created on: 4.2016
 *      Author: K Briggl (kbriggl@kip.uni-heidelberg.de)
 */
#ifndef UDPINTERFACE_H_
#define UDPINTERFACE_H_

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

#include "VirtualInterface.h"


class TLinuxSPIdevInterface: public TVirtualInterface
{

public:
    TLinuxSPIdevInterface(const char* devname="/dev/spidev0.0",uint32_t i2c_speed = 500000);
    ~TLinuxSPIdevInterface();



    static TVirtualInterface* Create(const char* devname="/dev/spidev0.0",uint32_t spi_speed = 500000){
	return new TLinuxSPIdevInterface(devname,spi_speed);
    };

private:
    virtual int read_Reply(unsigned int ifaceHandID,int reply_len, char* reply, TVirtualConfig* sender=NULL);
    virtual int send_cmd(const char cmd, unsigned int ifaceHandID, int len, char* data, TVirtualConfig* sender=NULL);

    uint32_t spi_mode;
    uint32_t spi_speed;
    uint16_t spi_delay;

    char* spi_rx_data;
};


#endif /* STIC2INTERFACE_H_ */
