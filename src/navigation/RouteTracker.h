/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#ifndef __NAVIGATION__ROUTETRACKER__H__
#define __NAVIGATION__ROUTETRACKER__H__

#include <geo/Position.h>

class Route;
class Track;

namespace navigation {

/// Interface to route trackers.
class RouteTracker
{
public:
	virtual ~RouteTracker()
	{
	}

	/// Starts the tracking.
	virtual void start() = 0;

	/// Stops the tracking.
	virtual void stop(bool do_add_point = false, bool process_track = true) = 0;

	/// Restarts the tracking.
	virtual void restart() = 0;

	/// Sets the precision of the tracking.
	virtual void set_precision(long) = 0;

	/// Returns true if there is an active track. This does not
	/// mean, the tracking is currently active.
	virtual bool has_active_track() const = 0;

	/// Returns true if the specified route is the active track.
	virtual bool is_active_track(const Route*) const = 0;

	/// Returns true if there is a current track and it is running.
	virtual bool is_running() const = 0;

	/// Returns true if the tracker is tracking.
	virtual bool is_active() const = 0;

	/// Returns the last tracked position, if there is a current track.
	virtual geo::Position get_last_position() const = 0;
};

}

#endif
