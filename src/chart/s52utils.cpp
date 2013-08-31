/**************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   Copyright (C) 2000-2004  Sylvain Duclos                               *
 *   sduclos@users.sourceforgue.net                                        *
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

#include "s52utils.h"

#define TRUE 1
#define FALSE 0


// WARNING: must be in sync with _MARparamNm
static double _MARparamVal[] =
{
	0.0,      // NONE
	TRUE,     // SHOW_TEXT
	TRUE,     // TWO_SHADES

	8.0,     // SAFETY_CONTOUR
	//0.0,     // SAFETY_CONTOUR  --to test DEPCNT02 selection (GL) in CA49995A.000
	//0.5,     // SAFETY_CONTOUR  --to test DEPCNT02 selection (GL) in CA49995A.000

	5.0,      // SAFETY_DEPTH
	//5.0,      // SAFETY_DEPTH

	3.0,      // SHALLOW_CONTOUR
	10.0,     // DEEP_CONTOUR

	//FALSE,    // SHALLOW_PATTERN
	TRUE,    // SHALLOW_PATTERN

	FALSE,    // SHIPS_OUTLINE
	0.0,      // DISTANCE_TAGS
	0.0,      // TIME_TAGS
	TRUE,     // FULL_SECTORS
	TRUE,     // SYMBOLIZED_BND

	TRUE,     // SYMPLIFIED_PNT

	//    'D',      // S52_MAR_DISP_CATEGORY --DISPLAYBASE
	//    'S',      // S52_MAR_DISP_CATEGORY --STANDARD
	'O',      // S52_MAR_DISP_CATEGORY --OTHER

	//    0,        // S52_MAR_COLOR_PALETTE --DAY_BRIGHT
	1,        // S52_MAR_COLOR_PALETTE --DAY_BLACKBACK
	//    2,        // S52_MAR_COLOR_PALETTE --DAY_WHITEBACK
	//    3,        // S52_MAR_COLOR_PALETTE --DUSK
	//    4,        // S52_MAR_COLOR_PALETTE --NIGHT

	16.0      // NUM
};


// return Mariner parameter or '0.0' if fail
// FIXME: check mariner param against groups selection
double S52_getMarinerParam(S52_MAR_param_t param)
{
	return _MARparamVal[param];
}

int S52_setMarinerParam(S52_MAR_param_t param, double val)
{
	if (S52_MAR_NONE<param && param<S52_MAR_NUM)
		_MARparamVal[param] = val;
	else
		return FALSE;

	return TRUE;
}

