TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += . ./boost/include ./mysql/include
QMAKE_CXXFLAGS += "-std=c++11"
CXXFLAGS="-std=c++0x"

SOURCES += \
    fast/histogram.cpp \
    sqlexample.cpp \
    main.cpp \
    fast/reader/RawReader.cc \
    fast/reader/Reader.cc \
    fast/testpeakanalyzer.cpp \
    fast/reader/Reader.cc \
    gzstream/gzstream.C \
    fast/completereader/unpackx.cpp \
    fast/completereader/unpack.cpp \
    fast/completereader/header.cpp \
    fast/completereader/fio.cpp \
    fast/completereader/main.cpp \
    fast/completereader/misc.cpp

OTHER_FILES += \
    SConstruct \
    fast/sample data/SCIN4_r15185_segm0_ev44_s1.dat \
    fast/sample data/PBWO1_r15229_segm0_ev7_s1.dat \
    fast/sample data/PBWO1_r15229_segm0_ev5_s1.dat \
    fast/sample data/FIMG4_r14325_segm0_ev3_s1.dat \
    fast/sample data/FIMG4_r14325_segm0_ev2_s1.dat \
    fast/sample data/FIMG4_r14325_segm0_ev1_s1.dat \
    CMakeLists.txt \
    createdb.py

HEADERS += \
    fast/peakwindow.hpp \
    fast/reader/RawReader.hh \
    fast/fastreader.hpp \
    fast/peakanalyzer.hpp \
    slow/slowreader.hpp \
    dateutils.hpp \
    gui.hpp \
    Linkdef.h \
    fast/castor.hpp \
    gzstream/gzstream.h \
    correlation.hpp \
    fast/completereader/header.h \
    fast/completereader/DaqType.h \
    fast/completereader/fio.h \
    fast/completereader/unpackx.h \
    fast/completereader/misc.h \
    fast/completereader/DaqDefs.h

