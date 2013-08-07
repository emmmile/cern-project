#ifndef ANALYZER_HPP
#define ANALYZER_HPP

#include <iostream>
#include <sys/stat.h>
#include <mutex>
#include <cmath>
#include "fast/window.hpp"
using namespace std;

enum type { POSITIVE, NEGATIVE, UNSET };
enum status { START, BASELINE, PEAKEND, PEAKSTART };

///
/// \brief The peakanalyzer class searches for peaks in the raw data coming from a detector move,
/// inside an event of a run.
///
class peakanalyzer {
    float beta, dv, dvthreshold, baseline, alpha, vthreshold;
    window<float> last;
    status detector_status;
    type   detector_type;
    string name;

    int peaksnumber;
    int typecounter;
    static const int typethreshold = 64;

    float oldtime, oldvalue;
public:
    peakanalyzer( const string& n ) : name( n ), peaksnumber( 0 ), typecounter( 0 ) {
        // for computing the average derivative of the last samples
        beta = 1.0 / 4.0;
        dv = 0.0; // the derivative of the value. EWMA
        dvthreshold = 0.2; // standard deviation for the derivative, EWMA

        // the value of the baseline, when the detector is in "idle"
        baseline = 0.0; // EWMA
        alpha = 1.0 / 32.0;
        vthreshold = 0.5; // standard deviation for the value, EWMA

        // initial status
        detector_status = START;
        detector_type = UNSET;

        oldtime = 0;
        oldvalue = 0;
    }


    void resetStatus ( ) {
        last.clear();
        detector_status = BASELINE;
    }


    ///
    /// \brief Analyze a single additional pair \ref time, \ref value.
    ///
    void push( const float& time, const float& value ) {

        //cout << std::this_thread::get_id() << " " << time << " " << value << endl;
        if ( detector_status == START ) {
            baseline = value; // just the first time
            oldtime = time;
            oldvalue = value;
            detector_status = BASELINE;
            return;
        }

        // compute the last derivative
        float dt = time - oldtime;
        float currentdv = ( value - oldvalue ) / dt;
        dv = ( 1.0 - beta ) * dv + beta * currentdv;
        oldtime = time;
        oldvalue = value;
        typecounter++;


        switch ( detector_status ) {
            case BASELINE:
              // compute the baseline value, using a moving average. Ask Carlos.
              baseline = alpha * value + ( 1.0 - alpha ) * baseline;
              dvthreshold = alpha * fabs ( dv - currentdv ) + ( 1.0 - alpha ) * dvthreshold; // update the standard deviation for the derivative
              vthreshold = alpha * fabs ( value - baseline ) + ( 1.0 - alpha ) * vthreshold; // update the standard deviation for the values


              if ( typecounter < typethreshold )
                break;
              else if ( detector_type == UNSET ) {
                if ( baseline > 127.0 )
                     detector_type = POSITIVE;
                else detector_type = NEGATIVE;
                cout << name << " is a " << (detector_type == POSITIVE ? "positive" : "negative") << " detector." << endl;
              }

              // if derivative is big change state, a possible peak is happening
              if ( ( dv < -dvthreshold && detector_type == NEGATIVE ) ||
                   ( dv >  dvthreshold && detector_type == POSITIVE ) ) {
                detector_status = PEAKSTART;
                last.push ( time, value, dv );
              }

              // probably found big noise, go into another state to avoid computing the baseline
              if ( ( dv >  dvthreshold && detector_type == NEGATIVE ) ||
                   ( dv < -dvthreshold && detector_type == POSITIVE ) )
                detector_status = PEAKEND;
              break;
            case PEAKSTART:
              // accumulate integral and put derivatives inside a vector for future check
              last.push ( time, value, dv );

              // when the derivative changes sign...
              if ( ( dv > 0.0 && detector_type == NEGATIVE ) ||
                   ( dv < 0.0 && detector_type == POSITIVE ) ) {

                detector_status = PEAKEND;

                // and we are close to the baseline, it's not a peak.
                // I simply empty the window to remember it.
                if ( fabs ( value - baseline ) < 3 * max ( vthreshold, 1.0f ) )
                    last.clear();

                //if ( fabs ( value - baseline ) > 3 * max ( vthreshold, 1.0f ) ) {
                //} else { last.clear(); }
              }
              break;
            case PEAKEND:
              if ( last.size() > 0 ) {
                last.push ( time, value, dv );
              }

              // if value returns reasonably close to the baseline, we have to go back into BASELINE state
              if ( fabs ( dv ) < dvthreshold && fabs ( value - baseline ) < 3 * max ( vthreshold, 1.0f ) ) {
                if ( last.size() > 0 ) { // so this was a peak (at least looking at the distance from the baseline...
                  // but I check also the integral
                  if ( last.integral ( baseline ) > last.minimum_integral() ) {
                    //    cout << last << endl;
                    //cout << "[fast] Good peak at " << last.minimum_time() << " [" << name << "].\n";
                    ++peaksnumber;
                    //getchar();
                  }
                }

                last.clear();
                detector_status = BASELINE;
              }
              break;
            default:
              break;
        }
    }


    ///
    /// \return the number of peaks found.
    ///
    int peaks ( ) const {
        return peaksnumber;
    }


    void analyze ( iostream& s ) { //, mutex& m ) {
        //m.lock();
        cout << "[fast] Started analyzing " << name << "..." << endl;

        // start the reading loop, I think could be easily tuned for online behaviour
        float time, value;
        while ( s >> time >> value ) {
            push(time, value );
        }

//        cout << "Last baseline was " << baseline << endl;
//        cout << "Last dvthreshold was " << dvthreshold << endl;
//        cout << "Last vthreshold was " << vthreshold << endl;

        cout << "[fast] Done with " << name << "" << endl;
        //m.unlock();
    }
};

#endif // ANALYZER_HPP
