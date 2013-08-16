#ifndef PEAKANALYZER_HPP
#define PEAKANALYZER_HPP

#include <iostream>
#include <sys/stat.h>
#include <mutex>
#include <algorithm>
#include "peakwindow.hpp"
using namespace std;

enum status { START, BASELINE, PEAKRECORDING, GAMMAFLASH };

///
/// \brief The peakanalyzer class searches for peaks in the raw data coming from a detector move,
/// inside an event of a run. It tries to be as general as possible, adapting to various detector types.
/// It also understand if the detector will have negative or positive peaks.
///
///
class peakanalyzer {
        double          beta;
        double          alpha;
        double          dv;
        double          dvthreshold;
        double          baseline;
        double          baselinethreshold;

        double          oldtime;
        double          oldvalue;

        peakwindow      last;
        status          detector_status;
        type            detector_type;
        string          name;

        int peaksnumber;
        int typecounter;
        static const int typethreshold = 128;
        int signal;
public:
        peakanalyzer( const string& n, const int s );

        ///
        /// \return the number of peaks found.
        ///
        int peaks ( ) const;

        void updatewindow ( );


        void update ( const double& time, const double& value );

        ///
        /// \brief Analyze a single additional pair \ref time, \ref value.
        ///
        void push( const double& time, const double& value );
};

#endif // PEAKANALYZER_HPP
