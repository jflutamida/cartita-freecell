#Jorgitus 20180713

TARGET := cartita
SOURCEDIRS := .

INCLUDEDIRS := -I/usr/include/gtkmm-3.0 -I/usr/lib/x86_64-linux-gnu/gtkmm-3.0/include 
INCLUDEDIRS += -I/usr/include/gdkmm-3.0 -I/usr/include/atkmm-1.6 
INCLUDEDIRS += -I/usr/lib/x86_64-linux-gnu/gdkmm-3.0/include -I/usr/include/giomm-2.4 
INCLUDEDIRS += -I/usr/lib/x86_64-linux-gnu/giomm-2.4/include -I/usr/include/pangomm-1.4 
INCLUDEDIRS += -I/usr/lib/x86_64-linux-gnu/pangomm-1.4/include -I/usr/include/glibmm-2.4 
INCLUDEDIRS += -I/usr/lib/x86_64-linux-gnu/glibmm-2.4/include -I/usr/include/gtk-3.0 
INCLUDEDIRS += -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 -I/usr/include/dbus-1.0 
INCLUDEDIRS += -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/include/gtk-3.0 
INCLUDEDIRS += -I/usr/include/gio-unix-2.0/ -I/usr/include/cairo -I/usr/include/pango-1.0 
INCLUDEDIRS += -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/cairo 
INCLUDEDIRS += -I/usr/include/cairomm-1.0 -I/usr/lib/x86_64-linux-gnu/cairomm-1.0/include 
INCLUDEDIRS += -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng16 
INCLUDEDIRS += -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/sigc++-2.0 
INCLUDEDIRS += -I/usr/lib/x86_64-linux-gnu/sigc++-2.0/include -I/usr/include/gdk-pixbuf-2.0 
INCLUDEDIRS += -I/usr/include/libpng16 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

LIBDIRS :=
LIBS := -lgtkmm-3.0 -latkmm-1.6 -lgdkmm-3.0 -lgiomm-2.4 -lpangomm-1.4 -lglibmm-2.4 -lgtk-3 -lgdk-3 
LIBS += -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo-gobject -lgio-2.0 -lcairomm-1.0 -lcairo -lsigc-2.0 
LIBS += -lgdk_pixbuf-2.0 -lgobject-2.0 -lglib-2.0

#-----------------------------------------
OBJECTDIR := temp
SOURCES := $(foreach dir,$(SOURCEDIRS),$(wildcard $(dir)/*.cc))
HEADERS := $(foreach dir,$(SOURCEDIRS),$(wildcard $(dir)/*.h))
OBJECTS := $(addprefix $(OBJECTDIR)/,$(notdir $(patsubst %.cc,%.o,$(SOURCES))))
VPATH := $(SOURCEDIRS)
#compiler_flags := -m64 -std=c++11 -fexceptions -Wno-write-strings -mms-bitfields -g
COMPILERFLAGS := -pthread -std=c++1z
LINKERFLAGS := -no-pie

define colorecho
	@tput setaf 2
	@echo $1
	@tput sgr0
endef


$(OBJECTDIR)/%.o : %.cc
	@-mkdir -p temp
	g++ $(COMPILERFLAGS) $(INCLUDEDIRS) -c $< -o $@

$(TARGET) : $(OBJECTS)
	g++ $(LINKERFLAGS) -o $(TARGET) $(OBJECTS) $(LIBDIRS) $(LIBS)
	$(call colorecho,"Created " $(TARGET))
$(OBJECTS) : $(HEADERS)


.PHONY: clean print
print: 
	@echo VPATH = $(VPATH)
	@echo TARGET = $(TARGET)
	@echo SOURCEDIRS = $(SOURCEDIRS)
	@echo SOURCES = $(SOURCES)
	@echo HEADERS = $(HEADERS)
	@echo OBJECTS = $(OBJECTS)

clean:
	@-rm -rf $(OBJECTDIR)
	@-rm -f $(TARGET)

