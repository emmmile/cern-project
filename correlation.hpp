#ifndef CORRELATION_HPP
#define CORRELATION_HPP

#ifndef __CINT__
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompChol.h"
#include "TF1.h"
#include <TGraph.h>

// every C++11 object must be enclosed for safety reasons into this #ifndef __CINT__
// otherwise rootcint fails, at least for what concerns ROOT 5
// http://root.cern.ch/phpBB3/viewtopic.php?f=5&t=15858
#include <mutex>
#include <map>
#include <set>
#include <boost/progress.hpp>
#include <boost/bimap.hpp>
#endif

#include "correlationentry.hpp"
#include <vector>
using namespace std;


class correlator {
        // since this class uses heavily incompatible structures with rootcint I enclose everything with a guard
#ifndef __CINT__
        typedef boost::bimap<string,int>         bimaptype;
        typedef bimaptype::value_type            pairtype;
        typedef vector<correlationentry<double>> cvector;
        typedef pair<TGraph*, cvector>           ventry;

        mutex           datamutex;
        bimaptype       detectors;      // stores the association name <-> index
        vector<ventry>  data;           // stores the real data and its correlations

        double          twindow;

        // I'm using TGraph because otherwise I would have to keep two vectors for each detector (time and value)
        // and then duplicate the memory creating a TGraph for viewing purposes. In fact there is no constructor in
        // TGraph where the data is actually SHARED, see http://root.cern.ch/root/html/TGraph.html





        class correlationsorter {
                int size;
                double* base;
        public:
                correlationsorter( int size, double* C )
                        : size( size ), base( C ) {
                }

                bool operator() ( const double* a, const double* b ) {
                        return fabs(*a - 1.0) < fabs(*b - 1.0);
                }
        };

public:
        correlator( );

        void lock ( );
        void unlock( );

        ///
        /// \brief plotsnumber
        /// \return the number of the detectors
        ///
        const int plotsnumber ( ) const;

        ///
        /// \brief plot
        /// \param index the index of the detector in the data structure.
        /// \return the data of the detector with a given index
        ///
        TGraph* plot ( const int index ) const;

        ///
        /// \brief index
        /// \param name the name of the detector as a string
        /// \return the index of the detector with the given name
        ///
        const int index ( const string& name ) const;

        ///
        /// \brief name
        /// \param index the index of the detector in the data vector
        /// \return the name of the detector with the given index
        ///
        const string& name ( const int index ) const;

        ///
        /// \brief addPoint
        ///        Add a point to the data structure
        /// \param name the name of the detectors
        /// \param t the time of the new point
        /// \param y the value of the new point
        ///
        void addPoint( const string& name, double t, double y );

        ///
        /// \brief updateCorrelation
        ///        updates the existing correlation between detector i and j
        /// \param i the index of the first detector, that can be used on the data vector
        /// \param j the index of the second detector
        ///
        void updateCorrelation ( const int i, const int j );


        ///
        /// \brief linear
        ///        find linear correlation coefficient between A(t) and A(t)
        /// \param an the number of points for the function A
        /// \param at the begin of the time array for the function A
        /// \param av the begin of the value array for the function A
        /// \param bn the number of points for the function B
        /// \param bt the times of B
        /// \param bv the values of B
        /// \return
        ///
        double linear ( const int an, double const* at, double const* av,
                        const int bn, double const* bt, double const* bv );


        ///
        /// \brief interpolate
        ///        find a linear interpolation for the points that don't have a correspondent time in one of
        ///        the two plots
        /// \param an the number of points for the function A
        /// \param at the begin of the time array for the function A
        /// \param av the begin of the value array for the function A
        /// \param bn the number of points for the function B
        /// \param bt the times of B
        /// \param bv the values of B
        /// \param t the output vector of times
        /// \param aa the output vector of values of A
        /// \param bb the output vector of values of B
        ///
        void interpolate ( const int an, double const* at, double const* av,
                           const int bn, double const* bt, double const* bv,
                           vector<double>& t, vector<double>& aa, vector<double>& bb );
#endif
};

#endif // CORRELATION_HPP
