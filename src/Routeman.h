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

#ifndef __ROUTEMAN_H__
#define __ROUTEMAN_H__

#include <cmath>

#include "App.h"
#include "nmea0183.h"
#include "ColorScheme.h"

class Route;
class RoutePoint;

class Routeman
{
	public:
		Routeman(App * parent);
		~Routeman();

		void DeleteRoute(Route *pRoute);
		void DeleteAllRoutes(void);
		void DeleteAllTracks(void);

		void DeleteTrack(Route *pRoute);

		Route * FindRouteByGUID(const wxString & guid) const;
		Route *FindRouteContainingWaypoint(RoutePoint *pWP);
		wxArrayPtrVoid *GetRouteArrayContaining(RoutePoint *pWP);
		bool DoesRouteContainSharedPoints( Route *pRoute );

		bool ActivateRoute(Route *pRouteToActivate, RoutePoint *pStartPoint = NULL);
		bool ActivateRoutePoint(Route *pA, RoutePoint *pRP);
		bool ActivateNextPoint(Route *pr, bool skipped);
		RoutePoint *FindBestActivatePoint(Route *pR, double lat, double lon, double cog, double sog);

		bool UpdateProgress();
		bool UpdateAutopilot();
		bool DeactivateRoute( bool b_arrival = false );
		bool IsAnyRouteActive(void) const { return pActiveRoute != NULL; } // FIXME: move to cpp
		void SetColorScheme(ColorScheme cs);

		Route *GetpActiveRoute(){ return pActiveRoute;}
		RoutePoint *GetpActivePoint(){ return pActivePoint;}
		double GetCurrentRngToActivePoint(){ return CurrentRngToActivePoint;}
		double GetCurrentBrgToActivePoint(){ return CurrentBrgToActivePoint;}
		double GetCurrentRngToActiveNormalArrival(){ return CurrentRangeToActiveNormalCrossing;}
		double GetCurrentXTEToActivePoint(){ return CurrentXTEToActivePoint;}
		double GetCurrentSegmentCourse(){ return CurrentSegmentCourse;}
		int   GetXTEDir(){ return XTEDir;}

		wxPen   * GetRoutePen(void){return m_pRoutePen;}
		wxPen   * GetSelectedRoutePen(void){return m_pSelectedRoutePen;}
		wxPen   * GetActiveRoutePen(void){return m_pActiveRoutePen;}
		wxPen   * GetActiveRoutePointPen(void){return m_pActiveRoutePointPen;}
		wxPen   * GetRoutePointPen(void){return m_pRoutePointPen;}
		wxBrush * GetRouteBrush(void){return m_pRouteBrush;}
		wxBrush * GetSelectedRouteBrush(void){return m_pSelectedRouteBrush;}
		wxBrush * GetActiveRouteBrush(void){return m_pActiveRouteBrush;}
		wxBrush * GetActiveRoutePointBrush(void){return m_pActiveRoutePointBrush;}
		wxBrush * GetRoutePointBrush(void){return m_pRoutePointBrush;}

		wxString GetRouteReverseMessage(void);

		Route * RouteExists(const wxString & guid) const;
		bool RouteExists(Route * route) const;
		bool IsRouteValid(Route *pRoute) const;

		bool m_bDataValid; // FIXME: public attribute

	private:
		void DoAdvance(void);

		App * m_pparent_app;
		Route * pActiveRoute;
		RoutePoint * pActivePoint;
		double RouteBrgToActivePoint;        //TODO all these need to be doubles
		double CurrentSegmentBeginLat;
		double CurrentSegmentBeginLon;
		double CurrentRngToActivePoint;
		double CurrentBrgToActivePoint;
		double CurrentXTEToActivePoint;
		double CourseToRouteSegment;
		double CurrentRangeToActiveNormalCrossing;
		RoutePoint * pActiveRouteSegmentBeginPoint;
		RoutePoint * pRouteActivatePoint;
		double CurrentSegmentCourse;
		int XTEDir;
		bool m_bArrival;
		wxPen * m_pRoutePen;
		wxPen * m_pSelectedRoutePen;
		wxPen * m_pActiveRoutePen;
		wxPen * m_pActiveRoutePointPen;
		wxPen * m_pRoutePointPen;
		wxBrush * m_pRouteBrush;
		wxBrush * m_pSelectedRouteBrush;
		wxBrush * m_pActiveRouteBrush;
		wxBrush * m_pActiveRoutePointBrush;
		wxBrush * m_pRoutePointBrush;

		NMEA0183 m_NMEA0183; // For autopilot output

		double m_arrival_min;
		int m_arrival_test;
};

#endif
