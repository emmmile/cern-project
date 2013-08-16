
#include <math.h>
#include "peakanalyzer.hpp"


peakanalyzer::peakanalyzer( const string& n, const int s )
        : name( n ), peaksnumber( 0 ), typecounter( 0 ), signal( s ) {
        // for computing the average derivative of the last samples
        beta = 1.0 / 4.0;
        dv = 0.0; // the derivative of the value. EWMA
        dvthreshold = 0.2; // standard deviation for the derivative, EWMA

        // the value of the baseline, when the detector is in "idle"
        baseline = 0.0; // EWMA
        alpha = 1.0 / typethreshold;
        baselinethreshold = 0.5; // standard deviation for the value, EWMA

        // initial status
        detector_status = START;
        detector_type = UNSET;

        oldtime = 0;
        oldvalue = 0;


        //cout << "#time value baseline vthreshold dv dvthreshold " << endl;
}

int peakanalyzer::peaks ( ) const {
        return peaksnumber;
}

void peakanalyzer::updatewindow ( ) {
        for ( unsigned int j = 0; j < last.size(); j++ ) {
                double value = last.value(j);
                double time = last.time(j);
                //F dv = last.dv(j);
                update( time, value );
        }
}


void peakanalyzer::update ( const double& time, const double& value ) {

        double dt = ( time - oldtime );
        double currentdv = ( value - oldvalue ) / dt;
        dv = ( 1.0 - beta ) * dv + beta * currentdv;
        oldtime = time;
        oldvalue = value;

        baseline = alpha * value + ( 1.0 - alpha ) * baseline;
        dvthreshold = alpha * fabs ( dv - currentdv ) + ( 1.0 - alpha ) * dvthreshold; // update the standard deviation for the derivative
        baselinethreshold = alpha * fabs ( value - baseline ) + ( 1.0 - alpha ) * baselinethreshold; // update the standard deviation for the values
}

void peakanalyzer::push( const double& time, const double& value ) {
        // just the first time run the following code
        if ( detector_status == START ) {
                baseline = value;
                oldtime = time;
                oldvalue = value;
                detector_status = BASELINE;
                return;
        }

        double dt = ( time - oldtime );
        double currentdv = ( value - oldvalue ) / dt;
        dv = ( 1.0 - beta ) * dv + beta * currentdv;
        oldtime = time;
        oldvalue = value;
        typecounter++;


        switch ( detector_status ) {
        case BASELINE:
                baseline = alpha * value + ( 1.0 - alpha ) * baseline;
                dvthreshold = beta * fabs ( dv - currentdv ) + ( 1.0 - beta ) * dvthreshold; // update the standard deviation for the derivative
                baselinethreshold = alpha * fabs ( value - baseline ) + ( 1.0 - alpha ) * baselinethreshold; // update the standard deviation for the values

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
                        if ( signal == 1 ) {
                                detector_status = GAMMAFLASH;
                        } else {
                                detector_status = PEAKRECORDING;
                        }
                        last.push ( time, value );
                }
                break;
        case GAMMAFLASH:
                // don't do anything special at the moment
        case PEAKRECORDING:
                // accumulate integral and put derivatives inside a vector for future check
                last.push ( time, value );

                // and we are close to the baseline, it's not a peak.
                // I simply empty the window to remember it.
                if ( fabs ( value - baseline ) < 3 * max ( baselinethreshold, 1.0 ) ) {
                        if ( last.good( baseline, baselinethreshold, detector_type ) ) {
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
