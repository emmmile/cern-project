#include "fastreader.hpp"
using namespace std;


fastreader::fastreader( const time_t s, const time_t e, correlator* d )
        : start( s ), end( e ), data( d ) {
}

void fastreader::read( ifstream& db ) {
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
