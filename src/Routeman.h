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

#ifndef __ROUTEMAN__H__
#define __ROUTEMAN__H__

#include "nmea0183.h"
#include <global/ColorScheme.h>
#include <geo/Position.h>

#include <vector>

class Route;
class RoutePoint;

class wxBrush;
class wxPen;


/// Interface for route managers. FIXME: in the future, this will be used as interface to the routemanager (globally)
class RouteManager
{
public:
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

class Routeman
{
public:
	typedef std::vector<Route*> RouteArray;

public:
	Routeman();
	~Routeman();

	void DeleteRoute(Route* pRoute);
	void DeleteAllRoutes(void);
	void DeleteAllTracks(void);

	void DeleteTrack(Route* pRoute);

	Route* FindRouteByGUID(const wxString& guid) const;
	Route* FindRouteContainingWaypoint(const RoutePoint* pWP) const;
	RouteArray* GetRouteArrayContaining(const RoutePoint* pWP); // FIXME: returns std container
	bool DoesRouteContainSharedPoints(const Route* pRoute);

	bool ActivateRoute(Route* pRouteToActivate, RoutePoint* pStartPoint = NULL);
	bool ActivateRoutePoint(Route* pA, RoutePoint* pRP);
	bool ActivateNextPoint(Route* pr, bool skipped);
	RoutePoint* FindBestActivatePoint(Route* pR, const geo::Position& pos, double cog, double sog); // FIXME: move this to 'Route'

	bool UpdateProgress();
	bool UpdateAutopilot();
	bool DeactivateRoute(bool b_arrival = false);
	bool IsAnyRouteActive(void) const;
	void SetColorScheme(global::ColorScheme cs);

	Route* GetpActiveRoute();
	RoutePoint* GetpActivePoint();
	double GetCurrentRngToActivePoint() const;
	double GetCurrentBrgToActivePoint() const;
	double GetCurrentRngToActiveNormalArrival() const;
	double GetCurrentXTEToActivePoint() const;
	double GetCurrentSegmentCourse() const;
	int GetXTEDir() const;

	const wxPen& GetRoutePen(void) const;
	const wxPen& GetSelectedRoutePen(void) const;
	const wxPen& GetRoutePointPen(void) const;
	const wxBrush& GetActiveRouteBrush(void) const;
	const wxPen& GetActiveRoutePointPen(void) const;
	const wxPen& GetActiveRoutePen(void) const;
	const wxBrush& GetSelectedRouteBrush(void) const;

	const wxString& GetRouteReverseMessage(void) const;

	Route* RouteExists(const wxString& guid) const;
	bool RouteExists(const Route* route) const;
	bool IsRouteValid(const Route* pRoute) const;
	bool is_data_valid() const;

private:
	void DoAdvance(void);

	bool m_bDataValid;
	Route* pActiveRoute;
	RoutePoint* pActivePoint;
	double RouteBrgToActivePoint; // TODO all these need to be doubles
	double CurrentSegmentBeginLat;
	double CurrentSegmentBeginLon;
	double CurrentRngToActivePoint;
	double CurrentBrgToActivePoint;
	double CurrentXTEToActivePoint;
	double CourseToRouteSegment;
	double CurrentRangeToActiveNormalCrossing;
	RoutePoint* pActiveRouteSegmentBeginPoint;
	RoutePoint* pRouteActivatePoint;
	double CurrentSegmentCourse;
	int XTEDir;
	bool m_bArrival;
	wxPen* m_pRoutePen;
	wxPen* m_pSelectedRoutePen;
	wxPen* m_pActiveRoutePen;
	wxPen* m_pActiveRoutePointPen;
	wxPen* m_pRoutePointPen;
	wxBrush* m_pRouteBrush;
	wxBrush* m_pSelectedRouteBrush;
	wxBrush* m_pActiveRouteBrush;
	wxBrush* m_pActiveRoutePointBrush;
	wxBrush* m_pRoutePointBrush;

	NMEA0183 m_NMEA0183; // For autopilot output

	double m_arrival_min;
	int m_arrival_test;
};

#endif
