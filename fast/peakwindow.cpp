#include <math.h>
#include <algorithm>
#include "peakwindow.hpp"


void peakwindow::push ( const double& time, const double& value ) {
        times.push_back ( time );
        values.push_back ( value );
}


const double& peakwindow::time ( int index ) const {
        return times[index];
}

const double& peakwindow::value ( int index ) const {
        return values[index];
}

void peakwindow::clear ( ) {
        times.clear();
        values.clear();
}

uint peakwindow::size ( ) const {
        return times.size();
}

double peakwindow::integral ( double baseline ) const {
        double out = 0.0;
        for ( uint i = 1; i < times.size(); ++i ) {
                out += fabs( baseline - values[i] ) * ( times[i] - times[i - 1] );
        }

        return out;
}

double peakwindow::minimum_integral( const double& baseline ) const {
        return ( times.back() - times.front() ) * fabs( baseline - minimum() ) * 0.25;
}

bool peakwindow::good ( const double& baseline, const double& vthreshold, type detector ) {
        double step = 6 * max( vthreshold, 1.0 );

        return integral( baseline ) > minimum_integral( baseline ) &&
                        ( ( detector == NEGATIVE && fabs( minimum() - baseline ) > step ) ||
                          ( detector == POSITIVE && fabs( maximum() - baseline ) > step ) );
}

const double& peakwindow::minimum ( ) const {
        return *min_element ( values.begin(), values.end() );
}

const double& peakwindow::maximum ( ) const {
        return *max_element( values.begin(), values.end() );
}

double peakwindow::minimum_time ( ) const {
        return times[min_element ( values.begin(), values.end() ) - values.begin()];
}

