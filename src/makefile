.SUFFIXES: .cpp

#for sgi   -- comment out the lines below to use on HP
#CC=CC -g0 -o32
#CC=gcc

# Compiler options
OPTS=-g
OPTS=-O0
#OPTS=-O2

UNAME = $(shell uname)

ifeq ($(UNAME), Linux)
CXX       = g++
CPPFLAGS += $(OPTS) -std=c++11 -Wall -pedantic
endif

#######################################

CPPFLAGS += -I./ -I./include

LIBGLUI = -L./lib -lglui
LIBGL   = -lGLU -lGL
LIBS    = -lXmu -lXext -lX11 -lXi -lm -lglut -lrfftw -lfftw -lm

##################
# From smoke build
INCLUDEDIRS = -I./fftw-2.1.5/include/
LIBDIRS     = -L./fftw-2.1.5/lib

# One of the following options only...

# (1) OpenGLUT
# LIBGLUT   = -L/usr/X11R6/lib -lopenglut
# CPPFLAGS += -I/usr/X11R6/include -DGLUI_OPENGLUT

# (2) FreeGLUT
# LIBGLUT   = -L/usr/X11R6/lib -lfreeglut
# CPPFLAGS += -I/usr/X11R6/include -DGLUI_FREEGLUT

# (3) GLUT
LIBGLUT   = -L/usr/X11R6/lib -lglut
CPPFLAGS += -I/usr/X11R6/include

#######################################

GLUI_OBJS = lib/glui_add_controls.o lib/glui_string.o lib/glui.o lib/glui_bitmap_img_data.o lib/glui_bitmaps.o lib/glui_button.o lib/glui_edittext.o lib/glui_commandline.o lib/glui_checkbox.o lib/glui_node.o lib/glui_radio.o lib/glui_statictext.o lib/glui_panel.o lib/glui_separator.o lib/glui_spinner.o lib/glui_control.o lib/glui_column.o lib/glui_translation.o lib/glui_rotation.o lib/glui_mouse_iaction.o lib/glui_listbox.o lib/glui_rollout.o lib/glui_window.o lib/arcball.o lib/algebra3.o lib/quaternion.o lib/viewmodel.o lib/glui_treepanel.o lib/glui_tree.o lib/glui_textbox.o lib/glui_scrollbar.o lib/glui_list.o lib/glui_filebrowser.o

GLUI_LIB = lib/libglui.a

GLUI_EXAMPLES = bin/main

CLASSES = code/Color.cpp code/ColorMap.cpp code/Glyphs.cpp code/Isoline.cpp code/IsolineManager.cpp code/Globals.cpp code/Slices.cpp

GLUI_TOOLS = bin/ppm2array

.PHONY: all setup examples tools clean depend dist

#all: setup $(GLUI_LIB) examples
all: examples

setup:
	mkdir -p bin
	mkdir -p lib

examples: $(GLUI_EXAMPLES)

tools: $(GLUI_TOOLS)

bin/ppm2array: tools/ppm2array.cpp tools/ppm.cpp
	$(CXX) $(CPPFLAGS) -o $@ $^

bin/%: code/%.cpp $(GLUI_LIB)
	clear
	$(CXX) -g $(CPPFLAGS) -o $@ $< $(CLASSES) $(LIBGLUI) $(LIBGLUT) $(LIBGL) $(LIBS)

$(GLUI_LIB): $(GLUI_OBJS)
	ar -r $(GLUI_LIB) $(GLUI_OBJS)

.cpp.o:
	$(CXX) $(CPPFLAGS) -c $<

.c.o:
	$(CXX) $(CPPFLAGS) -c $<

docs:
	doxygen doc/doxygen.cfg

clean:
	rm -f *.o $(GLUI_LIB) $(GLUI_EXAMPLES) $(GLUI_TOOLS)
	rm -fr doc/html

depend:
	makedepend -Y./include `find -name "*.cpp"` `find -name "*.c"`

DIST = glui-2.3.0

dist: clean
	mkdir -p $(DIST)
	cp --parents \
		`find -type f -name "*.cpp"` \
		`find -type f -name "*.c"` \
		`find -type f -name "*.h"` \
		`find -type f -name "*.dev"` \
		`find -type f -name "*.dsp"` \
		`find -type f -name "*.dsw"` \
		`find -type f -name "*.vcproj"` \
		`find -type f -name "*.sln"` \
		`find -type f -name "*.txt"` \
		makefile \
		$(DIST)
	tar cv $(DIST) | gzip -9 - > $(DIST).tgz
	rm -Rf $(DIST)


