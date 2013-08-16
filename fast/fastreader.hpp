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
        fastreader( const time_t s, const time_t e, correlator* d );

        ///
        /// \brief Reads the FAST data from CASTOR. It uses a local database avoiding reading all the
        /// CASTOR data for querying the file creation dates.
        /// \param db the location of the local database. Since the amount of data is not very big,
        /// at the moment it is a simple text file.
        ///
        void read( ifstream& db );
};


#endif // FASTREADER_HPP
