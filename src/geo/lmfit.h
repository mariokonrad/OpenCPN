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

#ifndef __GEO__LMFIT__HPP__
#define __GEO__LMFIT__HPP__

namespace geo {

/*
 * lmfit
 *
 * Solves or minimizes the sum of squares of m nonlinear
 * functions of n variables.
 *
 * From public domain Fortran version
 * of Argonne National Laboratories MINPACK
 *     argonne national laboratory. minpack project. march 1980.
 *     burton s. garbow, kenneth e. hillstrom, jorge j. more
 * C translation by Steve Moshier
 * Joachim Wuttke converted the source into C++ compatible ANSI style
 * and provided a simplified interface
 */

// parameters for calling the high-level interface lmfit
//   ( lmfit.c provides lm_initialize_control which sets default values ):
typedef struct {
	double ftol;       // relative error desired in the sum of squares.
	double xtol;       // relative error between last two approximations.
	double gtol;       // orthogonality desired between fvec and its derivs.
	double epsilon;    // step used to calculate the jacobian.
	double stepbound;  // initial bound to steps in the outer loop.
	double fnorm;      // norm of the residue vector fvec.
	int maxcall;       // maximum number of iterations.
	int nfev;          // actual number of iterations.
	int info;          // status of minimization.
} lm_control_type;


// the subroutine that calculates fvec:
typedef void (lm_evaluate_ftype) (
		double* par, int m_dat, double* fvec, void *data, int *info );
// default implementation therof, provided by lm_eval.c:
void lm_evaluate_default (
		double* par, int m_dat, double* fvec, void *data, int *info );

// the subroutine that informs about fit progress:
typedef void (lm_print_ftype) (
		int n_par, double* par, int m_dat, double* fvec, void *data,
		int iflag, int iter, int nfev );
// default implementation therof, provided by lm_eval.c:
void lm_print_default (
		int n_par, double* par, int m_dat, double* fvec, void *data,
		int iflag, int iter, int nfev );

// compact high-level interface:
void lm_initialize_control( lm_control_type *control );
void lm_minimize ( int m_dat, int n_par, double* par,
		lm_evaluate_ftype *evaluate, lm_print_ftype *printout,
		void *data, lm_control_type *control );
double lm_enorm( int, double* );

// low-level interface for full control:
void lm_lmdif( int m, int n, double* x, double* fvec, double ftol, double xtol,
		double gtol, int maxfev, double epsfcn, double* diag, int mode,
		double factor, int *info, int *nfev,
		double* fjac, int* ipvt, double* qtf,
		double* wa1, double* wa2, double* wa3, double* wa4,
		lm_evaluate_ftype *evaluate, lm_print_ftype *printout,
		void *data );


#ifndef _LMDIF
extern const char *lm_infmsg[];
extern const char *lm_shortmsg[];
#endif

// This is an opaque (to lmfit) structure set up before the call to lmfit()
typedef struct
{
	double* user_tx;
	double* user_ty;
	double* user_y;
	double (*user_func)( double user_tx_point, double user_ty_point, int n_par, double* par );
	int     print_flag;
	int     n_par;
} lm_data_type;

}

#endif
