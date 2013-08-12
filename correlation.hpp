#ifndef CORRELATION_HPP
#define CORRELATION_HPP

#ifndef __CINT__
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompChol.h"
#include "TF1.h"
#include <TGraph.h>

// every C++11 object must be enclosed for safety reasons into this guard
// http://root.cern.ch/phpBB3/viewtopic.php?f=5&t=15858
// otherwise rootcint fails, at least for what concerns ROOT 5
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

        mutex           __datamutex;
        bimaptype       __detectors;      // stores the association name <-> index
        vector<TGraph*> __data;           // stores the real data


public:
        correlator() {
        }

        void lock ( ) {
                __datamutex.lock();
        }

        void unlock( ) {
                __datamutex.unlock();
        }

        const int plotsnumber ( ) const {
                return __data.size();
        }

        TGraph* plot ( const int index ) const {
                return __data[index];
        }

        const int index ( const string& name ) const {
                assert( __detectors.left.count( name ) != 0 );
                return __detectors.left.at( name );
        }

        const string& name ( const int index ) const {
                assert( __detectors.right.count( index ) != 0 );
                return __detectors.right.at( index );
        }

        void addPoint( const string& name, double t, double y ) {
                if ( __detectors.left.count( name ) == 0 ) {
                        TGraph* newone = new TGraph();
                        newone->SetTitle( name.c_str() );
                        int maxIndex = __data.size();

                        __detectors.insert( pairtype( name, maxIndex ) );
                        __data.push_back( newone );
                }

                // add a single point to the plot
                int index = __detectors.left.at( name );
                int n = __data[index]->GetN();
                //cout << "Adding " << name << " " << n << " " << t << " " << y << endl;
                __data[index]->SetPoint( n, t, y );
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

        void run ( void ) {
                while ( 1 ) {
                        sleep( 5 );

                        boost::progress_timer elapsed;
                        lock();
                        for ( int j = 0; j < plotsnumber(); ++j ) {

                                for ( int i = j + 1; i < plotsnumber(); ++i ) {

                                        TGraph* a = __data[j];
                                        TGraph* b = __data[i];
                                        string first  = name( j );
                                        string second = name( i );
                                        //if ( !a || !b ) continue;

                                        double ab = c( a->GetN(), a->GetX(), a->GetY(), b->GetN(), b->GetX(), b->GetY() );
                                        double ba = c( b->GetN(), b->GetX(), b->GetY(), a->GetN(), a->GetX(), a->GetY() );

                                        if ( ab * ba > 0.80 && ab * ba < 1.20 ) {
                                                cout.precision( 10 );
                                                cout << "[correlator] correlation between " << first << " and " << second << " is " << ab * ba << endl;
                                        }
                                }
                        }
                        unlock();

                        cout << "[correlator] ";
                }
        }

#endif
};

#endif // CORRELATION_HPP
