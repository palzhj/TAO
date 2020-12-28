#!/bin/bash
gpios="4 17 27 22 14 15 18 23 24"


for i in $gpios
do
	echo "Exporting GPIO #$i"
	echo $i >> /sys/class/gpio/export
done
for i in $gpios
do
	echo "Changing permissions for GPIO #$i"
	chmod 777 /sys/class/gpio/gpio$i/value
	chmod 777 /sys/class/gpio/gpio$i/direction
	chmod 777 /sys/class/gpio/gpio$i/*
done
