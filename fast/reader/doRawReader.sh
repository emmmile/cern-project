#!/bin/bash

INC='-I/usr/include/shift -I/cern/pro/include/cfortran  -I/usr/local/include -I/usr/include'
LIB='-L/cern/pro/lib -lmathlib -lpacklib -lkernlib  -L/usr/lib -lshift -lnsl -lcrypt -ldl -lm -L/afs/cern.ch/user/e/emendoza/dataViewers/SignalAnalyzer/lx5libs -lg2c -llapack'
OPT='-m32 -O0 -fPIC'

gcc ${OPT} -c RawReader.cc


g++ ${OPT}  -c main.cc $INC

OBJ='RawReader.o main.o'

g++ ${OPT} ${OBJ} $INC `root-config --glibs` $LIB -o RawReader.exe 

rm ${OBJ}


