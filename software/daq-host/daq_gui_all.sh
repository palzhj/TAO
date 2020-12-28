#!/bin/bash


ASICS=$(./getASICs ropi4k2 | grep ASIC | cut -d" " -f 2)

for it in $ASICS
do
	./daq_gui ropi4k2 $it &
done

