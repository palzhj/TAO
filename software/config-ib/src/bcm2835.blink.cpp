#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>

#define PIN RPI_V2_GPIO_P1_40

//SIGINT handler
#include <signal.h>
bool run;
void sig(int){
	printf("stopping.\n");
	run=false;
}

int main(int argc, char **argv)
{
	int delay=1;
	if(argc>1){
		delay=atoi(argv[1]);
		if(delay<1) delay=1;
	}

	printf("Will flash LED system with delay ~~ %d ms\n",delay);

	signal(SIGINT,&sig);
	run=true;
	if (!bcm2835_init())
	return 1;
	bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);

	while (run)
	{
		bcm2835_gpio_write(PIN, HIGH);
		bcm2835_gpio_write(PIN, HIGH);
		bcm2835_gpio_write(PIN, HIGH);
		bcm2835_gpio_write(PIN, LOW);

		bcm2835_delay(delay); //1ms
	}
	bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_INPT);
	bcm2835_close();
	printf("stopping.\n");
	return 0;
}

