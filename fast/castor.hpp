#ifndef CASTOR_HPP
#define CASTOR_HPP

#include "fast/reader/RawReader.hh"
#include "fast/peakanalyzer.hpp"
#include "gui.hpp"
#include <iostream>
using namespace std;



///
/// \brief The castor class reads a single run from CASTOR, processing it with a peak analyzer.
///
class castor {

        RunReader*   theRunReader;
        ACQCInfo*    theACQCInfo;
        ModuleInfo*  info;

        time_t       runstart;
        time_t       userstart;
        correlator*  data;
public:
        ///
        /// \brief Construct a \ref castor object, starting reading the data of the detectors.
        ///
        /// \param path the location of the experiment directory on CASTOR.
        /// \param run the run number.
        /// \param rs the run start time.
        /// \param us the start time specified by the user.
        /// \param d pointer to the correlator, where the final data is stored.
        ///
        castor( const string& path, const int& run, const time_t rs, const time_t us, correlator* d );


        /// \brief Read the data for a specific detector of the current run. After reading it runs the
        /// peak analyzer saving the results into \ref out.
        ///
        /// \param detector the detector name.
        /// \param id the detector id.
        /// \param out a stream where to write the (analyzed) data.
        ///
        void readDetector( const string& detector, const int& id );
};

#endif // CASTOR_HPP
