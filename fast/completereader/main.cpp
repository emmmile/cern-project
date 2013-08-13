
#include <iostream>
#include "unpackx.h"
using namespace std;

int g_debug = 1;

int main ( ) {
        //UnPackX boh( "/castor/cern.ch/ntof/2012/Ni59/stream0/run15500_0.idx.finished", true );
        UnPackX test( "/castor/cern.ch/ntof/2012/Ni59/stream1/run15500_0_s1.raw.finished", true );

        return 0;
}
