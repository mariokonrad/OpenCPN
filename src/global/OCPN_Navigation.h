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
	Track track;
	Anchor data_anchor;

public: // data
	virtual const Data& get_data() const;
	virtual void set_position(const Position&);
	virtual void set_latitude(double);
	virtual void set_longitude(double);
	virtual void set_magn_var(double);
	virtual void set_heading_true(double);
	virtual void set_heading_magn(double);
	virtual void set_speed_over_ground(double);
	virtual void set_course_over_ground(double);

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
};

}

#endif
