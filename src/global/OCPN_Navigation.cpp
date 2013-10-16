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

#include "OCPN_Navigation.h"

namespace global {

const Navigation::Data & OCPN_Navigation::get_data() const
{
	return data;
}

void OCPN_Navigation::set_latitude(double value)
{
	data.lat = value;
}

void OCPN_Navigation::set_longitude(double value)
{
	data.lon = value;
}

void OCPN_Navigation::set_magn_var(double value)
{
	data.var = value;
}

void OCPN_Navigation::set_heading_true(double value)
{
	data.hdt = value;
}

void OCPN_Navigation::set_heading_magn(double value)
{
	data.hdm = value;
}

void OCPN_Navigation::set_speed_over_ground(double value)
{
	data.sog = value;
}

void OCPN_Navigation::set_course_over_ground(double value)
{
	data.cog = value;
}

}

