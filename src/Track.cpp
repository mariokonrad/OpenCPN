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

#include "Track.h"

#include "Select.h"
#include "ocpnDC.h"
#include "RouteProp.h"
#include "navutil.h"
#include "gpx/gpx.h"

#include <ChartCanvas.h>
#include <UserColors.h>

#include <geo/GeoRef.h>

#define TIMER_TRACK1 778

extern int g_nTrackPrecision;
extern double gLat;
extern double gLon;
extern RouteList * pRouteList;
extern Select * pSelect;
extern bool g_bTrackDaily;
extern bool g_bHighliteTracks;
extern int g_route_line_width;
extern ChartCanvas * cc1;
extern double g_TrackDeltaDistance;
extern RouteProp * pRoutePropDialog;
extern double g_PlanSpeed;

double _distance2(Vector2D & a, Vector2D & b) // FIXME: vector operator -
{
	return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
}

double _distance(Vector2D & a, Vector2D & b) // FIXME: vector operator -
{
	return sqrt(_distance2( a, b ));
}

BEGIN_EVENT_TABLE(Track, wxEvtHandler)
	EVT_TIMER(TIMER_TRACK1, Track::OnTimerTrack)
END_EVENT_TABLE()


Track::Track(void)
{
	m_TimerTrack.SetOwner(this, TIMER_TRACK1);
	m_TimerTrack.Stop();
	m_bRunning = false;
	m_bIsTrack = true;

	SetPrecision(g_nTrackPrecision);

	m_prev_time = wxInvalidDateTime;
	m_lastStoredTP = NULL;

	wxDateTime now = wxDateTime::Now();
	m_ConfigRouteNum = now.GetTicks();
	trackPointState = firstPoint;
	m_lastStoredTP = NULL;
	m_removeTP = NULL;
	m_fixedTP = NULL;
	m_track_run = 0;
}

Track::~Track()
{
	Stop();
}

bool Track::IsRunning()
{
	return m_bRunning;
}

void Track::SetPrecision( int prec )
{
	m_nPrecision = prec;
	switch( m_nPrecision ) {
		case 0: // Low
			m_allowedMaxAngle = 10;
			m_allowedMaxXTE = 0.008;
			m_TrackTimerSec = 8;
			m_minTrackpoint_delta = .004;
			break;

		case 1: // Medium
			m_allowedMaxAngle = 10;
			m_allowedMaxXTE = 0.004;
			m_TrackTimerSec = 4;
			m_minTrackpoint_delta = .002;
			break;

		case 2: // High
			m_allowedMaxAngle = 10;
			m_allowedMaxXTE = 0.0015;
			m_TrackTimerSec = 2;
			m_minTrackpoint_delta = .001;
			break;
	}
}

void Track::Start( void )
{
	if( !m_bRunning ) {
		AddPointNow( true );                   // Add initial point
		m_TimerTrack.Start( 1000, wxTIMER_CONTINUOUS );
		m_bRunning = true;
	}
}

void Track::Stop( bool do_add_point )
{
	double delta = 0.0;
	if( m_lastStoredTP )
		delta = DistGreatCircle(gLat, gLon, m_lastStoredTP->m_lat, m_lastStoredTP->m_lon);

	if( ( m_bRunning ) && ( ( delta > m_minTrackpoint_delta ) || do_add_point ) ) AddPointNow(
			true );                   // Add last point

	m_TimerTrack.Stop();
	m_bRunning = false;
	m_track_run = 0;
}

Track *Track::DoExtendDaily()
{
	Route *pExtendRoute = NULL;
	RoutePoint *pExtendPoint = NULL;

	RoutePoint *pLastPoint = this->GetPoint( 1 );

	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route * route = *i;
		if (!route->m_bIsInLayer && route->m_bIsTrack && (route->m_GUID != this->m_GUID)) {
			RoutePoint * track_node = route->GetLastPoint();
			if (track_node->GetCreateTime() <= pLastPoint->GetCreateTime()) {
				if (!pExtendPoint || track_node->GetCreateTime() > pExtendPoint->GetCreateTime()) {
					pExtendPoint = track_node;
					pExtendRoute = route;
				}
			}
		}
	}
	if( pExtendRoute
			&& pExtendRoute->GetPoint( 1 )->GetCreateTime().FromTimezone( wxDateTime::GMT0 ).IsSameDate(
				pLastPoint->GetCreateTime().FromTimezone( wxDateTime::GMT0 ) ) ) {
		int begin = 1;
		if( pLastPoint->GetCreateTime() == pExtendPoint->GetCreateTime() ) begin = 2;
		pSelect->DeleteAllSelectableTrackSegments(pExtendRoute);
		wxString suffix = _T("");
		if( this->m_RouteNameString.IsNull() ) {
			suffix = pExtendRoute->m_RouteNameString;
			if( suffix.IsNull() ) suffix = wxDateTime::Today().FormatISODate();
		}
		pExtendRoute->CloneTrack( this, begin, this->GetnPoints(), suffix );
		pSelect->AddAllSelectableTrackSegments( pExtendRoute );
		pSelect->DeleteAllSelectableTrackSegments( this );
		this->ClearHighlights();
		return (Track *)pExtendRoute;
	} else {
		if( this->m_RouteNameString.IsNull() )
			this->m_RouteNameString = wxDateTime::Today().FormatISODate();
		return NULL;
	}
}

void Track::AdjustCurrentTrackPoint( RoutePoint *prototype )
{
	if(prototype) {
		CloneAddedTrackPoint( m_lastStoredTP, prototype );
		m_prev_time = prototype->GetCreateTime().FromUTC();
	}
}

void Track::OnTimerTrack(wxTimerEvent &)
{
	m_TimerTrack.Stop();
	m_track_run++;

	if( m_lastStoredTP )
		m_prev_dist = DistGreatCircle( gLat, gLon, m_lastStoredTP->m_lat, m_lastStoredTP->m_lon );
	else
		m_prev_dist = 999.0;

	bool b_addpoint = false;

	if( ( m_TrackTimerSec > 0. ) && ( (double) m_track_run >= m_TrackTimerSec )
			&& ( m_prev_dist > m_minTrackpoint_delta ) ) {
		b_addpoint = true;
		m_track_run = 0;
	}

	if( b_addpoint )
		AddPointNow();
	else   //continuously update track beginning point timestamp if no movement.
		if ((trackPointState == firstPoint) && !g_bTrackDaily)
		{
			wxDateTime now = wxDateTime::Now();
			pRoutePointList->GetFirst()->GetData()->SetCreateTime(now.ToUTC());
		}

	m_TimerTrack.Start( 1000, wxTIMER_CONTINUOUS );
}

RoutePoint* Track::AddNewPoint(Vector2D point, wxDateTime time )
{
	RoutePoint *rPoint = new RoutePoint(point.lat, point.lon, _T( "empty"), _T(""));
	rPoint->m_bShowName = false;
	rPoint->m_bIsVisible = true;
	rPoint->m_GPXTrkSegNo = 1;
	rPoint->SetCreateTime(time);
	AddPoint( rPoint );

	//    This is a hack, need to undo the action of Route::AddPoint
	rPoint->m_bIsInRoute = false;
	rPoint->m_bIsInTrack = true;
	return rPoint;
}

void Track::AddPointNow( bool do_add_point )
{
	static std::vector<RoutePoint> skippedPoints;

	wxDateTime now = wxDateTime::Now();

	if( m_prev_dist < 0.0005 )              // avoid zero length segs
		if( !do_add_point ) return;

	if( m_prev_time.IsValid() ) if( m_prev_time == now )                    // avoid zero time segs
		if( !do_add_point ) return;

	Vector2D gpsPoint( gLon, gLat );

	// The dynamic interval algorithm will gather all track points in a queue,
	// and analyze the cross track errors for each point before actually adding
	// a point to the track.

	switch( trackPointState ) { // FIXME: cases of switch contain too much code
		case firstPoint: {
			 RoutePoint *pTrackPoint = AddNewPoint( gpsPoint, now.ToUTC() );
			 m_lastStoredTP = pTrackPoint;
			 trackPointState = secondPoint;
			 do_add_point = false;
			 break;
			 }
		case secondPoint: {
			  Vector2D pPoint( gLon, gLat );
			  skipPoints.push_back( pPoint );
			  skipTimes.push_back( now.ToUTC() );
			  trackPointState = potentialPoint;
			  break;
			  }
		case potentialPoint: {
								 if( gpsPoint == skipPoints[skipPoints.size()-1] ) break;

								 unsigned int xteMaxIndex = 0;
								 double xteMax = 0;

								 // Scan points skipped so far and see if anyone has XTE over the threshold.
								 for( unsigned int i=0; i<skipPoints.size(); i++ ) {
									 double xte = GetXTE( m_lastStoredTP->m_lat, m_lastStoredTP->m_lon, gLat, gLon, skipPoints[i].lat, skipPoints[i].lon );
									 if( xte > xteMax ) {
										 xteMax = xte;
										 xteMaxIndex = i;
									 }
								 }
								 if( xteMax > m_allowedMaxXTE ) {
									 RoutePoint *pTrackPoint = AddNewPoint( skipPoints[xteMaxIndex], skipTimes[xteMaxIndex] );
									 pSelect->AddSelectableTrackSegment( m_lastStoredTP->m_lat, m_lastStoredTP->m_lon,
											 pTrackPoint->m_lat, pTrackPoint->m_lon,
											 m_lastStoredTP, pTrackPoint, this );

									 m_prevFixedTP = m_fixedTP;
									 m_fixedTP = m_removeTP;
									 m_removeTP = m_lastStoredTP;
									 m_lastStoredTP = pTrackPoint;
									 for( unsigned int i=0; i<=xteMaxIndex; i++ ) {
										 skipPoints.pop_front();
										 skipTimes.pop_front();
									 }

									 // Now back up and see if we just made 3 points in a straight line and the middle one
									 // (the next to last) point can possibly be eliminated. Here we reduce the allowed
									 // XTE as a function of leg length. (Half the XTE for very short legs).
									 if( GetnPoints() > 2 ) {
										 double dist = DistGreatCircle( m_fixedTP->m_lat, m_fixedTP->m_lon, m_lastStoredTP->m_lat, m_lastStoredTP->m_lon );
										 double xte = GetXTE( m_fixedTP, m_lastStoredTP, m_removeTP );
										 if( xte < m_allowedMaxXTE / wxMax(1.0, 2.0 - dist*2.0) ) {
											 pRoutePointList->pop_back();
											 pRoutePointList->pop_back();
											 pRoutePointList->push_back( m_lastStoredTP );
											 SetnPoints();
											 pSelect->DeletePointSelectableTrackSegments( m_removeTP );
											 pSelect->AddSelectableTrackSegment( m_fixedTP->m_lat, m_fixedTP->m_lon,
													 m_lastStoredTP->m_lat, m_lastStoredTP->m_lon,
													 m_fixedTP, m_lastStoredTP, this );
											 delete m_removeTP;
											 m_removeTP = m_fixedTP;
											 m_fixedTP = m_prevFixedTP;
										 }
									 }
								 }

								 skipPoints.push_back( gpsPoint );
								 skipTimes.push_back( now.ToUTC() );
								 break;
							 }
	}

	// Check if this is the last point of the track.
	if( do_add_point ) {
		RoutePoint *pTrackPoint = AddNewPoint( gpsPoint, now.ToUTC() );
		pSelect->AddSelectableTrackSegment( m_lastStoredTP->m_lat, m_lastStoredTP->m_lon,
				pTrackPoint->m_lat, pTrackPoint->m_lon,
				m_lastStoredTP, pTrackPoint, this );
	}

	m_prev_time = now;
}

void Track::Draw(ocpnDC& dc, ViewPort &VP)
{
	if (!IsVisible() || GetnPoints() == 0)
		return;

	double radius = 0.;
	if (g_bHighliteTracks) {
		double radius_meters = 20;
		radius = radius_meters * VP.view_scale_ppm;
	}

	unsigned short int FromSegNo = 1;


	wxRoutePointListNode *node = pRoutePointList->GetFirst();
	RoutePoint *prp = node->GetData();

	//  Establish basic colour
	wxColour basic_colour;
	if( m_bRunning || prp->m_IconName.StartsWith( _T("xmred") ) ) {
		basic_colour = GetGlobalColor( _T ( "URED" ) );
	} else
		if( prp->m_IconName.StartsWith( _T("xmblue") ) ) {
			basic_colour = GetGlobalColor( _T ( "BLUE3" ) );
		} else
			if( prp->m_IconName.StartsWith( _T("xmgreen") ) ) {
				basic_colour = GetGlobalColor( _T ( "UGREN" ) );
			} else {
				basic_colour = GetGlobalColor( _T ( "CHMGD" ) );
			}

	int style = wxSOLID;
	int width = g_route_line_width;
	wxColour col;
	if( m_style != STYLE_UNDEFINED )
		style = m_style;
	if( m_width != STYLE_UNDEFINED )
		width = m_width;
	if( m_Colour == wxEmptyString ) {
		col = basic_colour;
	} else {
		for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
			if( m_Colour == ::GpxxColorNames[i] ) {
				col = ::GpxxColors[i];
				break;
			}
		}
	}
	dc.SetPen(*wxThePenList->FindOrCreatePen(col, width, style));
	dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(col, wxSOLID));

	//  Draw the first point
	wxPoint rpt, rptn;
	DrawPointWhich( dc, 1, &rpt );

	node = node->GetNext();
	while( node ) {
		RoutePoint *prp = node->GetData();
		unsigned short int ToSegNo = prp->m_GPXTrkSegNo;

		wxPoint r;
		cc1->GetCanvasPointPix( prp->m_lat, prp->m_lon, &r );

		//  We do inline decomposition of the line segments, in a simple minded way
		//  If the line segment length is less than approximately 2 pixels, then simply don't render it,
		//  but continue on to the next point.
		if((abs(r.x - rpt.x) > 1) || (abs(r.y- rpt.y) > 1) ){
			prp->Draw( dc, &rptn );

			if( ToSegNo == FromSegNo )
				RenderSegment( dc, rpt.x, rpt.y, rptn.x, rptn.y, VP, false, (int) radius ); // no arrows, with hilite

			rpt = rptn;
		}

		node = node->GetNext();
		FromSegNo = ToSegNo;

	}

	//    Draw last segment, dynamically, maybe.....

	if( m_bRunning ) {
		wxPoint r;
		cc1->GetCanvasPointPix( gLat, gLon, &r );
		RenderSegment( dc, rpt.x, rpt.y, r.x, r.y, VP, false, (int) radius ); // no arrows, with hilite
	}
}

Route *Track::RouteFromTrack(wxProgressDialog *pprog)
{

	Route *route = new Route();
	wxRoutePointListNode *prpnode = pRoutePointList->GetFirst();
	RoutePoint *pWP_src = prpnode->GetData();
	wxRoutePointListNode *prpnodeX;
	RoutePoint *pWP_dst;
	RoutePoint *prp_OK = NULL;  // last routepoint known not to exceed xte limit, if not yet added

	wxString icon = _T("xmblue");
	if (g_TrackDeltaDistance >= 0.1) icon = _T("diamond");

	int ic = 0;
	int next_ic = 0;
	int back_ic = 0;
	int nPoints = pRoutePointList->GetCount();
	bool isProminent = true;
	double delta_dist, delta_hdg, xte;
	double leg_speed = 0.1;

	if (pRoutePropDialog)
		leg_speed = pRoutePropDialog->m_planspeed;
	else
		leg_speed = g_PlanSpeed;

	// add first point

	pWP_dst = new RoutePoint(pWP_src->m_lat, pWP_src->m_lon, icon, _T ( "" ));
	route->AddPoint( pWP_dst );

	pWP_dst->m_bShowName = false;

	pSelect->AddSelectableRoutePoint( pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst );

	// add intermediate points as needed

	prpnode = prpnode->GetNext();

	while( prpnode ) {
		RoutePoint *prp = prpnode->GetData();
		prpnodeX = prpnode;
		pWP_dst = pWP_src;

		delta_dist = 0.0;
		delta_hdg = 0.0;
		back_ic = next_ic;

		DistanceBearingMercator( prp->m_lat, prp->m_lon, pWP_src->m_lat, pWP_src->m_lon, &delta_hdg,
				&delta_dist );

		if( ( delta_dist > ( leg_speed * 6.0 ) ) && !prp_OK ) {
			int delta_inserts = floor( delta_dist / ( leg_speed * 4.0 ) );
			delta_dist = delta_dist / ( delta_inserts + 1 );
			double tlat = 0.0;
			double tlon = 0.0;

			while( delta_inserts-- ) {
				ll_gc_ll( pWP_src->m_lat, pWP_src->m_lon, delta_hdg, delta_dist, &tlat, &tlon );
				pWP_dst = new RoutePoint(tlat, tlon, icon, _T (""));
				route->AddPoint( pWP_dst );
				pWP_dst->m_bShowName = false;
				pSelect->AddSelectableRoutePoint( pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst );

				pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP_dst->m_lat,
						pWP_dst->m_lon, pWP_src, pWP_dst, route );

				pWP_src = pWP_dst;
			}
			prpnodeX = prpnode;
			pWP_dst = pWP_src;
			next_ic = 0;
			delta_dist = 0.0;
			back_ic = next_ic;
			prp_OK = prp;
			isProminent = true;
		} else {
			isProminent = false;
			if( delta_dist >= ( leg_speed * 4.0 ) ) isProminent = true;
			if( !prp_OK ) prp_OK = prp;
		}

		while( prpnodeX ) {

			RoutePoint *prpX = prpnodeX->GetData();
			xte = GetXTE( pWP_src, prpX, prp );
			if( isProminent || ( xte > g_TrackDeltaDistance ) ) {

				pWP_dst = new RoutePoint(prp_OK->m_lat, prp_OK->m_lon, icon, _T ( "" ));

				route->AddPoint( pWP_dst );
				pWP_dst->m_bShowName = false;

				pSelect->AddSelectableRoutePoint( pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst );

				pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP_dst->m_lat,
						pWP_dst->m_lon, pWP_src, pWP_dst, route );

				pWP_src = pWP_dst;
				next_ic = 0;
				prpnodeX = NULL;
				prp_OK = NULL;
			}

			if( prpnodeX ) prpnodeX = prpnodeX->GetPrevious();
			if( back_ic-- <= 0 ) {
				prpnodeX = NULL;
			}
		}

		if( prp_OK ) {
			prp_OK = prp;
		}

		DistanceBearingMercator( prp->m_lat, prp->m_lon, pWP_src->m_lat, pWP_src->m_lon, NULL,
				&delta_dist );

		if( !( ( delta_dist > ( g_TrackDeltaDistance ) ) && !prp_OK ) ) {
			prpnode = prpnode->GetNext(); //RoutePoint
			next_ic++;
		}
		ic++;
		if( pprog ) pprog->Update( ( ic * 100 ) / nPoints );
	}

	// add last point, if needed
	if( delta_dist >= g_TrackDeltaDistance ) {
		pWP_dst = new RoutePoint(pRoutePointList->GetLast()->GetData()->m_lat,
				pRoutePointList->GetLast()->GetData()->m_lon, icon, _T ( "" ));
		route->AddPoint( pWP_dst );

		pWP_dst->m_bShowName = false;

		pSelect->AddSelectableRoutePoint( pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst );

		pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP_dst->m_lat,
				pWP_dst->m_lon, pWP_src, pWP_dst, route );
	}
	route->m_RouteNameString = m_RouteNameString;
	route->m_RouteStartString = m_RouteStartString;
	route->m_RouteEndString = m_RouteEndString;
	route->m_bDeleteOnArrival = false;

	return route;
}

void Track::DouglasPeuckerReducer( std::vector<RoutePoint*>& list, int from, int to, double delta )
{
	list[from]->m_bIsActive = true;
	list[to]->m_bIsActive = true;

	int maxdistIndex = -1;
	double maxdist = 0;

	for( int i=from+1; i<to; i++ ) {

		double dist = 1852.0 * GetXTE( list[from], list[to], list[i] );

		if( dist > maxdist ) {
			maxdist = dist;
			maxdistIndex = i;
		}
	}

	if( maxdist > delta ) {
		DouglasPeuckerReducer( list, from, maxdistIndex, delta );
		DouglasPeuckerReducer( list, maxdistIndex, to, delta );
	}
}

int Track::Simplify( double maxDelta )
{
	int reduction = 0;
	wxRoutePointListNode *pointnode = pRoutePointList->GetFirst();
	RoutePoint *routepoint;
	std::vector<RoutePoint*> pointlist;

	::wxBeginBusyCursor();

	while( pointnode ) {
		routepoint = pointnode->GetData();
		routepoint->m_bIsActive = false;
		pointlist.push_back(routepoint);
		pointnode = pointnode->GetNext();
	}

	DouglasPeuckerReducer( pointlist, 0, pointlist.size()-1, maxDelta );

	pSelect->DeleteAllSelectableTrackSegments( this );
	pRoutePointList->Clear();

	for( size_t i=0; i<pointlist.size(); i++ ) {
		if( pointlist[i]->m_bIsActive ) {
			pointlist[i]->m_bIsActive = false;
			pRoutePointList->Append( pointlist[i] );
		} else {
			delete pointlist[i];
			reduction++;
		}
	}

	SetnPoints();
	pSelect->AddAllSelectableTrackSegments( this );

	UpdateSegmentDistances();
	::wxEndBusyCursor();
	return reduction;
}

double Track::GetXTE( double fm1Lat, double fm1Lon, double fm2Lat, double fm2Lon, double toLat, double toLon  )
{
	Vector2D v, w, p;

	// First we get the cartesian coordinates to the line endpoints, using
	// the current position as origo.

	double brg1, dist1, brg2, dist2;
	DistanceBearingMercator( toLat, toLon, fm1Lat, fm1Lon, &brg1, &dist1 );
	w.x = dist1 * sin( brg1 * M_PI / 180. );
	w.y = dist1 * cos( brg1 * M_PI / 180. );

	DistanceBearingMercator( toLat, toLon, fm2Lat, fm2Lon, &brg2, &dist2 );
	v.x = dist2 * sin( brg2 * M_PI / 180. );
	v.y = dist2 * cos( brg2 * M_PI / 180. );

	p.x = 0.0; p.y = 0.0;

	const double lengthSquared = _distance2( v, w );
	if ( lengthSquared == 0.0 ) {
		// v == w case
		return _distance( p, v );
	}

	// Consider the line extending the segment, parameterized as v + t (w - v).
	// We find projection of origo onto the line.
	// It falls where t = [(p-v) . (w-v)] / |w-v|^2

	Vector2D a = p - v;
	Vector2D b = w - v;

	double t = vDotProduct( &a, &b ) / lengthSquared;

	if (t < 0.0) return _distance(p, v);       // Beyond the 'v' end of the segment
	else if (t > 1.0) return _distance(p, w);  // Beyond the 'w' end of the segment
	Vector2D projection = v + t * (w - v);     // Projection falls on the segment
	return _distance(p, projection);
}

double Track::GetXTE( RoutePoint *fm1, RoutePoint *fm2, RoutePoint *to )
{
	if( !fm1 || !fm2 || !to ) return 0.0;
	if( fm1 == to ) return 0.0;
	if( fm2 == to ) return 0.0;
	return GetXTE( fm1->m_lat, fm1->m_lon, fm2->m_lat, fm2->m_lon, to->m_lat, to->m_lon );
	;
}

