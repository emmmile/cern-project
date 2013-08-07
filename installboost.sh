#!/bin/bash

INSTALL_DIR=boost32

# Get the latest version of the boost libraries
#svn co http://svn.boost.org/svn/boost/trunk boost-trunk
# or
#wget http://sourceforge.net/projects/boost/files/boost/1.46.1/boost_1_46_1.tar.bz2 
wget http://freefr.dl.sourceforge.net/project/boost/boost/1.54.0/boost_1_54_0.tar.gz
tar -zxvf boost_1_54_0.tar.gz
mv boost_1_54_0 boost-trunk

rm -rf $INSTALL_DIR
mkdir $INSTALL_DIR

cd boost-trunk
./bootstrap.sh --with-libraries=program_options,system,serialization --prefix=../$INSTALL_DIR
./bjam -j 4 install

cd ..
rm -rf boost-trunk

echo "Boost libraries successfully installed in $INSTALL_DIR."
