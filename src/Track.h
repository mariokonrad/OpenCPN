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

#ifndef __TRACK__H__
#define __TRACK__H__

#include <wx/datetime.h>
#include <wx/progdlg.h>
#include <wx/timer.h>

#include <vector>
#include <deque>

#include "Route.h"
#include "Vector2D.h"

class Track
	: public wxEvtHandler
	, public Route
{
		DECLARE_EVENT_TABLE()

	public:
		Track(void);
		virtual ~Track(void);

		void SetPrecision(int precision);

		void Start(void);
		void Stop(bool do_add_point = false);
		Track *DoExtendDaily(void);
		bool IsRunning();
		void Draw(ocpnDC& dc, const ViewPort &VP);

		RoutePoint* AddNewPoint(Vector2D point, wxDateTime time);
		Route *RouteFromTrack(wxProgressDialog * pprog);

		void DouglasPeuckerReducer(std::vector<RoutePoint*> & list, int from, int to, double delta);
		int Simplify( double maxDelta );
		double GetXTE(RoutePoint * fm1, RoutePoint * fm2, RoutePoint * to);
		double GetXTE(double fm1Lat, double fm1Lon, double fm2Lat, double fm2Lon, double toLat, double toLon);

		void AdjustCurrentTrackPoint(RoutePoint * prototype);

	private:
		void OnTimerTrack(wxTimerEvent & event);
		void AddPointNow(bool do_add_point = false);

		bool m_bRunning;
		wxTimer m_TimerTrack;

		int m_nPrecision;
		double m_TrackTimerSec;
		double m_allowedMaxXTE;
		double m_allowedMaxAngle;

		Vector2D m_lastAddedPoint;
		double m_prev_dist;
		wxDateTime m_prev_time;

		RoutePoint * m_lastStoredTP;
		RoutePoint * m_removeTP;
		RoutePoint * m_prevFixedTP;
		RoutePoint * m_fixedTP;
		int m_track_run;
		double m_minTrackpoint_delta;

		enum eTrackPointState {
			firstPoint,
			secondPoint,
			potentialPoint
		} trackPointState;

		std::deque<Vector2D> skipPoints;
		std::deque<wxDateTime> skipTimes;
};

#endif
