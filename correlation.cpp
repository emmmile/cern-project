#include "correlation.hpp"
using namespace std;


correlator::correlator( ) {
        twindow = 3600.0; // seconds
}

void correlator::lock ( ) {
        datamutex.lock();
}

void correlator::unlock( ) {
        datamutex.unlock();
}

const int correlator::plotsnumber ( ) const {
        return data.size();
}

TGraph* correlator::plot ( const int index ) const {
        return data[index].first;
}

const int correlator::index ( const string& name ) const {
        //assert( detectors.left.count( name ) != 0 );
        return detectors.left.at( name );
}

const string& correlator::name ( const int index ) const {
        //assert( detectors.right.count( index ) != 0 );
        return detectors.right.at( index );
}

void correlator::addPoint( const string& name, double t, double y ) {
        if ( detectors.left.count( name ) == 0 ) {
                TGraph* newone = new TGraph();
                newone->SetTitle( name.c_str() );
                int maxIndex = data.size();

                detectors.insert( pairtype( name, maxIndex ) );

                // Add a column to the correlation matrix
                for ( int i = 0; i < data.size(); ++i ) {
                        data[i].second.push_back(correlationentry<double>());
                }

                // Add a row to the correlation matrix
                // create the correlation vector, there is one entry for every plot/detector
                // (included himself, for semplicity of the calculation of the indexes)
                cvector current( data.size() + 1, correlationentry<double>() );
                data.push_back( { newone, current } );

        }

        // Add a single point to the plot
        int index = detectors.left.at( name );
        int n = data[index].first->GetN();
        data[index].first->SetPoint( n, t, y );

        // Update the correlation entries relative to this plot
        for ( int j = 0; j < data.size(); ++j ) {
                //updateCorrelation( index, j );
                //updateCorrelation( j, index );
        }
}

void correlator::updateCorrelation ( const int i, const int j ) {
        if ( i == j ) return;

        TGraph* a = data[i].first;
        TGraph* b = data[j].first;

        if ( a->GetN() < 2 || b->GetN() < 2 ) return;

        // Since the two plots may be referred two different time intervals because
        // 1. Maybe data relative to one detector has still to come
        // 2. The time resolution of some detectors is not constant or different from the others

        // I have to search for a subset of the data that is overlapped, that is
        // referred to the same interval of time
        double* tabegin = a->GetX();
        double* tbbegin = b->GetX();
        double* taend = tabegin + a->GetN() - 1; // I want to include the right extreme
        double* tbend = tbbegin + b->GetN() - 1;

        // Find the time interval that is shared from the two plots
        double tbegin = max( *tabegin, *tbbegin );
        double tend   = min( *taend,   *tbend );

        // If the intersection is smaller than the wanted window, there
        // no enough information to compute the correlation
        if ( tend - tbegin < twindow ) return;


        // Now take a window at the back of the data
        tbegin = tend - twindow;
        // [tbegin, tend] is the time interval where the correlation will be computed

        // Search for the corresponding points in the data.
        // (I reuse the same variables for semplicity)
        // For example the A plot contains data from 0 to 1000s but I want
        // only the last window of 600s, that is [400,1000]. Where is the first
        // time after 400 in the plot? I will use this information also to understand
        // which is the corresponding value.
        tabegin = lower_bound( a->GetX(), a->GetX() + a->GetN(), tbegin );
        tbbegin = lower_bound( b->GetX(), b->GetX() + b->GetN(), tbegin );
        taend   = lower_bound( a->GetX(), a->GetX() + a->GetN(), tend );
        tbend   = lower_bound( b->GetX(), b->GetX() + b->GetN(), tend );
        //cout << "searching for [" << tbegin << ", " << tend << "]\n";
        //printf( "%lf %lf %lf %lf\n", *tabegin, *taend, *tbbegin, *tbend );

        int aoffset = tabegin - a->GetX();
        int boffset = tbbegin - b->GetX();
        int asize = taend - tabegin;
        int bsize = tbend - tbbegin;
        //printf( "%d %d %d %d\n", aoffset, boffset, asize, bsize );

        data[i].second[j].c[0] = linear( asize, tabegin, a->GetY() + aoffset, bsize, tbbegin, b->GetY() + boffset );
}

double correlator::linear ( const int an, double const* at, double const* av,
                            const int bn, double const* bt, double const* bv ) {
        if ( !an || !bn ) return 0.0;


        vector<double> t;
        vector<double> a;
        vector<double> b;
        interpolate( an, at, av, bn, bt, bv, t, a, b );

        int n = t.size();
        if ( n < 2 ) return 0.0;

        TVectorD x; x.Use( n, &(a[0]) );
        TVectorD y; y.Use( n, &(b[0]) );

        TMatrixD A(n,2);
        TMatrixDColumn(A,0) = 1.0;
        TMatrixDColumn(A,1) = x;

        const TVectorD c = NormalEqn(A,y);
        //cout << c[1] << endl;
        return c[1];
}


void correlator::interpolate ( const int an, double const* at, double const* av,
                               const int bn, double const* bt, double const* bv,
                               vector<double>& t, vector<double>& aa, vector<double>& bb ) {


        // this code is quite tricky.
        // basically scans the two time sequences and find the interpolation when the time instants
        // are not matching in the two plots.
        double const* atend = at + an;
        double const* btend = bt + bn;

        double const* a = at;
        double const* b = bt;
        for ( ; a != atend && b != btend; ) {
                // if *b falls outside the interval..
                if ( ( a != at && *a > *b && *(a-1) > *b ) ||
                                ( a == at && *a > *b ) ) {
                        swap( atend, btend );
                        swap( a, b );
                        swap( av, bv );
                        //swap( aa , bb );
                        aa.swap( bb );
                }

                while ( *a < *b && a != atend ) {
                        ++a;
                        ++av;
                }

                if ( a == atend ) break;

                if ( *a == *b ) {
                        //cout << *av << " " << *bv << endl;
                        t.push_back( *a );
                        aa.push_back( *av );
                        bb.push_back( *bv );
                        ++b; ++bv;
                        ++a; ++av;
                } else {
                        // now *a is bigger time than *b, but *(a-1) is smaller
                        // that is the time *b falls in the interval [*a, *(a-1)]
                        double m = ( *av - *(av-1) ) / ( *a - *(a-1) );
                        double ainterpolated = m * (*b - *(a-1)) + *(av-1);
                        //cout << ainterpolated << " " << *bv << endl;


                        t.push_back( *b );
                        aa.push_back( ainterpolated );
                        bb.push_back( *bv );
                        ++b; ++bv;
                }
        }

        // final swap
        if ( a > at && a < atend ); // ok
        else {
                aa.swap( bb );
        }
}
