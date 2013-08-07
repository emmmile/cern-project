
####### Compiler, tools and options

CC       = gcc
CXX      = g++
LEX      = flex
YACC     = yacc
LEXFLAGS = 
YACCFLAGS= -d
LINK     = g++
AR       = ar cqs
RANLIB   = 
TAR      = tar -cf
GZIP     = gzip -9f
COPY     = cp -f
COPY_FILE= $(COPY)
COPY_DIR = $(COPY) -r
INSTALL_FILE= $(COPY_FILE)
INSTALL_DIR = $(COPY_DIR)
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rmdir
MOVE     = mv -f
CHK_DIR_EXISTS= test -d
MKDIR    = mkdir -p

#CFLAGS   = -m32 -O3 #-O0 -fPIC
#CXXFLAGS = $(CFLAGS)
#INCPATH  = -I/usr/include/shift -I/cern/pro/include/cfortran -I/usr/local/include -I/usr/include
#LFLAGS   = -m32 #-s -Os -L/cern/pro/lib -L/usr/lib -L/afs/cern.ch/user/e/emendoza/dataViewers/SignalAnalyzer/lx5libs
#LIBS     = -lshift -lnsl -ldl -lm #-lcrypt -llapack -lg2c


CFLAGS   = -g3
CXXFLAGS = $(CFLAGS)
LFLAGS   = -g3
LIBS     = -lshift -lnsl -ldl -lm -lz
INCPATH  =


first: all

####### Build rules

TARGETS = main

all: Makefile $(TARGETS)

yaccclean:
lexclean:
clean:
	-$(DEL_FILE) *.o *~ main

####### Compile

RawReader.o: RawReader.cc
	$(CXX) $(CXXFLAGS) $(INCPATH) -c RawReader.cc -o RawReader.o

main.o: main.cc
	$(CXX) $(CXXFLAGS) $(INCPATH) -c main.cc -o main.o

main: main.o RawReader.o
	$(CXX) $(LFLAGS) $(LIBS) main.o RawReader.o -o main
		
		

