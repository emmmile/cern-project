#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iterator>
#include <iomanip>      // std::setprecision
#include "window.hpp"
#include "peakanalyzer.hpp"
using namespace std;


//for i in sample\ data/*; do ./main "$i"; done

int main( int argc, char** argv ) {
        if ( argc < 2 ) { cerr << "Give me a file." << endl; exit( 1 ); }
        fstream sample( argv[1] );

        double time, value;
        peakanalyzer p( "sample" );
        while ( sample >> time >> value ) {
                p.push( time, value );
        }

        //p.printstatus();

        return 0;
}

