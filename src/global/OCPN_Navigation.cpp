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

void OCPN_Navigation::set_view_point(const geo::Position& view_point)
{
	data.view_point = view_point;
}

void OCPN_Navigation::set_position(const geo::Position& position)
{
	data.pos = position;
}

void OCPN_Navigation::set_latitude(double latitude)
{
	set_position(geo::Position(latitude, data.pos.lon()));
}

void OCPN_Navigation::set_longitude(double longitude)
{
	set_position(geo::Position(data.pos.lat(), longitude));
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

void OCPN_Navigation::set_user_var(double value)
{
	data.user_var = value;
}

void OCPN_Navigation::set_CourseUp(bool value)
{
	data.CourseUp = value;
}

void OCPN_Navigation::set_COGAvgSec(int value)
{
	data.COGAvgSec = value;
}

void OCPN_Navigation::set_MagneticAPB(bool value)
{
	data.MagneticAPB = value;
}

const Navigation::Route& OCPN_Navigation::route() const
{
	return data_route;
}

void OCPN_Navigation::set_route_arrival_circle_radius(double value)
{
	data_route.arrival_circle_radius = value;
}

const Navigation::Track& OCPN_Navigation::get_track() const
{
	return track;
}

void OCPN_Navigation::set_TrackPrecision(long value)
{
	track.TrackPrecision = value;
}

void OCPN_Navigation::set_HighliteTracks(bool value)
{
	track.HighliteTracks = value;
}

void OCPN_Navigation::set_TrackDaily(bool value)
{
	track.TrackDaily = value;
}

void OCPN_Navigation::set_TrackDeltaDistance(double value)
{
	track.TrackDeltaDistance = value;
}

void OCPN_Navigation::set_PlanSpeed(double value)
{
	track.PlanSpeed = value;
}

const Navigation::Anchor& OCPN_Navigation::anchor() const
{
	return data_anchor;
}

void OCPN_Navigation::set_anchor_PointMinDist(double value)
{
	data_anchor.PointMinDist = value;
}

void OCPN_Navigation::set_anchor_AlertOn1(bool value)
{
	data_anchor.AlertOn1 = value;
}

void OCPN_Navigation::set_anchor_AlertOn2(bool value)
{
	data_anchor.AlertOn2 = value;
}

void OCPN_Navigation::set_anchor_AWDefault(long value)
{
	data_anchor.AWDefault = value;
}

void OCPN_Navigation::set_anchor_AWMax(long value)
{
	data_anchor.AWMax = value;
}

void OCPN_Navigation::set_anchor_AW1GUID(const wxString& value)
{
	data_anchor.AW1GUID = value;
}

void OCPN_Navigation::set_anchor_AW2GUID(const wxString& value)
{
	data_anchor.AW2GUID = value;
}

const Navigation::GPS& OCPN_Navigation::gps() const
{
	return data_gps;
}

void OCPN_Navigation::set_gps_valid(bool value)
{
	data_gps.valid = value;
}

void OCPN_Navigation::set_gps_SatsInView(int value)
{
	data_gps.SatsInView = value;
}

void OCPN_Navigation::set_gps_SatValid(bool value)
{
	data_gps.SatValid = value;
}

}

