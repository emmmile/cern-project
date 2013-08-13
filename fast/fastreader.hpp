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
        time_t          start;
        time_t          end;
        correlator*     data;
        string          outdir;

public:
        ///
        /// \brief fastreader
        /// \param s the starting time specified by the user.
        /// \param e the ending time specified by the user.
        /// \param d pointer to the correlator data structure.
        /// \param o the directory where the data is stored.
        ///
        fastreader( time_t s, time_t e, correlator* d, const string& o = "./output/" )
                : start( s ), end( e ), data( d ), outdir( o ) {
        }

        ///
        /// \brief Reads the FAST data from CASTOR. It uses a local database avoiding reading all the
        /// CASTOR data for querying the file creation dates.
        /// \param db the location of the local database. Since the amount of data is not very big,
        /// at the moment it is a simple text file.
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

                // reads one line at the time (that is one run at the time)
                while ( db >> run >> timestamp >> segments >> path ) {
                        time_t current = atoi( timestamp.c_str() );

                        // if the the run start is inside the specified interval then
                        // really read it
                        if ( difftime(current, start) > 0 && difftime( current, end ) < 0 ) {
                                cout << "[fast] Reading run " << run << " in " << path << "." << endl;
                                out << current << "\t" << run << "\t";

                                castor( path, atoi( run.c_str() ), out, current, start, data );
                                out << endl;
                                out.flush();
                                cout << "[fast] Read run " << run << "." << endl;
                        }
                }

                cerr << "[fast] Done." << endl;
        }
};


#endif // FASTREADER_HPP
