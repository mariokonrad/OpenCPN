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

class Routeman : public navigation::RouteManager
{
public:
	Routeman();
	virtual ~Routeman();

	// route handling
	virtual bool ActivateNextPoint(Route* pr, bool skipped);
	virtual bool ActivateRoute(Route* pRouteToActivate, RoutePoint* pStartPoint = NULL);
	virtual bool ActivateRoutePoint(Route* pA, RoutePoint* pRP);
	virtual bool DeactivateRoute(bool b_arrival = false);
	virtual void DeleteAllRoutes(void);
	virtual void DeleteRoute(Route* pRoute);
	virtual bool DoesRouteContainSharedPoints(const Route* pRoute);
	virtual Route* FindRouteByGUID(const wxString& guid) const;
	virtual Route* FindRouteContainingWaypoint(const RoutePoint* pWP) const;
	virtual Route* GetpActiveRoute();
	virtual RoutePoint* GetpActivePoint();
	virtual bool IsRouteValid(const Route* pRoute) const;
	virtual bool RouteExists(const Route* route) const;
	virtual Route* RouteExists(const wxString& guid) const;
	virtual bool IsAnyRouteActive(void) const;
	virtual RouteArray GetRouteArrayContaining(const RoutePoint* pWP);

	// track handling
	virtual void DeleteAllTracks(void);
	virtual void DeleteTrack(Route* pRoute);

	// navigation
	virtual double GetCurrentRngToActivePoint() const;
	virtual double GetCurrentBrgToActivePoint() const;
	virtual double GetCurrentRngToActiveNormalArrival() const;
	virtual double GetCurrentXTEToActivePoint() const;
	virtual double GetCurrentSegmentCourse() const;
	virtual int GetXTEDir() const;

	// gui
	virtual const wxBrush& GetActiveRouteBrush(void) const;
	virtual const wxPen& GetActiveRoutePointPen(void) const;
	virtual const wxPen& GetActiveRoutePen(void) const;
	virtual const wxBrush& GetSelectedRouteBrush(void) const;
	virtual const wxPen& GetRoutePen(void) const;
	virtual const wxPen& GetRoutePointPen(void) const;
	virtual const wxPen& GetSelectedRoutePen(void) const;
	virtual void SetColorScheme(global::ColorScheme cs);

	// misc
	virtual bool is_data_valid() const;
	virtual bool UpdateProgress();
	virtual const wxString& GetRouteReverseMessage(void) const;

private:
	bool UpdateAutopilot();
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
