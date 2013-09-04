/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef __CUTIL_H__
#define __CUTIL_H__

struct MyPoint
{
	double x;
	double y;
};

struct float_2Dpt
{
	float y;
	float x;
};

int G_PtInPolygon(MyPoint *, int, float, float);
int G_PtInPolygon_FL(float_2Dpt *, int, float, float);

inline int roundint(double x)
{
	int tmp = static_cast<int>(x);
	tmp += (x - tmp >= 0.5) - (x - tmp <= -0.5);
	return tmp;
}


//-------------------------------------------------------------------------------------------------------
//  Cohen & Sutherland Line clipping algorithms
//-------------------------------------------------------------------------------------------------------
/*
 *
 * Copyright (C) 1999,2000,2001,2002,2003 Percy Zahl
 *
 * Authors: Percy Zahl <zahl@users.sf.net>
 * additional features: Andreas Klust <klust@users.sf.net>
 * WWW Home: http://gxsm.sf.net
 *
 */

enum ClipResult { Visible, Invisible };

ClipResult cohen_sutherland_line_clip_d(
		double * x0,
		double * y0,
		double * x1,
		double * y1,
		double xmin_,
		double xmax_,
		double ymin_,
		double ymax_);

ClipResult cohen_sutherland_line_clip_i(
		int * x0,
		int * y0,
		int * x1,
		int * y1,
		int xmin_,
		int xmax_,
		int ymin_,
		int ymax_);

#endif
