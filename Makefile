#############################################################################
# Use make variable_name=' options ' to override the variables or make -e to
# override the file variables with the environment variables
#	make CXXFLAGS='-g'
#	make prefix='/usr'
#	make CXX=g++
# External environment variable: CFISIO, ROOTSYS, CTARTA, ICEDIR
# Instructions:
# - modify the section 1)
# - in section 10), modify the following action:
#	* all: and or remove exe and lib prerequisite
#	* lib: and or remove staticlib and dynamiclib prerequisite
#	* clean: add or remove the files and directories that should be cleaned
#	* install: add or remove the files and directories that should be installed
#	* uninstall: add or remove the files and directories that should be uninstalled
#############################################################################

PROJECT= libagilebz
SHELL = /bin/sh

####### 0) PREREQUISITES
ifndef AGEXTSPOTV2
$(error AGEXTSPOTV2 is not set. Please run the following command: export AGEXTSPOTV2=<path to AG_extspot-v2 folder>)
endif


####### 1) Project names and system

SYSTEM= $(shell gcc -dumpmachine)

LINKERENV= cfitsio, agile

# Applications
LIB_NAME = libextspot


####### 2) Directories for the installation
# The directory to install the libraries in.
libdir=${CURDIR}/lib
#installed_libdir=$(AGILE)/lib

####### 3) Directories for the compiler
OBJECTS_DIR = obj
SOURCE_DIR = src
INCLUDE_DIR = include
LIB_DESTDIR = lib
AGEXTSPOTV2_LIB_DESTDIR = $(AGEXTSPOTV2)/libs
AGEXTSPOTV2_INCLUDE_DIR = $(AGEXTSPOTV2)/include


####### 4) Compiler, tools and options
CXX = g++
CXXFLAGS = -g -std=c++11 -pipe -I $(INCLUDE_DIR)


ifneq (, $(findstring agile, $(LINKERENV)))
    ifeq (, $(findstring -I $(AGILE)/include, $(CXXFLAGS)))
        CXXFLAGS += -I $(AGILE)/include
    endif
    #LIBS += -L$(AGILE)/lib -lagilesci
endif
ifneq (, $(findstring opencv, $(LINKERENV)))
    ifeq (, $(findstring -I $(OPENCV)/include, $(CXXFLAGS)))
        CXXFLAGS += -I $(OPENCV)/include
	CXXFLAGS += -std=c++11
    endif
    #LIBS += -L$(OPENCV)/lib -lopencv_core -lopencv_highgui -lopencv_imgproc
endif
ifneq (, $(findstring root, $(LINKERENV)))
    CXXFLAGS += -W -fPIC -D_REENTRANT $(shell root-config --cflags)
    #LIBS += $(shell root-config --glibs) -lMinuit
endif
ifneq (, $(findstring cfitsio, $(LINKERENV)))
    CXXFLAGS += -I$(CFITSIO)/include
    #LIBS += -L$(CFITSIO)/lib -lcfitsio
endif


#LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc


LINK     = $(CXX)
#for link
LFLAGS = -shared -Wl,-soname,$(TARGET1) -Wl,-rpath,$(DESTDIR)
AR       = ar cqs
TAR      = tar -cf
GZIP     = gzip -9f
COPY     = cp -f -r
COPY_FILE= $(COPY) -p
COPY_DIR = $(COPY) -pR
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rm -rf
MOVE     = mv -f
CHK_DIR_EXISTS= test -d
MKDIR    = mkdir -p

####### 5) VPATH

VPATH=$(SOURCE_DIR):$(INCLUDE_DIR):
vpath %.o $(OBJECTS_DIR)

####### 6) Files of the project

INCLUDE=$(foreach dir,$(INCLUDE_DIR), $(wildcard $(dir)/*.h))
SOURCE=$(foreach dir,$(SOURCE_DIR), $(wildcard $(dir)/*.cpp))
#Objects to build
OBJECTS=$(addsuffix .o, $(basename $(notdir $(SOURCE))))


####### 7) Only for library generation
TARGETA = $(LIB_NAME).a


####### 8) Preliminar operations

####### 9) Pattern rules
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $(OBJECTS_DIR)/$@



$(info $$OBJECTS is [${OBJECTS}])

####### 10) Build rules

#all: compile the entire program.
all: staticlib

staticlib: makelibdir makeobjdir $(OBJECTS)

	# generate of static library
	test -d $(LIB_DESTDIR) || mkdir -p $(LIB_DESTDIR)
	$(DEL_FILE) $(LIB_DESTDIR)/$(TARGETA)
	$(AR) $(LIB_DESTDIR)/$(TARGETA) $(OBJECTS_DIR)/*.o

	# move library to AG_extspot-v2/libs
	test -d $(AGEXTSPOTV2_LIB_DESTDIR) || mkdir -p $(AGEXTSPOTV2_LIB_DESTDIR)
	$(COPY_FILE) $(LIB_DESTDIR)/$(TARGETA) $(AGEXTSPOTV2_LIB_DESTDIR)

	# move headers to AG_extspot-v2/include
	$(DEL_FILE) $(AGEXTSPOTV2_INCLUDE_DIR)/*.h
	$(COPY_FILE) $(INCLUDE_DIR)/* $(AGEXTSPOTV2_INCLUDE_DIR)


makeobjdir:
	test -d $(OBJECTS_DIR) || mkdir -p $(OBJECTS_DIR)

makelibdir:
	test -d $(LIB_DESTDIR) || mkdir -p $(LIB_DESTDIR)

#clean: delete all files from the current directory that are normally created by building the program.
clean:
	$(DEL_FILE) $(OBJECTS_DIR)/*.o
	$(DEL_FILE) $(LIB_DESTDIR)/*.a
	test $(OBJECTS_DIR) = . || $(DEL_DIR) $(OBJECTS_DIR)
	test $(LIB_DESTDIR) = . || $(DEL_DIR) $(LIB_DESTDIR)

#install: to copy the library
#install: all
#	$(shell echo $(prefix) > prefix)
#	test -d $(installed_libdir) || mkdir -p $(installed_libdir)
#	$(COPY_FILE) $(LIB_DESTDIR)/* $(installed_libdir)

#uninstall:
	#For library uninstall
#	$(DEL_FILE) $(libdir)/$(TARGETA)
#	$(DEL_FILE) $(addprefix $(includedir)/, $(notdir $(INCLUDE)))
