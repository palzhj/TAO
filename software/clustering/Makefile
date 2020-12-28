all: all_

objects+= src/ClusterTypes.o src/Clustering.o
dict_headers+= include/ClusterTypes.h include/LinkDef.h
CFLAGS+=-Iinclude `root-config --cflags` -fPIC
INCLUDE+= -I../daq-common/include/
#LDFLAGS+= -L../daq-common/ -lklaus4_storagetypes
include ../daq-common/Makefile.inc
include ../daq-host/Makefile.inc
objects+=
libraries+= libklaus4_clustering.so
binaries+= aqu_reorder  ClusterTree

include ../Makerules.inc
all_: obj bin slibs
