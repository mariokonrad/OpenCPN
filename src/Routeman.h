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
#include <RouteManager.h>

class Routeman
{
public:
	typedef RouteManager::RouteArray RouteArray;

	Routeman();
	~Routeman();

	void DeleteRoute(Route* pRoute);
	void DeleteAllRoutes(void);
	void DeleteAllTracks(void);

	void DeleteTrack(Route* pRoute);

	Route* FindRouteByGUID(const wxString& guid) const;
	Route* FindRouteContainingWaypoint(const RoutePoint* pWP) const;
	RouteArray GetRouteArrayContaining(const RoutePoint* pWP);
	bool DoesRouteContainSharedPoints(const Route* pRoute);

	bool ActivateRoute(Route* pRouteToActivate, RoutePoint* pStartPoint = NULL);
	bool ActivateRoutePoint(Route* pA, RoutePoint* pRP);
	bool ActivateNextPoint(Route* pr, bool skipped);

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
	double RouteBrgToActivePoint;
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
