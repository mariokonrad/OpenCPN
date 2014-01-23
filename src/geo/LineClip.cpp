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

#include "LineClip.h"

namespace geo {

enum edge {
	LEFT,
	RIGHT,
	BOTTOM,
	TOP
};

// Local variables for cohen_sutherland_line_clip:
struct LOC_cohen_sutherland_line_clip
{
	double xmin;
	double xmax;
	double ymin;
	double ymax;
};

static long CompOutCode(double x, double y, struct LOC_cohen_sutherland_line_clip* LINK)
{
	// Compute outcode for the point (x,y)
	long code = 0;

	if (y > LINK->ymax)
		code = 1L << ((long)TOP);
	else if (y < LINK->ymin)
		code = 1L << ((long)BOTTOM);
	if (x > LINK->xmax)
		code |= 1L << ((long)RIGHT);
	else if (x < LINK->xmin)
		code |= 1L << ((long)LEFT);

	return code;
}

ClipResult cohen_sutherland_line_clip_d(double* x0, double* y0, double* x1, double* y1,
										double xmin_, double xmax_, double ymin_, double ymax_)
{
	// Cohen-Sutherland clipping algorithm for line P0=(x1,y0) to P1=(x1,y1)
	// and clip rectangle with diagonal from (xmin,ymin) to (xmax,ymax).

	struct LOC_cohen_sutherland_line_clip V;
	bool accept = false;
	bool done = false;
	ClipResult clip = Visible;
	long outcode0;
	long outcode1;
	long outcodeOut;
	// Outcodes for P0,P1, and whichever point lies outside the clip rectangle
	double x = 0.0;
	double y = 0.0;

	V.xmin = xmin_;
	V.xmax = xmax_;
	V.ymin = ymin_;
	V.ymax = ymax_;
	outcode0 = CompOutCode(*x0, *y0, &V);
	outcode1 = CompOutCode(*x1, *y1, &V);
	do {
		if (outcode0 == 0 && outcode1 == 0) { // Trivial accept and exit
			accept = true;
			done = true;
		} else if ((outcode0 & outcode1) != 0) {
			clip = Invisible;
			done = true;
		} else { // Logical intersection is true, so trivial reject and exit.
			clip = Visible;
			// Failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge.
			// At least one endpoint is outside the clip rectangle; pick it.
			if (outcode0 != 0)
				outcodeOut = outcode0;
			else
				outcodeOut = outcode1;
			// Now find intersection point;
			// use formulas y=y0+slope*(x-x0),x=x0+(1/slope)*(y-y0).

			if (((1L << ((long)TOP)) & outcodeOut) != 0) {
				// Divide line at top of clip rectangle
				x = *x0 + (*x1 - *x0) * (V.ymax - *y0) / (*y1 - *y0);
				y = V.ymax;
			} else if (((1L << ((long)BOTTOM)) & outcodeOut) != 0) {
				// Divide line at bottom of clip rectangle
				x = *x0 + (*x1 - *x0) * (V.ymin - *y0) / (*y1 - *y0);
				y = V.ymin;
			} else if (((1L << ((long)RIGHT)) & outcodeOut) != 0) {
				// Divide line at right edge of clip rectangle
				y = *y0 + (*y1 - *y0) * (V.xmax - *x0) / (*x1 - *x0);
				x = V.xmax;
			} else if (((1L << ((long)LEFT)) & outcodeOut) != 0) {
				// Divide line at left edge of clip rectangle
				y = *y0 + (*y1 - *y0) * (V.xmin - *x0) / (*x1 - *x0);
				x = V.xmin;
			}
			// Now we move outside point to intersection point to clip,
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				*x0 = x;
				*y0 = y;
				outcode0 = CompOutCode(*x0, *y0, &V);
			} else {
				*x1 = x;
				*y1 = y;
				outcode1 = CompOutCode(*x1, *y1, &V);
			}
		}
	} while (!done);
	return clip;
}

ClipResult cohen_sutherland_line_clip_i(int* x0_, int* y0_, int* x1_, int* y1_, int xmin_,
										int xmax_, int ymin_, int ymax_)
{
	ClipResult ret;
	double x0 = *x0_;
	double y0 = *y0_;
	double x1 = *x1_;
	double y1 = *y1_;

	ret = cohen_sutherland_line_clip_d(&x0, &y0, &x1, &y1, (double)xmin_, (double)xmax_,
									   (double)ymin_, (double)ymax_);

	*x0_ = (int)x0;
	*y0_ = (int)y0;
	*x1_ = (int)x1;
	*y1_ = (int)y1;
	return ret;
}

}

