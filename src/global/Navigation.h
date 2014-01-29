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

#ifndef __GLOBAL__NAVIGATION__H__
#define __GLOBAL__NAVIGATION__H__

#include <geo/Position.h>

namespace global {

class Navigation
{
public:
	virtual ~Navigation()
	{
	}

public:
	struct Data
	{
		geo::Position view_point; // position of view point
		geo::Position pos; // latitude/longitude
		double cog; // course over ground in degrees
		double sog; // speed over ground in knots
		double hdt; // heading degrees true
		double hdm; // heading degrees magnetic
		double var; // magnetic variationn in degrees
	};

	virtual const Data& get_data() const = 0;
	virtual void set_view_point(const geo::Position&) = 0;
	virtual void set_position(const geo::Position&) = 0;
	virtual void set_latitude(double) = 0;
	virtual void set_longitude(double) = 0;
	virtual void set_magn_var(double) = 0;
	virtual void set_heading_true(double) = 0;
	virtual void set_heading_magn(double) = 0;
	virtual void set_speed_over_ground(double) = 0;
	virtual void set_course_over_ground(double) = 0;

public:
	struct Route
	{
		double arrival_circle_radius; // nautical miles
	};

	virtual const Route& route() const = 0;
	virtual void set_route_arrival_circle_radius(double) = 0;

public:
	struct Track
	{
		long TrackPrecision;
		bool HighliteTracks;
		bool TrackDaily;
		double TrackDeltaDistance;
		double PlanSpeed;
	};

	virtual const Track& get_track() const = 0;
	virtual void set_TrackPrecision(long) = 0;
	virtual void set_HighliteTracks(bool) = 0;
	virtual void set_TrackDaily(bool) = 0;
	virtual void set_TrackDeltaDistance(double) = 0;
	virtual void set_PlanSpeed(double) = 0;

public:
	struct Anchor
	{
		double PointMinDist;
		bool AlertOn1;
		bool AlertOn2;
		long AWDefault;
		long AWMax;
	};

	virtual const Anchor& anchor() const = 0;
	virtual void set_anchor_PointMinDist(double) = 0;
	virtual void set_anchor_AlertOn1(bool) = 0;
	virtual void set_anchor_AlertOn2(bool) = 0;
	virtual void set_anchor_AWDefault(long) = 0;
	virtual void set_anchor_AWMax(long) = 0;

public:
	struct GPS
	{
		bool valid;
		int SatsInView;
		bool SatValid;
	};

	virtual const GPS& gps() const = 0;
	virtual void set_gps_valid(bool) = 0;
	virtual void set_gps_SatsInView(int) = 0;
	virtual void set_gps_SatValid(bool) = 0;
};

}

#endif
