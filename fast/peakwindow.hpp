#ifndef PEAKWINDOW_HPP
#define PEAKWINDOW_HPP
#include <vector>
#include <numeric>
#include <iostream>
using namespace std;

typedef unsigned int uint;


enum type { POSITIVE, NEGATIVE, UNSET };

///
/// \class peakwindow
/// \brief This class represents a time window of values coming from a detector. It used, while analyzing the data,
/// to remember a part of it without having to read it again.
///
class peakwindow {
        vector<double> times;
        vector<double> values;

public:
        ///
        /// Adds a triple to the window
        ///
        void push ( const double& time, const double& value );

        const double& time ( int index ) const;

        const double& value ( int index ) const;

        ///
        /// Clears the window.
        ///
        void clear ( );

        uint size ( ) const;

        ///
        /// \return the integral, referred to the argument baseline of the values present in the window.
        ///
        double integral ( double baseline ) const;

        ///
        /// \return a lower bound for the integral in the current window, to indentify a peak.
        /// It is calculated as the area of a rectangle having width equal to the window and height of 1/4 the
        /// maximum peak (computed with \ref minimum).
        ///
        double minimum_integral( const double& baseline ) const;

        bool good ( const double& baseline, const double& vthreshold, type detector );

        ///
        /// \return the minumum value in the window.
        ///
        const double& minimum ( ) const;

        ///
        /// \return the maximum value in the window.
        ///
        const double& maximum ( ) const;

        ///
        /// \return the time instant correspondent to the minimum value.
        ///
        double minimum_time( ) const;



        friend ostream& operator << ( ostream& out, const peakwindow& w ) {
                for ( uint i = 0; i < w.times.size(); ++i )
                        out << w.times[i] << " " << w.values[i] << endl;

                //out << "Total integral is " << w.integral() << endl;
                out << "Minimum is " << w.minimum() << endl;
                return out;
        }
};

#endif // PEAKWINDOW_HPP
