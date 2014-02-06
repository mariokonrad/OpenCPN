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

#ifndef __GLOBAL__OCPN_NAVIGATION__H__
#define __GLOBAL__OCPN_NAVIGATION__H__

#include <global/Navigation.h>

namespace global {

class OCPN_Navigation : public Navigation
{
private:
	Data data;
	Route data_route;
	Track track;
	Anchor data_anchor;
	GPS data_gps;

public: // data
	virtual const Data& get_data() const;
	virtual void set_view_point(const geo::Position&);
	virtual void set_position(const geo::Position&);
	virtual void set_latitude(double);
	virtual void set_longitude(double);
	virtual void set_magn_var(double);
	virtual void set_heading_true(double);
	virtual void set_heading_magn(double);
	virtual void set_speed_over_ground(double);
	virtual void set_course_over_ground(double);
	virtual void set_user_var(double);
	virtual void set_CourseUp(bool);
	virtual void set_COGAvgSec(int);
	virtual void set_MagneticAPB(bool);

public: // route
	virtual const Route& route() const;
	virtual void set_route_arrival_circle_radius(double);

public: // track
	virtual const Track& get_track() const;
	virtual void set_TrackPrecision(long);
	virtual void set_HighliteTracks(bool);
	virtual void set_TrackDaily(bool);
	virtual void set_TrackDeltaDistance(double);
	virtual void set_PlanSpeed(double);

public: // anchor
	virtual const Anchor& anchor() const;
	virtual void set_anchor_PointMinDist(double);
	virtual void set_anchor_AlertOn1(bool);
	virtual void set_anchor_AlertOn2(bool);
	virtual void set_anchor_AWDefault(long);
	virtual void set_anchor_AWMax(long);
	virtual void set_anchor_AW1GUID(const wxString&);
	virtual void set_anchor_AW2GUID(const wxString&);

public: // gps
	virtual const GPS& gps() const;
	virtual void set_gps_valid(bool);
	virtual void set_gps_SatsInView(int);
	virtual void set_gps_SatValid(bool);
};

}

#endif
