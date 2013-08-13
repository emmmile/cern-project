#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iterator>
#include <iomanip>      // std::setprecision
#include "peakwindow.hpp"
#include "peakanalyzer.hpp"
#include <boost/program_options.hpp>
#include "gzstream/gzstream.h"
using namespace std;
using namespace boost::program_options;


//for i in ../project/sample\ data/*.dat; do ./testpeakanalyzer -i "$i"; done


int main( int argc, char** argv ) {
        string input;
        bool gzip = false;

        // Declare the supported program options
        options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "produce this help message")
                ("input-data,i", value<string>(&input), "set the input data file, normally a .dat file corresponding to a single move, not to a full event")
                ("compressed,c", "use compressed input (default is uncompressed)")
        ;

        variables_map vm;

        try {
                store(parse_command_line(argc, argv, desc), vm);

                if (vm.count("help")) {
                        cout << desc << "\n";
                        exit( 1 );
                }


                notify(vm);
        } catch ( error& e ) {
                cerr << desc << endl;
                cerr << e.what() << endl;
                exit( 1 );
        }


        if (!vm.count("input-data")) {
                cerr << desc << endl;
                cerr << "option --input-data is required.\n";
                exit( 1 );
        }

        if (vm.count("compressed"))
                gzip = true;



        istream* sample;
        if ( !gzip )
                sample = new ifstream( input );
        else    sample = new igzstream( input );

        cout << "File: " << input << endl;

        double time, value;
        peakanalyzer<double> p( input );
        while ( (*sample) >> time >> value ) {
                p.push( time, value );
        }

        delete sample;

        //p.printstatus();

        return 0;
}

