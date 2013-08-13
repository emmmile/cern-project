#ifndef WINDOW_HPP
#define WINDOW_HPP
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
template<class F>
class peakwindow {
        vector<F> __times;
        vector<F> __values;

public:
        ///
        /// Adds a triple to the window
        ///
        void push ( const F& time, const F& value ) {
                __times.push_back ( time );
                __values.push_back ( value );
        }


        const F& time ( int index ) const {
                return __times[index];
        }

        const F& value ( int index ) const {
                return __values[index];
        }

        ///
        /// Clears the window.
        ///
        void clear ( ) {
                __times.clear();
                __values.clear();
        }

        uint size ( ) const {
                return __times.size();
        }


        ///
        /// \return the integral, referred to the argument baseline of the values present in the window.
        ///
        F integral ( F baseline ) const {
                F out = 0.0;
                for ( uint i = 1; i < __times.size(); ++i ) {
                        out += fabs( baseline - __values[i] ) * ( __times[i] - __times[i - 1] );
                }

                return out;
        }

        ///
        /// \return a lower bound for the integral in the current window, to indentify a peak.
        /// It is calculated as the area of a rectangle having width equal to the window and height of 1/4 the
        /// maximum peak (computed with \ref minimum).
        ///
        F minimum_integral( const F& baseline ) const {
                return ( __times.back() - __times.front() ) * fabs( baseline - minimum() ) * 0.25;
        }

        bool good ( const F& baseline, const F& vthreshold, type detector ) {
                F step = 6 * max( vthreshold, 1.0 );

                return integral( baseline ) > minimum_integral( baseline ) &&
                       ( ( detector == NEGATIVE && fabs( minimum() - baseline ) > step ) ||
                         ( detector == POSITIVE && fabs( maximum() - baseline ) > step ) );
        }

        ///
        /// \return the minumum value in the window.
        ///
        const F& minimum ( ) const {
                return *min_element ( __values.begin(), __values.end() );
        }

        ///
        /// \return the maximum value in the window.
        ///
        const F& maximum ( ) const {
                return *max_element( __values.begin(), __values.end() );
        }

        ///
        /// \return the time instant correspondent to the minimum value.
        ///
        F minimum_time ( ) const {
                return __times[min_element ( __values.begin(), __values.end() ) - __values.begin()];
        }

        friend ostream& operator << ( ostream& out, const peakwindow& w ) {
                for ( uint i = 0; i < w.__times.size(); ++i )
                        out << w.__times[i] << " " << w.__values[i] << endl;

                //out << "Total integral is " << w.integral() << endl;
                out << "Minimum is " << w.minimum() << endl;
                return out;
        }
};

#endif // WINDOW_HPP
