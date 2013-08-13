#ifndef PEAKANALYZER_HPP
#define PEAKANALYZER_HPP

#include <iostream>
#include <sys/stat.h>
#include <mutex>
#include "peakwindow.hpp"
using namespace std;

enum status { START, BASELINE, PEAKRECORDING, GAMMAFLASH };

///
/// \brief The peakanalyzer class searches for peaks in the raw data coming from a detector move,
/// inside an event of a run. It tries to be as general as possible, adapting to various detector types.
/// It also understand if the detector will have negative or positive peaks.
///
template<class F> // if you want to easily switch between double and float
class peakanalyzer {
        F               beta;
        F               alpha;
        F               dv;
        F               dvthreshold;
        F               baseline;
        F               vthreshold;

        F               oldtime;
        F               oldvalue;

        peakwindow<F>   last;
        status          detector_status;
        type            detector_type;
        string          name;

        int peaksnumber;
        int typecounter;
        static const int typethreshold = 128;
public:
        peakanalyzer( const string& n )
                : name( n ), peaksnumber( 0 ), typecounter( 0 ) {
                // for computing the average derivative of the last samples
                beta = 1.0 / 4.0;
                dv = 0.0; // the derivative of the value. EWMA
                dvthreshold = 0.2; // standard deviation for the derivative, EWMA

                // the value of the baseline, when the detector is in "idle"
                baseline = 0.0; // EWMA
                alpha = 1.0 / typethreshold;
                vthreshold = 0.5; // standard deviation for the value, EWMA

                // initial status
                detector_status = START;
                detector_type = UNSET;

                oldtime = 0;
                oldvalue = 0;


                //cout << "#time value baseline vthreshold dv dvthreshold " << endl;
        }

        ///
        /// \return the number of peaks found.
        ///
        int peaks ( ) const {
                return peaksnumber;
        }

        void updatewindow ( ) {
                for ( unsigned int j = 0; j < last.size(); j++ ) {
                        F value = last.value(j);
                        F time = last.time(j);
                        //F dv = last.dv(j);
                        update( time, value );
                }
        }


        void update ( const F& time, const F& value ) {

                F dt = ( time - oldtime );
                F currentdv = ( value - oldvalue ) / dt;
                dv = ( 1.0 - beta ) * dv + beta * currentdv;
                oldtime = time;
                oldvalue = value;

                baseline = alpha * value + ( 1.0 - alpha ) * baseline;
                dvthreshold = alpha * fabs ( dv - currentdv ) + ( 1.0 - alpha ) * dvthreshold; // update the standard deviation for the derivative
                vthreshold = alpha * fabs ( value - baseline ) + ( 1.0 - alpha ) * vthreshold; // update the standard deviation for the values
        }

        ///
        /// \brief Analyze a single additional pair \ref time, \ref value.
        ///
        void push( const F& time, const F& value ) {
                // just the first time run the following code
                if ( detector_status == START ) {
                        baseline = value;
                        oldtime = time;
                        oldvalue = value;
                        detector_status = BASELINE;
                        return;
                }

                F dt = ( time - oldtime );
                F currentdv = ( value - oldvalue ) / dt;
                dv = ( 1.0 - beta ) * dv + beta * currentdv;
                oldtime = time;
                oldvalue = value;
                typecounter++;


                switch ( detector_status ) {
                case BASELINE:
                        baseline = alpha * value + ( 1.0 - alpha ) * baseline;
                        dvthreshold = beta * fabs ( dv - currentdv ) + ( 1.0 - beta ) * dvthreshold; // update the standard deviation for the derivative
                        vthreshold = alpha * fabs ( value - baseline ) + ( 1.0 - alpha ) * vthreshold; // update the standard deviation for the values

                        // if I've not read enough samples to understand the detector type, then quit immediately
                        if ( typecounter < typethreshold )
                                break;
                        else if ( detector_type == UNSET ) {
                                if ( baseline > 127.0 )
                                        detector_type = NEGATIVE;
                                else detector_type = POSITIVE;
                                //cout << name << " is a " << (detector_type == POSITIVE ? "positive" : "negative") << " detector." << endl;
                        }

                        // if derivative is big then change state, because a possible peak is happening
                        if ( ( dv < -dvthreshold && detector_type == NEGATIVE ) ||
                             ( dv >  dvthreshold && detector_type == POSITIVE ) ) {
                                detector_status = PEAKRECORDING;
                                last.push ( time, value );
                        }
                        break;
                case PEAKRECORDING:
                        // accumulate integral and put derivatives inside a vector for future check
                        last.push ( time, value );

                        // and we are close to the baseline, it's not a peak.
                        // I simply empty the window to remember it.
                        if ( fabs ( value - baseline ) < 3 * max ( vthreshold, 1.0 ) ) {
                                if ( last.good( baseline, vthreshold, detector_type ) ) {
                                        //cerr << "Good peak at " << last.minimum_time() << " [" << name << "].\n";
                                        //cerr << last.minimum_integral(baseline) << " " << last.integral( baseline ) << endl;
                                        ++peaksnumber;
                                } else {
                                        updatewindow();
                                }


                                last.clear(); // TODO update the variance!!!! If it wasn't a peak, I have to update the averages..
                                detector_status = BASELINE;
                        }
                        break;
                default:
                        break;
                }

                //cout.precision( 30 );
                //cout << time << " " << value << " " << baseline << " " << vthreshold << " " << dv << " " << dvthreshold << endl;
        }

        friend ostream& operator<< ( ostream& out, const peakanalyzer& p ) {
                out << "Current baseline is " << p.baseline << endl;
                out << "Current dvthreshold is " << p.dvthreshold << endl;
                out << "Current vthreshold is " << p.vthreshold << endl;
                out << "Number of peaks is " << p.peaks() << endl;
                return out;
        }
};

#endif // PEAKANALYZER_HPP
