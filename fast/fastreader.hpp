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

public:
        ///
        /// \brief fastreader
        /// \param s the starting time specified by the user.
        /// \param e the ending time specified by the user.
        /// \param d pointer to the correlator data structure.
        ///
        fastreader( const time_t s, const time_t e, correlator* d )
                : start( s ), end( e ), data( d ) {
        }

        ///
        /// \brief Reads the FAST data from CASTOR. It uses a local database avoiding reading all the
        /// CASTOR data for querying the file creation dates.
        /// \param db the location of the local database. Since the amount of data is not very big,
        /// at the moment it is a simple text file.
        ///
        void read( ifstream& db ) {
                string run, timestamp, segments, path;

                // reads one line at the time (that is one run at the time)
                while ( db >> run >> timestamp >> segments >> path ) {
                        time_t current = atoi( timestamp.c_str() );

                        // if the the run start is inside the specified interval then really read it
                        if ( difftime(current, start) > 0 && difftime( current, end ) < 0 ) {
                                cout << "[fast] Reading run " << run << " in " << path << "." << endl;
                                castor( path, atoi( run.c_str() ), current, start, data );
                                cout << "[fast] Read run " << run << "." << endl;
                        }
                }

                cerr << "[fast] Done." << endl;
        }
};


#endif // FASTREADER_HPP
