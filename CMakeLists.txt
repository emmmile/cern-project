cmake_minimum_required (VERSION 2.6)
project (project)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-std=c++0x" )
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -rdynamic")
#set( CMAKE_VERBOSE_MAKEFILE on )

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dict.cxx ${CMAKE_CURRENT_BINARY_DIR}/dict.h
    COMMAND rootcint -f ${CMAKE_CURRENT_BINARY_DIR}/dict.cxx -c ${CMAKE_SOURCE_DIR}/gui.hpp ${CMAKE_SOURCE_DIR}/Linkdef.h
)

add_custom_target(dictionary ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dict.cxx)


SET(BASEPATH "${CMAKE_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${BASEPATH}")
#INCLUDE_DIRECTORIES("${BASEPATH}/boost/include")
#LINK_DIRECTORIES("${BASEPATH}/boost/lib")

#generated with root-config --cflags
INCLUDE_DIRECTORIES("/afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.03/x86_64-slc5-gcc43-opt/root/include")
LINK_DIRECTORIES("/afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.03/x86_64-slc5-gcc43-opt/root/lib")
#LINK_DIRECTORIES("/cern/pro/lib")

INCLUDE_DIRECTORIES("${BASEPATH}/mysql/include")
LINK_DIRECTORIES("${BASEPATH}/mysql/lib")

add_executable(main main.cpp fast/reader/RawReader.cc dict.cxx)
add_dependencies(main dictionary)

TARGET_LINK_LIBRARIES(main boost_program_options shift nsl crypt dl mysqlcppconn pthread)
#generated with root-config --glibs
TARGET_LINK_LIBRARIES(main Gui Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread)




add_executable(histogram fast/histogram.cpp)

TARGET_LINK_LIBRARIES(histogram boost_program_options Gui Core Cint RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread)


add_executable(testpeakanalyzer fast/testpeakanalyzer.cpp)
TARGET_LINK_LIBRARIES(testpeakanalyzer boost_program_options)

add_executable(rawreader fast/reader/RawReader.cc fast/reader/Reader.cc)
TARGET_LINK_LIBRARIES(rawreader shift nsl crypt dl)



#INCLUDE_DIRECTORIES("${BASEPATH}/boost/include")
#LINK_DIRECTORIES("${BASEPATH}/boost/lib")

#add_executable(client examples/client.cpp)
#add_executable(server examples/server.cpp)

#TARGET_LINK_LIBRARIES(client boost_system boost_serialization pthread)
#TARGET_LINK_LIBRARIES(server boost_system boost_serialization pthread)
