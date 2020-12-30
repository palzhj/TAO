#CC=${CROSS_COMPILE}g++
#C=${CROSS_COMPILE}gcc
CC=g++
C=gcc

#ROOT_CFL=`root-config --cflags`
#ROOT_LFL=`root-config --glibs`


CCFLAGS=-Iinclude -std=c11
CFLAGS= -Iinclude 

#CFLAGS+= -static

LIBS= -lrt
LIBS+= $(shell python-config --libs)
#IMPORT_LIBS=
#LDFLAGS=-Wl,--no-as-needed
#LDFLAGS=-static




driver=src/VirtualConfig.o src/VirtualInterface.o src/PatternMerger.o
driver+=src/SerialInterface.o src/UDPInterface.o src/MQInterface.o src/SerialBridge.o src/LinuxSPIdevInterface.o

objects= $(driver)

binaries=

all: $(objects) Makefile.inc $(binaries)

obj: $(objects)

#include/EventDict.h:	src/EventDict.cpp

#src/EventDict.cpp: 
#	@echo "Generating Dictionary $@..."
#	@cd include ; rootcint -f EventDict.cpp -c EventType.h LinkDef.h ; mv EventDict.cpp ../src/
	
clean:
	@echo "Cleaning Up Binaries"
	@rm -f $(binaries)
	@echo "Removing Makefile.inc"
	@rm -f Makefile.inc
#	@rm src/EventDict.cpp
#	@rm include/EventDict.h
	@echo "Removing object files"
	@rm -f src/*.o


%.o:%.cpp
	@echo "(CC) $<"
	@$(CC) $(CFLAGS) $(GUICFLAGS) $(INCLUDE) -o $@ -g -c $<

%.so:%.o
	echo "Building Library $<"

$(binaries): %:src/%.o $(objects)
	@echo "Building Binary $@"
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@



Makefile.inc: $(objects)
	@echo Creating Makefile.inc 
	@echo INCLUDE+= -I`pwd`/include/ > Makefile.inc
	@(for ofile in ${objects} ; do echo EXT_OBJECTS+= `pwd`/$${ofile} >> Makefile.inc ; done)
	@echo EXT_MODULES+=`pwd` >> Makefile.inc
	@echo LIBS+=$(LIBS) >> Makefile.inc
