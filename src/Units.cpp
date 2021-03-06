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

#include "Units.h"

#include <global/OCPN.h>
#include <global/System.h>

#include <wx/string.h>

// Converts the distance to the units selected by user
double toUsrDistance(double nm_distance, DistanceUnit unit)
{
	if (unit == DISTANCE_NONE)
		unit = static_cast<DistanceUnit>(global::OCPN::get().sys().config().DistanceFormat);
	switch (unit) {
		case DISTANCE_NMI:
			return nm_distance; // Nautical miles
		case DISTANCE_MI:
			return nm_distance * 1.15078; // statute miles
		case DISTANCE_KM:
			return nm_distance * 1.852;
		case DISTANCE_M:
			return nm_distance * 1852.0;
		case DISTANCE_FT:
			return nm_distance * 6076.12;
		case DISTANCE_FA:
			return nm_distance * 1012.68591;
		case DISTANCE_IN:
			return nm_distance * 72913.4;
		case DISTANCE_CM:
			return nm_distance * 185200.0;
		case DISTANCE_NONE:
			break;
	}
	return 0.0;
}

// Converts the distance from the units selected by user to NMi
double fromUsrDistance(double usr_distance, DistanceUnit unit)
{
	if (unit == DISTANCE_NONE)
		unit = static_cast<DistanceUnit>(global::OCPN::get().sys().config().DistanceFormat);
	switch (unit) {
		case DISTANCE_NMI:
			return usr_distance; // nautical miles
		case DISTANCE_MI:
			return usr_distance / 1.15078; // statute miles
		case DISTANCE_KM:
			return usr_distance / 1.852;
		case DISTANCE_M:
			return usr_distance / 1852.0;
		case DISTANCE_FT:
			return usr_distance / 6076.12;
		case DISTANCE_FA:
			return usr_distance / 1012.68591;
		case DISTANCE_IN:
			return usr_distance / 72913.4;
		case DISTANCE_CM:
			return usr_distance / 185200.0;
		case DISTANCE_NONE:
			break;
	}
	return 0.0;
}

// Returns the abbreviation of user selected distance unit
wxString getUsrDistanceUnit(DistanceUnit unit)
{
	if (unit == DISTANCE_NONE)
		unit = static_cast<DistanceUnit>(global::OCPN::get().sys().config().DistanceFormat);
	switch (unit) {
		case DISTANCE_NMI:
			return _T("NMi"); // nautical miles
		case DISTANCE_MI:
			return _T("mi"); // statute miles
		case DISTANCE_KM:
			return _T("km");
		case DISTANCE_M:
			return _T("m");
		case DISTANCE_FT:
			return _T("ft");
		case DISTANCE_FA:
			return _T("fa");
		case DISTANCE_IN:
			return _T("in");
		case DISTANCE_CM:
			return _T("cm");
		case DISTANCE_NONE:
			break;
	}
	return wxString();
}

// Converts the speed to the units selected by user
double toUsrSpeed(double kts_speed, SpeedUnit unit)
{
	if (unit == SPEED_NONE)
		unit = static_cast<SpeedUnit>(global::OCPN::get().sys().config().SpeedFormat);
	switch (unit) {
		case SPEED_KTS:
			return kts_speed; // kts
		case SPEED_MPH:
			return kts_speed * 1.15078; // mph
		case SPEED_KMH:
			return kts_speed * 1.852; // km/h
		case SPEED_MS:
			return kts_speed * 0.514444444; // m/s
		case SPEED_NONE:
			break;
	}
	return 0.0;
}

// Converts the speed from the units selected by user to knots
double fromUsrSpeed(double usr_speed, SpeedUnit unit)
{
	if (unit == SPEED_NONE)
		unit = static_cast<SpeedUnit>(global::OCPN::get().sys().config().SpeedFormat);
	switch (unit) {
		case SPEED_KTS:
			return usr_speed; // kts
		case SPEED_MPH:
			return usr_speed / 1.15078; // mph
		case SPEED_KMH:
			return usr_speed / 1.852; // km/h
		case SPEED_MS:
			return usr_speed / 0.514444444; // m/s
		case SPEED_NONE:
			break;
	}
	return 0.0;
}

// Returns the abbreviation of user selected speed unit
wxString getUsrSpeedUnit(SpeedUnit unit)
{
	if (unit == SPEED_NONE)
		unit = static_cast<SpeedUnit>(global::OCPN::get().sys().config().SpeedFormat);
	switch (unit) {
		case SPEED_KTS:
			return _T("kts"); // kts
		case SPEED_MPH:
			return _T("mph"); // mph
		case SPEED_KMH:
			return _T("km/h");
		case SPEED_MS:
			return _T("m/s");
		case SPEED_NONE:
			break;
	}
	return wxString();
}

