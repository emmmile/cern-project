#ifndef CORRELATION_HPP
#define CORRELATION_HPP

#ifndef __CINT__
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompChol.h"
#include "TF1.h"
#endif


class correlation {
public:

        correlation() {
        }




        // find linear correlation coefficient between a (x) and b (y)
        double operator() ( const int an, const double* at, const double* av, const int bn, const double* bt, const double* bv ) {
                int npoints = min( an, bn );

                TVectorD x; x.Use( an, av);
                TVectorD y; y.Use( bn, bv);

                TMatrixD A(npoints,2);
                TMatrixDColumn(A,0) = 1.0;
                TMatrixDColumn(A,1) = x;


                const TVectorD c = NormalEqn(A,y);
                return c[1];
        }
};

#endif // CORRELATION_HPP
