targets= chip-interfaces chip-interfaces-remote config-k4 config-ib config-common daq-common daq-i2c daq-host

all: configure build link


configure:
	@for dir in $(targets) ; do make -C $$dir -f Makefile.configure Makefile.inc ; done

build:
	@for dir in $(targets) ; do make -C $$dir -f Makefile obj ; done

link:
	@for dir in $(targets) ; do make -C $$dir -f Makefile  ; done

clean:
	for dir in $(targets) ; do make -C $$dir -f Makefile.configure clean ; done


