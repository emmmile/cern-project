cmake_minimum_required (VERSION 2.6)
project (project)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-std=c++0x -funroll-loops" )
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -rdynamic")
#set( CMAKE_VERBOSE_MAKEFILE on )

# this executes the rootcint
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dict.cxx ${CMAKE_CURRENT_BINARY_DIR}/dict.h
    COMMAND rootcint -f ${CMAKE_CURRENT_BINARY_DIR}/dict.cxx -c ${CMAKE_SOURCE_DIR}/gui.hpp ${CMAKE_SOURCE_DIR}/Linkdef.h
)

add_custom_target(dictionary ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dict.cxx)


SET(BASEPATH "${CMAKE_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${BASEPATH}")

# Add the root paths, generated with root-config --cflags
INCLUDE_DIRECTORIES("/afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.03/x86_64-slc5-gcc43-opt/root/include")
LINK_DIRECTORIES("/afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.03/x86_64-slc5-gcc43-opt/root/lib")
# Add the MySQL paths
INCLUDE_DIRECTORIES("${BASEPATH}/mysql/include")
LINK_DIRECTORIES("${BASEPATH}/mysql/lib")
# Add the gzstream path (used for compress files)
INCLUDE_DIRECTORIES("${BASEPATH}/gzstream")

# Builds the main program
add_executable(main main.cpp fast/castor.cpp fast/fastreader.cpp fast/peakanalyzer.cpp fast/peakwindow.cpp correlation.cpp
               slow/slowreader.cpp dateutils.cpp gui.cpp fast/reader/RawReader.cc dict.cxx)
add_dependencies(main dictionary)

# Add link flags for boost, MySQL and the RawReader
TARGET_LINK_LIBRARIES(main boost_program_options shift nsl crypt dl mysqlcppconn pthread)
# Add link flags for root, generated with root-config --glibs
TARGET_LINK_LIBRARIES(main Gui Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread)








# Utility programs
add_executable(histogram fast/histogram.cpp gzstream/gzstream.C)
TARGET_LINK_LIBRARIES(histogram z boost_program_options)
TARGET_LINK_LIBRARIES(histogram Gui Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread)

add_executable(testpeakanalyzer fast/testpeakanalyzer.cpp fast/peakanalyzer.cpp fast/peakwindow.cpp gzstream/gzstream.C)
TARGET_LINK_LIBRARIES(testpeakanalyzer boost_program_options z)

add_executable(rawreader fast/reader/RawReader.cc fast/reader/Reader.cc gzstream/gzstream.C)
TARGET_LINK_LIBRARIES(rawreader shift nsl crypt dl z)
