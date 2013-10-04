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

#ifndef __NAVUTIL__
#define __NAVUTIL__

#include <wx/string.h>

enum DistanceUnit
{
	DISTANCE_NONE = -1,
	DISTANCE_NMI = 0,
	DISTANCE_MI,
	DISTANCE_KM,
	DISTANCE_M
};

enum SpeedUnit
{
	SPEED_NONE = -1,
	SPEED_KTS = 0,
	SPEED_MPH,
	SPEED_KMH,
	SPEED_MS
};

double toUsrDistance(double nm_distance, DistanceUnit unit = DISTANCE_NONE);
double fromUsrDistance(double usr_distance, DistanceUnit unit = DISTANCE_NONE);
wxString getUsrDistanceUnit(DistanceUnit unit = DISTANCE_NONE);

double toUsrSpeed(double kts_speed, SpeedUnit unit = SPEED_NONE);
double fromUsrSpeed(double usr_speed, SpeedUnit unit = SPEED_NONE);
wxString getUsrSpeedUnit(SpeedUnit unit = SPEED_NONE);

#endif
