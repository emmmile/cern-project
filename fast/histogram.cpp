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
using namespace std;

// for i in sample\ data/*.dat; do ./histogram "$i"; done

int main ( int argc, char** argv )
{
        if ( argc < 2 ) {
                cerr << "Give me a file." << endl;
                exit ( 1 );
        }
        fstream sample ( argv[1] );
        cout << "File: " << argv[1] << endl;

        TCanvas *c = new TCanvas ( "c","Example",4000,3000 );
        c->cd();

        vector<float> times;
        vector<float> values;
        float time, value;
        while ( sample >> time >> value ) {
                times.push_back ( time );
                values.push_back ( value );
        }

        TGraph* gr = new TGraph ( times.size(), &times[0], &values[0] );
        gr->SetTitle ( argv[1] );
        gr->GetXaxis()->SetTitle ( "time" );
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
        string name = "" + string ( argv[1] ) + ".png";
        c->SaveAs ( name.c_str() );

        return 0;
}
