/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GSHHS Chart Object (Global Self-consistent, Hierarchical, High-resolution Shoreline)
 * Author:   Jesper Weissglas for the OpenCPN port.
 *
 *           Derived from http://www.zygrib.org/ and http://sourceforge.net/projects/qtvlm/
 *           which has the original copyright:
 *   zUGrib: meteorologic GRIB file data viewer
 *   Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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

#include "gshhs.h"
#include <chart/gshhs/QLineF.h>
#include <chart/gshhs/GshhsReader.h>
#include <chart/gshhs/Projection.h>

// so plugins can determine if a line segment crosses land
bool gshhsCrossesLand(double lat1, double lon1, double lat2, double lon2)
{
	static GshhsReader * reader = NULL;
	static Projection proj;

	if(reader == NULL)
		reader = new GshhsReader(&proj);

	while (lon1 < 0)
		lon1 += 360;
	while (lon2 < 0)
		lon2 += 360;

	return reader->crossing1(QLineF(lon1, lat1, lon2, lat2));
}

