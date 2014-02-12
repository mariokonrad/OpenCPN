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

#ifndef __ROUTEMANAGER__H__
#define __ROUTEMANAGER__H__

#include <global/ColorScheme.h>
#include <vector>

class Route;
class RoutePoint;
class wxBrush;
class wxPen;

/// Interface for route managers. FIXME: in the future, this will be used as interface to the routemanager (globally)
class RouteManager
{
public:
	typedef std::vector<Route*> RouteArray;

	virtual ~RouteManager() {}

	// route handling
	virtual bool ActivateNextPoint(Route* pr, bool skipped) = 0;
	virtual bool ActivateRoute(Route* pRouteToActivate, RoutePoint* pStartPoint = NULL) = 0;
	virtual bool ActivateRoutePoint(Route* pA, RoutePoint* pRP) = 0;
	virtual bool DeactivateRoute(bool b_arrival = false) = 0;
	virtual void DeleteAllRoutes(void) = 0;
	virtual void DeleteRoute(Route* pRoute) = 0;
	virtual bool DoesRouteContainSharedPoints(const Route* pRoute) const = 0;
	virtual Route* FindRouteByGUID(const wxString& guid) const = 0;
	virtual Route* FindRouteContainingWaypoint(const RoutePoint* pWP) const = 0;
	virtual Route* GetpActiveRoute() = 0;
	virtual RoutePoint* GetpActivePoint() = 0;
	virtual bool IsRouteValid(const Route* pRoute) const = 0;
	virtual bool RouteExists(const Route* route) const = 0;
	virtual bool IsAnyRouteActive(void) const = 0;
	virtual std::vector<Route*> GetRouteArrayContaining(const RoutePoint* pWP) const = 0;

	// track handling
	virtual void DeleteAllTracks(void) = 0;
	virtual void DeleteTrack(Route* track) = 0;

	// navigation
	virtual double GetCurrentRngToActivePoint() const = 0;
	virtual double GetCurrentBrgToActivePoint() const = 0;
	virtual double GetCurrentRngToActiveNormalArrival() const = 0;
	virtual double GetCurrentXTEToActivePoint() const = 0;
	virtual double GetCurrentSegmentCourse() const = 0;
	virtual int GetXTEDir() const = 0;

	// gui
	virtual const wxPen& GetActiveRoutePointPen(void) const = 0;
	virtual const wxPen& GetActiveRoutePen(void) const = 0;
	virtual const wxBrush& GetSelectedRouteBrush(void) const = 0;
	virtual const wxPen& GetRoutePen(void) const = 0;
	virtual const wxPen& GetRoutePointPen(void) const = 0;
	virtual const wxPen& GetSelectedRoutePen(void) const = 0;
	virtual void SetColorScheme(global::ColorScheme cs) = 0; // FIXME: this should be an observer pattern

	// misc
	virtual bool is_data_valid() const = 0;
	virtual bool UpdateProgress() = 0;
};

#endif
