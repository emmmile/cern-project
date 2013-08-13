/*===========================================================================================
compilation:
g++ -O3 -funroll-loops `root-config --cflags` `root-config --glibs` -o example example.cpp
run:
./example
==============================================================================================*/

#include <TRandom3.h>
#include <TH1.h>
#include <TGraph.h>
#include <TMath.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <TFile.h>
#include <string>
#include "gzstream/gzstream.h"
#include <boost/program_options.hpp>
using namespace std;
using namespace boost::program_options;

//for i in sample\ data/*.dat; do ./histogram -i "$i" -o "$i".png; done

int main ( int argc, char** argv ) {
        string input, output = "histogram.png";
        bool gzip = false;

        // Declare the supported program options
        options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "produce this help message")
                ("input-data,i", value<string>(&input), "set the input data file, normally a .dat")
                ("output-image,o", value<string>(&output), "set the output location, default to ./histogram.png")
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
        //cout << "Reading from file: " << argv[1] << endl;

        TCanvas *c = new TCanvas ( "c", "Sample", 1920, 900 );
        c->cd();
        c->SetGridx();
        c->SetGridy();

        vector<double> times;
        vector<double> values;
        double time, value;
        while ( (*sample) >> time >> value ) {
                times.push_back ( time );
                values.push_back ( value );
        }

        TGraph* gr = new TGraph ( times.size(), &times[0], &values[0] );
        gr->SetTitle ( input.c_str() );
        gr->GetXaxis()->SetTitle ( "time [ns]" );
        gr->GetYaxis()->SetTitle ( "value" );
        gr->GetXaxis()->SetLabelSize ( 0.02 );
        gr->GetYaxis()->SetLabelSize ( 0.02 );
        gr->GetXaxis()->SetTitleSize ( 0.02 );
        gr->GetYaxis()->SetTitleSize ( 0.02 );
        gr->GetXaxis()->SetTickLength ( 0.02 );
        gr->Draw ( "AL" );

        gr->SetLineColor ( kRed );
        gr->SetLineStyle ( 1 );
        c->Update();
        c->SaveAs ( output.c_str() );
        delete sample;

        return 0;
}
