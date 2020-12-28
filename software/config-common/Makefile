all: all_
include ../chip-interfaces/Makefile.inc
include ../chip-interfaces-remote/Makefile.inc
include ../config-ib/Makefile.inc
include ../config-k4/Makefile.inc
include Makefile.configure
all_: gen obj bin

src/resources.o: src/resources.cpp
	@echo "(CC)(GTK) $<"
	@$(CC) $(CFLAGS) `pkg-config --cflags gtk+-3.0` $(INCLUDE) -o $@ -g -c $<

src/resources.cpp: glade_xml/gresources_bundle.xml include/resources.h glade_xml/config_window.xml
	@echo "Generating $@..."
	@glib-compile-resources --target=$@ --generate-source --c-name config_wnd glade_xml/gresources_bundle.xml

include/resources.h: glade_xml/gresources_bundle.xml
	@echo "Generating $@..."
	@glib-compile-resources --target=$@ --generate-header --c-name config_wnd glade_xml/gresources_bundle.xml

