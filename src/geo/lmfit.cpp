/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#include "lmfit.h"
#include <cmath>
#include <cstdio>
#include <vector>

namespace geo {

/*
 * This file contains default implementation of the evaluate and printout
 * routines. In most cases, customization of lmfit can be done by modifying
 * these two routines. Either modify them here, or copy and rename them.
 */

void lm_evaluate_default( double* par, int m_dat, double* fvec,
                          void *data, int *info )
/*
        *    par is an input array. At the end of the minimization, it contains
        *        the approximate solution vector.
        *
        *    m_dat is a positive integer input variable set to the number
        *      of functions.
        *
        *    fvec is an output array of length m_dat which contains the function
        *        values the square sum of which ought to be minimized.
        *
        *    data is a read-only pointer to lm_data_type, as specified by lmuse.h.
        *
        *      info is an integer output variable. If set to a negative value, the
        *        minimization procedure will stop.
 */
{
    lm_data_type *mydata = (lm_data_type*)data;

    for (int i=0; i<m_dat; i++)
    {
        fvec[i] = mydata->user_y[i] - mydata->user_func( mydata->user_tx[i], mydata->user_ty[i], mydata->n_par, par);
    }
    *info = *info; /* to prevent a 'unused variable' warning */
    /* if <parameters drifted away> { *info = -1; } */
}

void lm_print_default( int n_par, double* par, int m_dat, double* fvec,
                       void *data, int iflag, int iter, int nfev )
/*
        *       data  : for soft control of printout behaviour, add control
        *                 variables to the data struct
        *       iflag : 0 (init) 1 (outer loop) 2(inner loop) -1(terminated)
        *       iter  : outer loop counter
        *       nfev  : number of calls to *evaluate
 */
{
    lm_data_type *mydata = (lm_data_type*)data;

    if(mydata->print_flag)
    {
        if (iflag==2) {
            printf ("trying step in gradient direction\n");
        } else if (iflag==1) {
            printf ("determining gradient (iteration %d)\n", iter);
        } else if (iflag==0) {
            printf ("starting minimization\n");
        } else if (iflag==-1) {
            printf ("terminated after %d evaluations\n", nfev);
        }

        printf( "  par: " );
        for(int i=0; i<n_par; ++i )
            printf( " %12g", par[i] );
        printf ( " => norm: %12g\n", lm_enorm( m_dat, fvec ) );

        if ( iflag == -1 ) {
            printf( "  fitting data as follows:\n" );
            for(int i=0; i<m_dat; ++i ) {
                const double tx = (mydata->user_tx)[i];
                const double ty = (mydata->user_ty)[i];
                const double y = (mydata->user_y)[i];
                const double f = mydata->user_func( tx, ty, mydata->n_par, par );
                printf( "    tx[%2d]=%8g     ty[%2d]=%8g     y=%12g fit=%12g     residue=%12g\n",
                        i, tx, i, ty, y, f, y-f );
            }
        }
    }       // if print_flag
}





///=================================================================================

/* *********************** high-level interface **************************** */


void lm_initialize_control( lm_control_type *control )
{
    control->maxcall = 100;
    control->epsilon = 1.e-10; //1.e-14;
    control->stepbound = 100; //100.;
    control->ftol = 1.e-14;
    control->xtol = 1.e-14;
    control->gtol = 1.e-14;
}

void lm_minimize( int m_dat, int n_par, double* par,
                  lm_evaluate_ftype *evaluate, lm_print_ftype *printout,
                  void *data, lm_control_type *control )
{
    std::vector<double> fvec(m_dat);
    std::vector<double> diag(n_par);
    std::vector<double> qtf(n_par);
    std::vector<double> fjac(n_par*m_dat);
    std::vector<double> wa1(n_par);
    std::vector<double> wa2(n_par);
    std::vector<double> wa3(n_par);
    std::vector<double> wa4(m_dat);
    std::vector<int> ipvt(n_par);

    // *** perform fit.

          control->info = 0;
          control->nfev = 0;

    // this goes through the modified legacy interface:
          lm_lmdif( m_dat, n_par, par, &fvec[0], control->ftol, control->xtol, control->gtol,
                    control->maxcall*(n_par+1), control->epsilon, &diag[0], 1,
                    control->stepbound, &(control->info),
                    &(control->nfev), &fjac[0], &ipvt[0], &qtf[0], &wa1[0], &wa2[0], &wa3[0], &wa4[0],
                    evaluate, printout, data );

          (*printout)( n_par, par, m_dat, &fvec[0], data, -1, 0, control->nfev );
          control->fnorm = lm_enorm(m_dat, &fvec[0]);
          if (control->info < 0 ) control->info = 10;
}


// ***** the following messages are referenced by the variable info.

const char *lm_infmsg[] = {
    "improper input parameters",
    "the relative error in the sum of squares is at most tol",
    "the relative error between x and the solution is at most tol",
    "both errors are at most tol",
    "fvec is orthogonal to the columns of the jacobian to machine precision",
    "number of calls to fcn has reached or exceeded 200*(n+1)",
    "ftol is too small. no further reduction in the sum of squares is possible",
    "xtol too small. no further improvement in approximate solution x possible",
    "gtol too small. no further improvement in approximate solution x possible",
    "not enough memory",
    "break requested within function evaluation"
};

const char *lm_shortmsg[] = {
    "invalid input",
    "success (f)",
    "success (p)",
    "success (f,p)",
    "degenerate",
    "call limit",
    "failed (f)",
    "failed (p)",
    "failed (o)",
    "no memory",
    "user break"
};


/* ************************** implementation ******************************* */

// the following values seem good for an x86:
//#define LM_MACHEP .555e-16 /* resolution of arithmetic */
//#define LM_DWARF  9.9e-324 /* smallest nonzero number */
// the follwoing values should work on any machine:
 #define LM_MACHEP 1.2e-16
 #define LM_DWARF 1.0e-38

// the squares of the following constants shall not under/overflow:
// these values seem good for an x86:
//#define LM_SQRT_DWARF 1.e-160
//#define LM_SQRT_GIANT 1.e150
// the following values should work on any machine:
 #define LM_SQRT_DWARF 3.834e-20
 #define LM_SQRT_GIANT 1.304e19


void lm_qrfac( int m, int n, double* a, int pivot, int* ipvt,
               double* rdiag, double* acnorm, double* wa);
void lm_qrsolv(int n, double* r, int ldr, int* ipvt, double* diag,
               double* qtb, double* x, double* sdiag, double* wa);
void lm_lmpar( int n, double* r, int ldr, int* ipvt, double* diag, double* qtb,
               double delta, double* par, double* x, double* sdiag,
               double* wa1, double* wa2);

#define MIN(a,b) (((a)<=(b)) ? (a) : (b))
#define MAX(a,b) (((a)>=(b)) ? (a) : (b))
#define SQR(x)   (x)*(x)


// ***** the low-level legacy interface for full control.

void lm_lmdif( int m, int n, double* x, double* fvec, double ftol, double xtol,
               double gtol, int maxfev, double epsfcn, double* diag, int mode,
               double factor, int *info, int *nfev,
               double* fjac, int* ipvt, double* qtf,
               double* wa1, double* wa2, double* wa3, double* wa4,
               lm_evaluate_ftype *evaluate, lm_print_ftype *printout,
               void *data )
{
    /*
    *   the purpose of lmdif is to minimize the sum of the squares of
    *   m nonlinear functions in n variables by a modification of
    *   the levenberg-marquardt algorithm. the user must provide a
    *   subroutine evaluate which calculates the functions. the jacobian
    *   is then calculated by a forward-difference approximation.
    *
    *   the multi-parameter interface lm_lmdif is for users who want
    *   full control and flexibility. most users will be better off using
    *   the simpler interface lmfit provided above.
    *
    *   the parameters are the same as in the legacy FORTRAN implementation,
    *   with the following exceptions:
    *      the old parameter ldfjac which gave leading dimension of fjac has
    *        been deleted because this C translation makes no use of two-
    *        dimensional arrays;
    *      the old parameter nprint has been deleted; printout is now controlled
    *        by the user-supplied routine *printout;
    *      the parameter field *data and the function parameters *evaluate and
    *        *printout have been added; they help avoiding global variables.
    *
    *   parameters:
    *
    *    m is a positive integer input variable set to the number
    *      of functions.
    *
    *    n is a positive integer input variable set to the number
    *      of variables. n must not exceed m.
    *
    *    x is an array of length n. on input x must contain
    *      an initial estimate of the solution vector. on output x
    *      contains the final estimate of the solution vector.
    *
    *    fvec is an output array of length m which contains
    *      the functions evaluated at the output x.
    *
    *    ftol is a nonnegative input variable. termination
    *      occurs when both the actual and predicted relative
    *      reductions in the sum of squares are at most ftol.
    *      therefore, ftol measures the relative error desired
    *      in the sum of squares.
    *
    *    xtol is a nonnegative input variable. termination
    *      occurs when the relative error between two consecutive
    *      iterates is at most xtol. therefore, xtol measures the
    *      relative error desired in the approximate solution.
    *
    *    gtol is a nonnegative input variable. termination
    *      occurs when the cosine of the angle between fvec and
    *      any column of the jacobian is at most gtol in absolute
    *      value. therefore, gtol measures the orthogonality
    *      desired between the function vector and the columns
    *      of the jacobian.
    *
    *    maxfev is a positive integer input variable. termination
    *      occurs when the number of calls to lm_fcn is at least
    *      maxfev by the end of an iteration.
    *
    *    epsfcn is an input variable used in determining a suitable
    *      step length for the forward-difference approximation. this
    *      approximation assumes that the relative errors in the
    *      functions are of the order of epsfcn. if epsfcn is less
    *      than the machine precision, it is assumed that the relative
    *      errors in the functions are of the order of the machine
    *      precision.
    *
    *    diag is an array of length n. if mode = 1 (see below), diag is
    *        internally set. if mode = 2, diag must contain positive entries
    *        that serve as multiplicative scale factors for the variables.
    *
    *    mode is an integer input variable. if mode = 1, the
    *      variables will be scaled internally. if mode = 2,
    *      the scaling is specified by the input diag. other
    *      values of mode are equivalent to mode = 1.
    *
    *    factor is a positive input variable used in determining the
    *      initial step bound. this bound is set to the product of
    *      factor and the euclidean norm of diag*x if nonzero, or else
    *      to factor itself. in most cases factor should lie in the
    *      interval (.1,100.). 100. is a generally recommended value.
    *
    *    info is an integer output variable that indicates the termination
    *        status of lm_lmdif as follows:
    *
    *        info < 0  termination requested by user-supplied routine *evaluate;
    *
    *      info = 0  improper input parameters;
    *
    *      info = 1  both actual and predicted relative reductions
    *              in the sum of squares are at most ftol;
    *
    *      info = 2  relative error between two consecutive iterates
    *              is at most xtol;
    *
    *      info = 3  conditions for info = 1 and info = 2 both hold;
    *
    *      info = 4  the cosine of the angle between fvec and any
    *              column of the jacobian is at most gtol in
    *              absolute value;
    *
    *      info = 5  number of calls to lm_fcn has reached or
    *              exceeded maxfev;
    *
    *      info = 6  ftol is too small. no further reduction in
    *              the sum of squares is possible;
    *
    *      info = 7  xtol is too small. no further improvement in
    *              the approximate solution x is possible;
    *
    *      info = 8  gtol is too small. fvec is orthogonal to the
    *              columns of the jacobian to machine precision;
    *
    *    nfev is an output variable set to the number of calls to the
    *        user-supplied routine *evaluate.
    *
    *    fjac is an output m by n array. the upper n by n submatrix
    *      of fjac contains an upper triangular matrix r with
    *      diagonal elements of nonincreasing magnitude such that
    *
    *           t     t       t
    *          p *(jac *jac)*p = r *r,
    *
    *      where p is a permutation matrix and jac is the final
    *      calculated jacobian. column j of p is column ipvt(j)
    *      (see below) of the identity matrix. the lower trapezoidal
    *      part of fjac contains information generated during
    *      the computation of r.
    *
    *    ipvt is an integer output array of length n. ipvt
    *      defines a permutation matrix p such that jac*p = q*r,
    *      where jac is the final calculated jacobian, q is
    *      orthogonal (not stored), and r is upper triangular
    *      with diagonal elements of nonincreasing magnitude.
    *      column j of p is column ipvt(j) of the identity matrix.
    *
    *    qtf is an output array of length n which contains
    *      the first n elements of the vector (q transpose)*fvec.
    *
    *    wa1, wa2, and wa3 are work arrays of length n.
    *
    *    wa4 is a work array of length m.
    *
    *   the following parameters are newly introduced in this C translation:
    *
    *      evaluate is the name of the subroutine which calculates the functions.
    *        a default implementation lm_evaluate_default is provided in lm_eval.c;
    *        alternatively, evaluate can be provided by a user calling program.
    *        it should be written as follows:
    *
    *        void evaluate ( double* par, int m_dat, double* fvec,
    *                       void *data, int *info )
    *        {
    *           // for ( i=0; i<m_dat; ++i )
    *           //     calculate fvec[i] for given parameters par;
    *           // to stop the minimization,
    *           //     set *info to a negative integer.
    *        }
    *
    *      printout is the name of the subroutine which nforms about fit progress.
    *        a default implementation lm_print_default is provided in lm_eval.c;
    *        alternatively, printout can be provided by a user calling program.
    *        it should be written as follows:
    *
    *        void printout ( int n_par, double* par, int m_dat, double* fvec,
    *                       void *data, int iflag, int iter, int nfev )
    *        {
    *           // iflag : 0 (init) 1 (outer loop) 2(inner loop) -1(terminated)
    *           // iter  : outer loop counter
    *           // nfev  : number of calls to *evaluate
    *        }
    *
    *      data is an input pointer to an arbitrary structure that is passed to
    *        evaluate. typically, it contains experimental data to be fitted.
    *
    */

    *nfev = 0; // function evaluation counter

    // *** check the input parameters for errors.

    if ( (n <= 0) || (m < n) || (ftol < 0.)
        || (xtol < 0.) || (gtol < 0.) || (maxfev <= 0) || (factor <= 0.) )
    {
        *info = 0; // invalid parameter
        return;
    }

    if ( mode == 2 )  /* scaling by diag[] */
    {
        for ( int j=0; j<n; j++ )  /* check for non positive elements */
        {
            if ( diag[j] <= 0.0 )
            {
                *info = 0; // invalid parameter
                return;
            }
        }
    }

    // *** evaluate the function at the starting point and calculate its norm.

    *info = 0;
    (*evaluate)( x, m, fvec, data, info );
    (*printout)( n, x, m, fvec, data, 0, 0, ++(*nfev) );
    if ( *info < 0 ) return;

    // *** the outer loop.
    int iter = 1;  // outer loop counter
    double delta = 0.0; // just to prevent a warning (initialization within if-clause)
    double xnorm = 0.0;
    double fnorm = lm_enorm(m,fvec);
    const double eps = sqrt(MAX(epsfcn,LM_MACHEP)); // used in calculating the Jacobian by forward differences

    do {
        // O** calculate the jacobian matrix.

        for ( int j=0; j<n; j++ )
        {
            const double temp = x[j];
            const double step = eps * fabs(temp) == 0.0 ? eps : eps * fabs(temp);

            x[j] = temp + step;
            *info = 0;
            (*evaluate)( x, m, wa4, data, info );
            (*printout)( n, x, m, wa4, data, 1, iter, ++(*nfev) );
            if ( *info < 0 ) return;  // user requested break
            x[j] = temp;
            for (int i=0; i<m; i++ )
                fjac[j*m+i] = (wa4[i] - fvec[i]) / step;
        }

        // O** compute the qr factorization of the jacobian.

        lm_qrfac( m, n, fjac, 1, ipvt, wa1, wa2, wa3);

        // O** on the first iteration ...

        if (iter == 1)
        {
            if (mode != 2)
                //      ... scale according to the norms of the columns of the initial jacobian.
            {
                for (int j=0; j<n; j++ )
                {
                    diag[j] = wa2[j];
                    if ( wa2[j] == 0. )
                        diag[j] = 1.;
                }
            }

            //      ... calculate the norm of the scaled x and
            //          initialize the step bound delta.

            for (int j=0; j<n; j++ )
                wa3[j] = diag[j] * x[j];

            xnorm = lm_enorm( n, wa3 );
            delta = factor*xnorm;
            if (delta == 0.)
                delta = factor;
        }

        // O** form (q transpose)*fvec and store the first n components in qtf.

        for (int i=0; i<m; i++ )
            wa4[i] = fvec[i];

        for (int j=0; j<n; j++ )
        {
            double temp3 = fjac[j*m+j];
            if (temp3 != 0.)
            {
                double sum = 0.0;
                for (int i=j; i<m; i++ )
                    sum += fjac[j*m+i] * wa4[i];
                double temp = -sum / temp3;
                for (int i=j; i<m; i++ )
                    wa4[i] += fjac[j*m+i] * temp;
            }
            fjac[j*m+j] = wa1[j];
            qtf[j] = wa4[j];
        }

        // O** compute the norm of the scaled gradient and test for convergence.

        double gnorm = 0;
        if ( fnorm != 0 )
        {
            for (int j=0; j<n; j++ )
            {
                if ( wa2[ ipvt[j] ] == 0 ) continue;

                double sum = 0.0;
                for (int i=0; i<=j; i++ )
                    sum += fjac[j*m+i] * qtf[i] / fnorm;
                gnorm = MAX( gnorm, fabs(sum/wa2[ ipvt[j] ]) );
            }
        }

        if ( gnorm <= gtol )
        {
            *info = 4;
            return;
        }

        // O** rescale if necessary.

        if ( mode != 2 )
        {
            for (int j=0; j<n; j++ )
                diag[j] = MAX(diag[j],wa2[j]);
        }

        // O** the inner loop.
        const double kP0001 = 1.0e-4;
        double ratio = 0.0;
        do {
            // OI* determine the levenberg-marquardt parameter.
            double par = 0;   // levenberg-marquardt parameter
            lm_lmpar( n,fjac,m,ipvt,diag,qtf,delta,&par,wa1,wa2,wa3,wa4 );

            // OI* store the direction p and x + p. calculate the norm of p.

            for (int j=0; j<n; j++ )
            {
                wa1[j] = -wa1[j];
                wa2[j] = x[j] + wa1[j];
                wa3[j] = diag[j]*wa1[j];
            }
            double pnorm = lm_enorm(n,wa3);

            // OI* on the first iteration, adjust the initial step bound.

            if ( *nfev<= 1+n ) // bug corrected by J. Wuttke in 2004
                delta = MIN(delta,pnorm);

            // OI* evaluate the function at x + p and calculate its norm.

            *info = 0;
            (*evaluate)( wa2, m, wa4, data, info );
            (*printout)( n, x, m, wa4, data, 2, iter, ++(*nfev) );
            if ( *info < 0 ) return;  // user requested break

            double fnorm1 = lm_enorm(m,wa4);

            // OI* compute the scaled actual reduction.
            const double kP1 = 0.1;
            const double actred = kP1 * fnorm1 < fnorm ? 1 - SQR( fnorm1/fnorm ) : -1;

            // OI* compute the scaled predicted reduction and
            //     the scaled directional derivative.

            for (int j=0; j<n; j++ )
            {
                wa3[j] = 0;
                for (int i=0; i<=j; i++ )
                    wa3[i] += fjac[j*m+i]*wa1[ ipvt[j] ];
            }
            const double temp1 = lm_enorm(n,wa3) / fnorm;
            const double temp2 = sqrt(par) * pnorm / fnorm;
            const double prered = SQR(temp1) + 2 * SQR(temp2);
            const double dirder = - ( SQR(temp1) + SQR(temp2) );

            // OI* compute the ratio of the actual to the predicted reduction.

            ratio = prered !=0 ? actred / prered : 0.0;

            // OI* update the step bound.
            const double kP5 = 0.5;
            const double kP25 = 0.25;
            const double kP75 = 0.75;

            if (ratio <= kP25)
            {
                double temp = actred >= 0.0 ? kP5 : kP5*dirder/(dirder + kP5*actred);
                if ( kP1*fnorm1 >= fnorm || temp < kP1 )
                    temp = kP1;
                delta = temp * MIN(delta,pnorm/kP1);
                par /= temp;
            }
            else if ( par == 0. || ratio >= kP75 )
            {
                delta = pnorm/kP5;
                par *= kP5;
            }

            // OI* test for successful iteration...
            if (ratio >= kP0001)
            {

                //     ... successful iteration. update x, fvec, and their norms.

                for (int j=0; j<n; j++ )
                {
                    x[j] = wa2[j];
                    wa2[j] = diag[j]*x[j];
                }
                for (int i=0; i<m; i++ )
                    fvec[i] = wa4[i];
                xnorm = lm_enorm(n,wa2);
                fnorm = fnorm1;
                iter++;
            }

            // OI* tests for convergence ( otherwise *info = 1, 2, or 3 )

            *info = 0; // do not terminate (unless overwritten by nonzero value)
            if ( fabs(actred) <= ftol && prered <= ftol && kP5*ratio <= 1 )
                *info = 1;
            if (delta <= xtol*xnorm)
                *info += 2;
            if ( *info != 0)
                return;

            // OI* tests for termination and stringent tolerances.

            if ( *nfev >= maxfev)
                *info = 5;
            if ( fabs(actred) <= LM_MACHEP &&
                prered <= LM_MACHEP && kP5*ratio <= 1 )
                *info = 6;
            if (delta <= LM_MACHEP*xnorm)
                *info = 7;
            if (gnorm <= LM_MACHEP)
                *info = 8;
            if ( *info != 0)
                return;

            // OI* end of the inner loop. repeat if iteration unsuccessful.

        } while (ratio < kP0001);

        // O** end of the outer loop.

    } while (1);

}



void lm_lmpar(int n, double* r, int ldr, int* ipvt, double* diag, double* qtb,
              double delta, double* par, double* x, double* sdiag,
              double* wa1, double* wa2)
{
    /*     given an m by n matrix a, an n by n nonsingular diagonal
    *     matrix d, an m-vector b, and a positive number delta,
    *     the problem is to determine a value for the parameter
    *     par such that if x solves the system
    *
    *        a*x = b ,       sqrt(par)*d*x = 0 ,
    *
    *     in the least squares sense, and dxnorm is the euclidean
    *     norm of d*x, then either par is 0. and
    *
    *        (dxnorm-delta) .le. 0.1*delta ,
    *
    *     or par is positive and
    *
    *        abs(dxnorm-delta) .le. 0.1*delta .
    *
    *     this subroutine completes the solution of the problem
    *     if it is provided with the necessary information from the
    *     qr factorization, with column pivoting, of a. that is, if
    *     a*p = q*r, where p is a permutation matrix, q has orthogonal
    *     columns, and r is an upper triangular matrix with diagonal
    *     elements of nonincreasing magnitude, then lmpar expects
    *     the full upper triangle of r, the permutation matrix p,
    *     and the first n components of (q transpose)*b. on output
    *     lmpar also provides an upper triangular matrix s such that
    *
    *         t       t               t
    *        p *(a *a + par*d*d)*p = s *s .
    *
    *     s is employed within lmpar and may be of separate interest.
    *
    *     only a few iterations are generally needed for convergence
    *     of the algorithm. if, however, the limit of 10 iterations
    *     is reached, then the output par will contain the best
    *     value obtained so far.
    *
    *     parameters:
    *
    *    n is a positive integer input variable set to the order of r.
    *
    *    r is an n by n array. on input the full upper triangle
    *      must contain the full upper triangle of the matrix r.
    *      on output the full upper triangle is unaltered, and the
    *      strict lower triangle contains the strict upper triangle
    *      (transposed) of the upper triangular matrix s.
    *
    *    ldr is a positive integer input variable not less than n
    *      which specifies the leading dimension of the array r.
    *
    *    ipvt is an integer input array of length n which defines the
    *      permutation matrix p such that a*p = q*r. column j of p
    *      is column ipvt(j) of the identity matrix.
    *
    *    diag is an input array of length n which must contain the
    *      diagonal elements of the matrix d.
    *
    *    qtb is an input array of length n which must contain the first
    *      n elements of the vector (q transpose)*b.
    *
    *    delta is a positive input variable which specifies an upper
    *      bound on the euclidean norm of d*x.
    *
    *    par is a nonnegative variable. on input par contains an
    *      initial estimate of the levenberg-marquardt parameter.
    *      on output par contains the final estimate.
    *
    *    x is an output array of length n which contains the least
    *      squares solution of the system a*x = b, sqrt(par)*d*x = 0,
    *      for the output par.
    *
    *    sdiag is an output array of length n which contains the
    *      diagonal elements of the upper triangular matrix s.
    *
    *    wa1 and wa2 are work arrays of length n.
    *
    */

    // *** compute and store in x the gauss-newton direction. if the
    //     jacobian is rank-deficient, obtain a least squares solution.

    int nsing = n;
    for (int j=0; j<n; j++ )
    {
        wa1[j] = qtb[j];
        if ( r[j*ldr+j] == 0 && nsing == n )
            nsing = j;
        if (nsing < n)
            wa1[j] = 0;
    }
    for (int j=nsing-1; j>=0; j-- )
    {
        wa1[j] = wa1[j]/r[j+ldr*j];
        const double temp = wa1[j];
        for (int i=0; i<j; i++ )
            wa1[i] -= r[j*ldr+i]*temp;
    }

    for (int j=0; j<n; j++ )
        x[ ipvt[j] ] = wa1[j];

    // *** initialize the iteration counter.
    //     evaluate the function at the origin, and test
    //     for acceptance of the gauss-newton direction.

    int iter = 0;
    for (int j=0; j<n; j++ )
        wa2[j] = diag[j]*x[j];

    double dxnorm = lm_enorm(n,wa2);
    double fp = dxnorm - delta;
    const double kP1 = 0.1;
    if (fp <= kP1*delta)
    {
        *par = 0;
        return;
    }

    // *** if the jacobian is not rank deficient, the newton
    //     step provides a lower bound, parl, for the 0. of
    //     the function. otherwise set this bound to 0..

    double parl = 0.0;
    if (nsing >= n)
    {
        for (int j=0; j<n; j++ )
            wa1[j] = diag[ ipvt[j] ] * wa2[ ipvt[j] ] / dxnorm;

        for (int j=0; j<n; j++ )
        {
            double sum = 0.0;
            for (int i=0; i<j; i++ )
                sum += r[j*ldr+i]*wa1[i];
            wa1[j] = (wa1[j] - sum)/r[j+ldr*j];
        }
        const double temp = lm_enorm(n,wa1);
        parl = fp/delta/temp/temp;
    }

    // *** calculate an upper bound, paru, for the 0. of the function.

    for (int j=0; j<n; j++ )
    {
        double sum = 0;
        for (int i=0; i<=j; i++ )
            sum += r[j*ldr+i]*qtb[i];
        wa1[j] = sum/diag[ ipvt[j] ];
    }
    const double gnorm = lm_enorm(n,wa1);
    double paru = gnorm/delta == 0.0 ? LM_DWARF/MIN(delta,kP1) : gnorm / delta;

    // *** if the input par lies outside of the interval (parl,paru),
    //     set par to the closer endpoint.

    *par = MAX( *par,parl);
    *par = MIN( *par,paru);
    if ( *par == 0.)
        *par = gnorm/dxnorm;

    // *** iterate.

    for ( ; ; iter++ ) {

        // *** evaluate the function at the current value of par.
        const double kP001 = 0.001;
        if ( *par == 0.)
            *par = MAX(LM_DWARF,kP001*paru);
        double temp = sqrt( *par );
        for (int j=0; j<n; j++ )
            wa1[j] = temp*diag[j];
        lm_qrsolv( n, r, ldr, ipvt, wa1, qtb, x, sdiag, wa2);
        for (int j=0; j<n; j++ )
            wa2[j] = diag[j]*x[j];
        dxnorm = lm_enorm(n,wa2);
        const double fp_old = fp;
        fp = dxnorm - delta;

        // ***       if the function is small enough, accept the current value
        //     of par. also test for the exceptional cases where parl
        //     is 0. or the number of iterations has reached 10.

        if ( fabs(fp) <= kP1*delta
            || (parl == 0. && fp <= fp_old && fp_old < 0.)
            || iter == 10 )
            break; // the only exit from this loop

        // *** compute the Newton correction.

        for (int j=0; j<n; j++ )
            wa1[j] = diag[ ipvt[j] ] * wa2[ ipvt[j] ] / dxnorm;

        for (int j=0; j<n; j++ )
        {
            wa1[j] = wa1[j]/sdiag[j];
            for (int i=j+1; i<n; i++ )
                wa1[i] -= r[j*ldr+i]*wa1[j];
        }
        temp = lm_enorm( n, wa1);
        double parc = fp/delta/temp/temp;

        // *** depending on the sign of the function, update parl or paru.

        if (fp > 0)
            parl = MAX(parl, *par);
        else if (fp < 0)
            paru = MIN(paru, *par);
        // the case fp==0 is precluded by the break condition

        // *** compute an improved estimate for par.

        *par = MAX(parl, *par + parc);

    }

}



void lm_qrfac(int m, int n, double* a, int pivot, int* ipvt,
              double* rdiag, double* acnorm, double* wa)
{
    /*
    *     this subroutine uses householder transformations with column
    *     pivoting (optional) to compute a qr factorization of the
    *     m by n matrix a. that is, qrfac determines an orthogonal
    *     matrix q, a permutation matrix p, and an upper trapezoidal
    *     matrix r with diagonal elements of nonincreasing magnitude,
    *     such that a*p = q*r. the householder transformation for
    *     column k, k = 1,2,...,min(m,n), is of the form
    *
    *                    t
    *        i - (1/u(k))*u*u
    *
    *     where u has 0.s in the first k-1 positions. the form of
    *     this transformation and the method of pivoting first
    *     appeared in the corresponding linpack subroutine.
    *
    *     parameters:
    *
    *    m is a positive integer input variable set to the number
    *      of rows of a.
    *
    *    n is a positive integer input variable set to the number
    *      of columns of a.
    *
    *    a is an m by n array. on input a contains the matrix for
    *      which the qr factorization is to be computed. on output
    *      the strict upper trapezoidal part of a contains the strict
    *      upper trapezoidal part of r, and the lower trapezoidal
    *      part of a contains a factored form of q (the non-trivial
    *      elements of the u vectors described above).
    *
    *    pivot is a logical input variable. if pivot is set true,
    *      then column pivoting is enforced. if pivot is set false,
    *      then no column pivoting is done.
    *
    *    ipvt is an integer output array of length lipvt. ipvt
    *      defines the permutation matrix p such that a*p = q*r.
    *      column j of p is column ipvt(j) of the identity matrix.
    *      if pivot is false, ipvt is not referenced.
    *
    *    rdiag is an output array of length n which contains the
    *      diagonal elements of r.
    *
    *    acnorm is an output array of length n which contains the
    *      norms of the corresponding columns of the input matrix a.
    *      if this information is not needed, then acnorm can coincide
    *      with rdiag.
    *
    *    wa is a work array of length n. if pivot is false, then wa
    *      can coincide with rdiag.
    *
    */

    // *** compute the initial column norms and initialize several arrays.

    for (int j=0; j<n; j++ )
    {
        acnorm[j] = lm_enorm(m, &a[j*m]);
        rdiag[j] = acnorm[j];
        wa[j] = rdiag[j];
        if ( pivot )
            ipvt[j] = j;
    }

    // *** reduce a to r with householder transformations.

    const int minmn = MIN(m,n);
    for (int j=0; j<minmn; j++ )
    {
        int kmax = j, k;
        if ( !pivot ) goto pivot_ok;

        // *** bring the column of largest norm into the pivot position.

        for (k=j+1; k<n; k++ )
            if (rdiag[k] > rdiag[kmax])
                kmax = k;
        if (kmax == j) goto pivot_ok; // bug fixed in rel 2.1

        for (int i=0; i<m; i++ )
        {
            std::swap(a[j*m+i], a[kmax*m+i]);
            //const double temp = a[j*m+i];
            //a[j*m+i]    = a[kmax*m+i];
            //a[kmax*m+i] = temp;
        }
        rdiag[kmax] = rdiag[j];
        wa[kmax] = wa[j];
        std::swap(ipvt[j], ipvt[kmax]);
        //k = ipvt[j];
        //ipvt[j] = ipvt[kmax];
        //ipvt[kmax] = k;

pivot_ok:

        // *** compute the Householder transformation to reduce the
        //     j-th column of a to a multiple of the j-th unit vector.

        double ajnorm = lm_enorm( m-j, &a[j*m+j] );
        if (ajnorm == 0.)
        {
            rdiag[j] = 0;
            continue;
        }

        if (a[j*m+j] < 0.)
            ajnorm = -ajnorm;
        for (int i=j; i<m; i++ )
            a[j*m+i] /= ajnorm;
        a[j*m+j] += 1;

        // *** apply the transformation to the remaining columns
        //     and update the norms.

        for ( k=j+1; k<n; k++ )
        {
            double sum = 0;

            for (int i=j; i<m; i++ )
                sum += a[j*m+i]*a[k*m+i];

            double temp = sum/a[j+m*j];

            for (int i=j; i<m; i++ )
                a[k*m+i] -= temp * a[j*m+i];

            if ( pivot && rdiag[k] != 0. )
            {
                temp = a[m*k+j]/rdiag[k];
                temp = MAX( 0., 1-temp*temp );
                rdiag[k] *= sqrt(temp);
                temp = rdiag[k]/wa[k];
                const double kP05 = 0.05;
                if ( kP05*SQR(temp) <= LM_MACHEP )
                {
                    rdiag[k] = lm_enorm( m-j-1, &a[m*k+j+1]);
                    wa[k] = rdiag[k];
                }
            }
        }

        rdiag[j] = -ajnorm;
    }
}



void lm_qrsolv(int n, double* r, int ldr, int* ipvt, double* diag,
               double* qtb, double* x, double* sdiag, double* wa)
{
    /*
    *     given an m by n matrix a, an n by n diagonal matrix d,
    *     and an m-vector b, the problem is to determine an x which
    *     solves the system
    *
    *        a*x = b ,       d*x = 0 ,
    *
    *     in the least squares sense.
    *
    *     this subroutine completes the solution of the problem
    *     if it is provided with the necessary information from the
    *     qr factorization, with column pivoting, of a. that is, if
    *     a*p = q*r, where p is a permutation matrix, q has orthogonal
    *     columns, and r is an upper triangular matrix with diagonal
    *     elements of nonincreasing magnitude, then qrsolv expects
    *     the full upper triangle of r, the permutation matrix p,
    *     and the first n components of (q transpose)*b. the system
    *     a*x = b, d*x = 0, is then equivalent to
    *
    *             t     t
    *        r*z = q *b ,  p *d*p*z = 0 ,
    *
    *     where x = p*z. if this system does not have full rank,
    *     then a least squares solution is obtained. on output qrsolv
    *     also provides an upper triangular matrix s such that
    *
    *         t       t           t
    *        p *(a *a + d*d)*p = s *s .
    *
    *     s is computed within qrsolv and may be of separate interest.
    *
    *     parameters
    *
    *    n is a positive integer input variable set to the order of r.
    *
    *    r is an n by n array. on input the full upper triangle
    *      must contain the full upper triangle of the matrix r.
    *      on output the full upper triangle is unaltered, and the
    *      strict lower triangle contains the strict upper triangle
    *      (transposed) of the upper triangular matrix s.
    *
    *    ldr is a positive integer input variable not less than n
    *      which specifies the leading dimension of the array r.
    *
    *    ipvt is an integer input array of length n which defines the
    *      permutation matrix p such that a*p = q*r. column j of p
    *      is column ipvt(j) of the identity matrix.
    *
    *    diag is an input array of length n which must contain the
    *      diagonal elements of the matrix d.
    *
    *    qtb is an input array of length n which must contain the first
    *      n elements of the vector (q transpose)*b.
    *
    *    x is an output array of length n which contains the least
    *      squares solution of the system a*x = b, d*x = 0.
    *
    *    sdiag is an output array of length n which contains the
    *      diagonal elements of the upper triangular matrix s.
    *
    *    wa is a work array of length n.
    *
    */

    // *** copy r and (q transpose)*b to preserve input and initialize s.
    //     in particular, save the diagonal elements of r in x.

    for (int j=0; j<n; j++ )
    {
        for (int i=j; i<n; i++ )
            r[j*ldr+i] = r[i*ldr+j];
        x[j] = r[j*ldr+j];
        wa[j] = qtb[j];
    }

    // *** eliminate the diagonal matrix d using a givens rotation.

    for (int j=0; j<n; j++ )
    {

        // ***       prepare the row of d to be eliminated, locating the
        //     diagonal element using p from the qr factorization.

        double qtbpj = 0.0;

        if (diag[ ipvt[j] ] == 0.)
            goto L90;
        for (int k=j; k<n; k++ )
            sdiag[k] = 0.;
        sdiag[j] = diag[ ipvt[j] ];

        // ***       the transformations to eliminate the row of d
        //     modify only a single element of (q transpose)*b
        //     beyond the first n, which is initially 0..

        for (int k=j; k<n; k++ )
        {
            const double p25 = 0.25;
            const double p5 = 0.5;

            //        determine a givens rotation which eliminates the
            //        appropriate element in the current row of d.

            if (sdiag[k] == 0.)
                continue;
            const int kk = k + ldr * k; // <! keep this shorthand !>
            double sin, cos; // these are local variables, not functions

            if ( fabs(r[kk]) < fabs(sdiag[k]) )
            {
                const double cotan = r[kk]/sdiag[k];
                sin = p5/sqrt(p25+p25*SQR(cotan));
                cos = sin*cotan;
            }
            else
            {
                const double tan = sdiag[k]/r[kk];
                cos = p5/sqrt(p25+p25*SQR(tan));
                sin = cos*tan;
            }

            // ***          compute the modified diagonal element of r and
            //        the modified element of ((q transpose)*b,0).

            r[kk] = cos*r[kk] + sin*sdiag[k];
            double temp = cos*wa[k] + sin*qtbpj;
            qtbpj = -sin*wa[k] + cos*qtbpj;
            wa[k] = temp;

            // *** accumulate the transformation in the row of s.

            for (int i=k+1; i<n; i++ )
            {
                temp = cos*r[k*ldr+i] + sin*sdiag[i];
                sdiag[i] = -sin*r[k*ldr+i] + cos*sdiag[i];
                r[k*ldr+i] = temp;
            }
        }
L90:

        // *** store the diagonal element of s and restore
        //     the corresponding diagonal element of r.

        sdiag[j] = r[j*ldr+j];
        r[j*ldr+j] = x[j];
    }

    // *** solve the triangular system for z. if the system is
    //     singular, then obtain a least squares solution.

    int nsing = n;
    for (int j=0; j<n; j++ )
    {
        if ( sdiag[j] == 0. && nsing == n )
            nsing = j;
        if (nsing < n)
            wa[j] = 0;
    }

    for (int j=nsing-1; j>=0; j-- )
    {
        double sum = 0.0;
        for (int i=j+1; i<nsing; i++ )
            sum += r[j*ldr+i]*wa[i];
        wa[j] = (wa[j] - sum)/sdiag[j];
    }

    // *** permute the components of z back to components of x.

    for (int j=0; j<n; j++ )
        x[ ipvt[j] ] = wa[j];
}



double lm_enorm( int n, double *x )
{
    /*     given an n-vector x, this function calculates the
    *     euclidean norm of x.
    *
    *     the euclidean norm is computed by accumulating the sum of
    *     squares in three different sums. the sums of squares for the
    *     small and large components are scaled so that no overflows
    *     occur. non-destructive underflows are permitted. underflows
    *     and overflows do not occur in the computation of the unscaled
    *     sum of squares for the intermediate components.
    *     the definitions of small, intermediate and large components
    *     depend on two constants, LM_SQRT_DWARF and LM_SQRT_GIANT. the main
    *     restrictions on these constants are that LM_SQRT_DWARF**2 not
    *     underflow and LM_SQRT_GIANT**2 not overflow.
    *
    *     parameters
    *
    *    n is a positive integer input variable.
    *
    *    x is an input array of length n.
    */

    double s1 = 0.0, s2 = 0.0, s3 = 0.0;
    double x1max = 0.0, x3max = 0.0;
    const double agiant = LM_SQRT_GIANT/( (double) n);

    for (int i=0; i<n; i++ )
    {
        double xabs = fabs(x[i]);
        if ( xabs > LM_SQRT_DWARF && xabs < agiant )
        {
            // **  sum for intermediate components.
            s2 += xabs*xabs;
            continue;
        }

        if ( xabs >  LM_SQRT_DWARF )
        {
            // **  sum for large components.
            if (xabs > x1max)
            {
                s1 = 1 + s1*SQR(x1max/xabs);
                x1max = xabs;
            }
            else
            {
                s1 += SQR(xabs/x1max);
            }
            continue;
        }
        // **  sum for small components.
        if (xabs > x3max)
        {
            s3 = 1 + s3*SQR(x3max/xabs);
            x3max = xabs;
        }
        else
        {
            if (xabs != 0.)
            {
                s3 += SQR(xabs/x3max);
            }
        }
    }

    // *** calculation of norm.

    if (s1 != 0)
        return x1max*sqrt(s1 + (s2/x1max)/x1max);
    if (s2 != 0)
    {
        if (s2 >= x3max)
            return sqrt( s2*(1+(x3max/s2)*(x3max*s3)) );
        return sqrt( x3max*((s2/x3max)+(x3max*s3)) );
    }

    return x3max*sqrt(s3);
}

}

