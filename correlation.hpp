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

#include <vector>
using namespace std;


class correlator {
// since this class uses heavily incompatible structures with rootcint I enclose everything with a guard
#ifndef __CINT__
        typedef boost::bimap<string,int> bimaptype;
        typedef bimaptype::value_type pairtype;

        mutex           datamutex;
        bimaptype       detectors;      // stores the association name <-> index
        vector<TGraph*> data;           // stores the real data

        // I'm using TGraph because otherwise I would have to keep two vectors for each detector (time and value)
        // and then duplicate the memory creating a TGraph for viewing purposes. In fact there is no constructor in
        // TGraph where the data is actually SHARED, see http://root.cern.ch/root/html/TGraph.html

public:
        void lock ( ) {
                datamutex.lock();
        }

        void unlock( ) {
                datamutex.unlock();
        }

        const int plotsnumber ( ) const {
                return data.size();
        }

        TGraph* plot ( const int index ) const {
                return data[index];
        }

        const int index ( const string& name ) const {
                assert( detectors.left.count( name ) != 0 );
                return detectors.left.at( name );
        }

        const string& name ( const int index ) const {
                assert( detectors.right.count( index ) != 0 );
                return detectors.right.at( index );
        }

        void addPoint( const string& name, double t, double y ) {
                if ( detectors.left.count( name ) == 0 ) {
                        TGraph* newone = new TGraph();
                        newone->SetTitle( name.c_str() );
                        int maxIndex = data.size();

                        detectors.insert( pairtype( name, maxIndex ) );
                        data.push_back( newone );
                }

                // add a single point to the plot
                int index = detectors.left.at( name );
                int n = data[index]->GetN();
                //cout << "Adding " << name << " " << n << " " << t << " " << y << endl;
                data[index]->SetPoint( n, t, y );
        }


        void interpolate ( const int an, double const* at, double const* av, const int bn, double const* bt, double const* bv, vector<double>& t, vector<double>& aa, vector<double>& bb ) {
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


        // find linear correlation coefficient between a (x) and b (y)
        double c ( const int an, double const* at, double const* av, const int bn, double const* bt, double const* bv ) {
                if ( !an || !bn ) return 0;


                vector<double> t;
                vector<double> a;
                vector<double> b;
                interpolate( an, at, av, bn, bt, bv, t, a, b );

                int n = t.size();
                if ( n <= 2 ) return 0;

                TVectorD x; x.Use( n, &(a[0]) );
                TVectorD y; y.Use( n, &(b[0]) );

                TMatrixD A(n,2);
                TMatrixDColumn(A,0) = 1.0;
                TMatrixDColumn(A,1) = x;

                const TVectorD c = NormalEqn(A,y);
                return c[1];
        }

        class sorter {
                int size;
                double* base;
        public:
                sorter( int size, double* C )
                        : size( size ), base( C ) {
                }

                bool operator() ( const double* a, const double* b ) {
                        return fabs(*a - 1.0) < fabs(*b - 1.0);
                }
        };

        void run ( void ) {
                while ( 1 ) {
                        sleep( 5 );

                        boost::progress_timer elapsed;
                        lock();
                        int size = plotsnumber();
                        double C[size * size];          // a correlation matrix
                        fill( C, C + size * size, numeric_limits<double>::max() );

                        for ( int j = 0; j < plotsnumber(); ++j ) {
                                for ( int i = j + 1; i < plotsnumber(); ++i ) {

                                        TGraph* a = data[j];
                                        TGraph* b = data[i];

                                        double ab = c( a->GetN(), a->GetX(), a->GetY(), b->GetN(), b->GetX(), b->GetY() );
                                        double ba = c( b->GetN(), b->GetX(), b->GetY(), a->GetN(), a->GetX(), a->GetY() );

                                        C[j * size + i] = ab * ba;
                                }
                        }

                        // sorting pass, to print say the most 20 correlated pairs
                        double* viewOnC[size * size];
                        for ( int i = 0; i < size * size; ++i ) viewOnC[i] = C + i;

                        sort( viewOnC, viewOnC + size * size, sorter(size, C) );

                        cout.precision( 10 );
                        for ( int i = 0; i < 20; ++i ) {
                                int j = viewOnC[i] - C; // distance from the base of the array

                                int first = j / size;   // go back to the indexes
                                int second = j % size;
                                if ( *viewOnC[i] == numeric_limits<double>::max() ) break;

                                cout << "[correlator] correlation between " << name(first) << " and " << name(second) << " is " << *viewOnC[i] << endl;
                        }
                        unlock();

                        cout << "[correlator] "; // will print the elapsed time if a progress_timer is used
                }
        }

#endif
};

#endif // CORRELATION_HPP
