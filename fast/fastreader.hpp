#ifndef FASTREADER_HPP
#define FASTREADER_HPP
#include "dateutils.hpp"
#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>
#include "peakanalyzer.hpp"
#include "castor.hpp"
#include "gui.hpp"
using namespace std;



///
/// \brief The fastreader class reads the FAST data from CASTOR.
///
class fastreader {
    time_t start;
    time_t end;
    gui* window;
    string outdir;


public:
    fastreader( time_t s, time_t e, gui* w, const string& o = "./output/" )
        : start( s ), end( e ), window( w ), outdir( o ) {
    }

    ///
    /// \brief Reads the FAST data from CASTOR. It uses a local database avoiding reading all the
    /// CASTOR data for querying the file creation dates.
    /// \param db the location of the local database.
    ///
    void read( fstream& db ) {
        string run, timestamp, segments, path;

        mkdir( outdir.c_str(), 0777 );
        fstream out( outdir + "Fast.dat", std::fstream::out );

        if ( !out ) {
            cerr << "[fast] ERROR: unable to open \"output/Fast.dat\".\n";
            exit( 1 );
        }

        out << "# Time\t Run\t Detectors..\n";
        out.flush();


        while ( db >> run >> timestamp >> segments >> path ) {
            //string tt = day + " " + month + " " + year + " " + hour;
            //string ff = "%d %b %Y %H:%M";

            time_t current = atoi( timestamp.c_str() );

            if ( difftime(current, start) > 0 && difftime( current, end ) < 0 ) {
                cout << "[fast] Reading run " << run << " in " << path << "." << endl;
                out << current << "\t" << run << "\t";

                castor( path, atoi( run.c_str() ), out, current, start, window );
                out << endl;
                out.flush();
                cout << "[fast] Read run " << run << "." << endl;
                //getchar();
            }
        }

        cerr << "[fast] Done." << endl;
    }
};


#endif // FASTREADER_HPP
