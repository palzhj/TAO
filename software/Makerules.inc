CC=g++
C=gcc


CFLAGS+=  -Iinclude -I.

LDFLAGS+=-Wl,--no-as-needed `root-config --glibs`
LIBS+=
SOFLAGS+=
generated+=Makefile.inc

EXT_SOURCES=$(substr .o,.cpp, foo.o)

info:
	@echo "---Compile info---"
	@echo "Dictionary headers: $(dict_headers)"
	@echo "Generated files:    $(generated)"
	@echo "Build Objects:      $(objects)"
	@echo "Dictionary name:	   $(dictname)"
	@echo "EXT MODULES:	   $(EXT_MODULES)"
	#@echo "EXT OBJECTS:	   $(EXT_OBJECTS)"
	#@echo "EXT SOURCES:	   $(EXT_OBJECTS:.o=.cpp)"
	@echo "------------------"


clear:	clean
clean:
	@echo "Cleaning Up Generated"
	@rm -f $(generated)
	@echo "Cleaning Up *.o"
	@rm -f src/*.o
	@echo "Cleaning Up Binaries"
	@rm -f $(binaries)
	@echo "Cleaning Up Libraries"
	@rm -f $(libraries)


# ROOT DICTIONARY TARGETS
ifneq ($(strip $(dict_headers)),)
#we have to give each dictionary a different name, because rootcint will call some created classes as the filename!!
dictname=$(shell basename $(CURDIR))Dict
objects+=src/$(dictname).o
generated+= include/$(dictname).h src/$(dictname).cpp

include/%Dict.h:src/%Dict.cpp
	
 
src/%Dict.cpp: $(dict_headers)
	@echo "Generating Dictionary $@..."
	@rootcint -f $@ -c $(CFLAGS) $(INCLUDE) -p $^
	#@mv -f src/`basename $@ .cpp`.h include/
endif

#GLOBAL TARGETS
obj:$(objects)

bin:$(binaries) $(gtk_binaries)
slibs:$(libraries)

gen:$(generated)


#MAKEFILE BUILDING
Makefile.inc: Makefile.configure
	@echo Generating Makefile.inc 
	@echo INCLUDE+= -I`pwd`/include/ > Makefile.inc
	@for f in $(objects) ; do echo EXT_OBJECTS+= `pwd`/$$f >> Makefile.inc ; done
	@for f in $(gtk_objects) ; do echo EXT_GTK_OBJECTS+= `pwd`/$$f >> Makefile.inc ; done
	@echo EXT_MODULES+=`pwd` >> Makefile.inc
	@echo LIBS+=$(LIBS) >> Makefile.inc


%/Makefile.inc:%
	@if [ ! -e $@ ] ; then make -C $< -f Makefile.configure Makefile.inc ;fi



#GENERAL C targets

%.o:%.cpp
	@echo "(CC) $<"
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ -g -c $<

%.o:%.c
	@echo "(C) $<"
	@$(C) $(CFLAGS) $(INCLUDE) -o $@ -g -c $<

$(libraries): %.so: $(objects) $(EXT_OBJECTS)
	@echo "Building Library $@"
	@$(CC) $^ $(LIBS) $(LDFLAGS) -shared -o $@


$(binaries): %:src/%.o $(objects) $(private_objects) $(EXT_OBJECTS)
	@echo "Building Binary $@"
	@$(CC) $^ $(LIBS) $(LDFLAGS) -o $@

$(gtk_binaries): %:src/%.o $(objects) $(gtk_objects) $(gtk_private_o) $(EXT_OBJECTS) $(EXT_GTK_OBJECTS)
	@echo "Building Binary $@ with GTK3 libs"
	@$(CC) -g $(LDFLAGS) -Wl,--no-as-needed -export-dynamic $^ $(LIBS) `pkg-config --libs gtk+-3.0` -o $@


#EXTERNAL OBJECTS, target for $(EXT_OBJECTS)  -> build recursively
$(EXT_OBJECTS): %.o: %.cpp
	@echo "(MAKE) Recursive build of Objects"
	@make -C $(subst src/,,$(dir $(@))) -f Makefile obj 

$(EXT_GUI_OBJECTS): %.o: %.cpp
	@echo "(MAKE) Recursive build of gtk objects"
	@make -C $(subst src/,,$(dir $(@))) -f Makefile obj 

$(EXT_OBJECTS:.o=.cpp): 
	@echo "(MAKE) Recursive generation of source files"
	@make -C $(subst src/,,$(dir $(@))) -f Makefile.configure gen

 
