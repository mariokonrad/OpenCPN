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

#ifndef __NAVIGATION__DEFAULTROUTETRACKER__H__
#define __NAVIGATION__DEFAULTROUTETRACKER__H__

#include <navigation/RouteTracker.h>

namespace navigation {

/// Default implementation of a route tracker.
///
/// FIXME: this should receive (NMEA) position events, not the actual track
class DefaultRouteTracker : public RouteTracker
{
public:
	DefaultRouteTracker();
	virtual ~DefaultRouteTracker();

	virtual void start();
	virtual void stop(bool do_add_point = false, bool process_track = true);
	virtual void restart();
	virtual void set_precision(long);

	virtual bool has_active_track() const;
	virtual bool is_active_track(const Route*) const;
	virtual bool is_running() const;
	virtual bool is_active() const;

	virtual geo::Position get_last_position() const;

private:
	Track* do_stop(bool, bool = true, bool do_notify = true);
	void notify_toolbar() const;
	void notify_routemanager() const;
	void notify_plugins_start() const;
	void notify_plugins_stop() const;

	Track* track; /// The current track which receives new positions.
	bool active; /// Indicates this tracker is actively tracking.
};

}

#endif
