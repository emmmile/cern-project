#ifndef CORRELATION_HPP
#define CORRELATION_HPP

#ifndef __CINT__
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompChol.h"
#include "TF1.h"
#endif


#include <vector>
using namespace std;


class correlation {
public:

        correlation() {
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
        double operator() ( const int an, double const* at, double const* av, const int bn, double const* bt, double const* bv ) {
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
};

#endif // CORRELATION_HPP
