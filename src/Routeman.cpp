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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
	#include "wx/wx.h"
#endif //precompiled headers

#include <wx/image.h>
#include <wx/tokenzr.h>
#include <wx/progdlg.h>
#include <wx/listimpl.cpp>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/apptrait.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "Routeman.h"
#include "dychart.h"
#include "styles.h"
#include "CDI.h"
#include "WayPointman.h"
#include "ConsoleCanvas.h"
#include "navutil.h"
#include "georef.h"
#include "RouteProp.h"
#include "RouteManagerDialog.h"
#include "pluginmanager.h"
#include "Multiplexer.h"
#include "Select.h"
#include "MarkIcon.h"

extern ConsoleCanvas * console;
extern RouteList * pRouteList;
extern Select * pSelect;
extern MyConfig * pConfig;

extern wxRect g_blink_rect;
extern wxString g_SData_Locn;
extern wxString g_PrivateDataDir;

extern double gLat;
extern double gLon;
extern double gSog;
extern double gCog;
extern double gVar;

extern Track * g_pActiveTrack;
extern RouteProp * pRoutePropDialog;
extern RouteManagerDialog * pRouteManagerDialog;
extern int g_route_line_width;
extern Multiplexer * g_pMUX;

extern PlugInManager * g_pi_manager;
extern ocpnStyle::StyleManager * g_StyleManager;
extern wxString g_uploadConnection;

class markicon_bitmap_list_type;
class markicon_key_list_type;
class markicon_description_list_type;

//    List definitions for Waypoint Manager Icons
WX_DECLARE_LIST(wxBitmap, markicon_bitmap_list_type);
WX_DECLARE_LIST(wxString, markicon_key_list_type);
WX_DECLARE_LIST(wxString, markicon_description_list_type);

//    List implementation for Waypoint Manager Icons
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(markicon_bitmap_list_type);
WX_DEFINE_LIST(markicon_key_list_type);
WX_DEFINE_LIST(markicon_description_list_type);


Routeman::Routeman( MyApp *parent )
{
	m_pparent_app = parent;
	pActiveRoute = NULL;
	pActivePoint = NULL;
	pRouteActivatePoint = NULL;
}

Routeman::~Routeman()
{
	if( pRouteActivatePoint ) delete pRouteActivatePoint;
}

bool Routeman::IsRouteValid( Route *pRoute )
{
	wxRouteListNode *node = pRouteList->GetFirst();
	while( node ) {
		if( pRoute == node->GetData() ) return true;
		node = node->GetNext();
	}
	return false;
}

//    Make a 2-D search to find the route containing a given waypoint
Route *Routeman::FindRouteContainingWaypoint( RoutePoint *pWP )
{
	wxRouteListNode *node = pRouteList->GetFirst();
	while( node ) {
		Route *proute = node->GetData();

		wxRoutePointListNode *pnode = ( proute->pRoutePointList )->GetFirst();
		while( pnode ) {
			RoutePoint *prp = pnode->GetData();
			if( prp == pWP )  return proute;
			pnode = pnode->GetNext();
		}

		node = node->GetNext();
	}

	return NULL;                              // not found
}

wxArrayPtrVoid *Routeman::GetRouteArrayContaining( RoutePoint *pWP )
{
	wxArrayPtrVoid *pArray = new wxArrayPtrVoid;

	wxRouteListNode *route_node = pRouteList->GetFirst();
	while( route_node ) {
		Route *proute = route_node->GetData();

		wxRoutePointListNode *waypoint_node = ( proute->pRoutePointList )->GetFirst();
		while( waypoint_node ) {
			RoutePoint *prp = waypoint_node->GetData();
			if( prp == pWP )                // success
				pArray->Add( (void *) proute );

			waypoint_node = waypoint_node->GetNext();           // next waypoint
		}

		route_node = route_node->GetNext();                         // next route
	}

	if( pArray->GetCount() ) return pArray;

	else {
		delete pArray;
		return NULL;
	}
}

RoutePoint *Routeman::FindBestActivatePoint( Route *pR, double lat, double lon, double cog,
		double sog )
{
	if( !pR ) return NULL;

	// Walk thru all the points to find the "best"
	RoutePoint *best_point = NULL;
	double min_time_found = 1e6;

	wxRoutePointListNode *node = ( pR->pRoutePointList )->GetFirst();
	while( node ) {
		RoutePoint *pn = node->GetData();

		double brg, dist;
		DistanceBearingMercator( pn->m_lat, pn->m_lon, lat, lon, &brg, &dist );

		double angle = brg - cog;
		double soa = cos( angle * PI / 180. );

		double time_to_wp = dist / soa;

		if( time_to_wp > 0 ) {
			if( time_to_wp < min_time_found ) {
				min_time_found = time_to_wp;
				best_point = pn;
			}
		}
		node = node->GetNext();
	}
	return best_point;
}

bool Routeman::ActivateRoute( Route *pRouteToActivate, RoutePoint *pStartPoint )
{
	pActiveRoute = pRouteToActivate;

	if( pStartPoint ) {
		pActivePoint = pStartPoint;
	} else {
		wxRoutePointListNode *node = ( pActiveRoute->pRoutePointList )->GetFirst();
		pActivePoint = node->GetData();               // start at beginning
	}

	wxJSONValue v;
	v[_T("Route_activated")] = pRouteToActivate->m_RouteNameString;
	v[_T("GUID")] = pRouteToActivate->m_GUID;
	wxString msg_id( _T("OCPN_RTE_ACTIVATED") );
	g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );

	ActivateRoutePoint( pRouteToActivate, pActivePoint );

	m_bArrival = false;

	pRouteToActivate->m_bRtIsActive = true;

	m_bDataValid = false;

	console->ShowWithFreshFonts();

	return true;
}

bool Routeman::ActivateRoutePoint( Route *pA, RoutePoint *pRP_target )
{
	wxJSONValue v;
	pActiveRoute = pA;

	pActivePoint = pRP_target;
	pActiveRoute->m_pRouteActivePoint = pRP_target;

	v[_T("GUID")] = pRP_target->m_GUID;
	v[_T("WP_activated")] = pRP_target->GetName();

	wxRoutePointListNode *node = ( pActiveRoute->pRoutePointList )->GetFirst();
	while( node ) {
		RoutePoint *pn = node->GetData();
		pn->m_bBlink = false;                     // turn off all blinking points
		pn->m_bIsActive = false;

		node = node->GetNext();
	}

	node = ( pActiveRoute->pRoutePointList )->GetFirst();
	RoutePoint *prp_first = node->GetData();

	//  If activating first point in route, create a "virtual" waypoint at present position
	if( pRP_target == prp_first ) {
		if( pRouteActivatePoint ) delete pRouteActivatePoint;

		pRouteActivatePoint = new RoutePoint( gLat, gLon, wxString( _T("") ), wxString( _T("") ),
				GPX_EMPTY_STRING, false ); // Current location
		pRouteActivatePoint->m_bShowName = false;

		pActiveRouteSegmentBeginPoint = pRouteActivatePoint;
	}

	else {
		prp_first->m_bBlink = false;
		node = node->GetNext();
		RoutePoint *np_prev = prp_first;
		while( node ) {
			RoutePoint *pnext = node->GetData();
			if( pnext == pRP_target ) {
				pActiveRouteSegmentBeginPoint = np_prev;
				break;
			}

			np_prev = pnext;
			node = node->GetNext();
		}
	}

	pRP_target->m_bBlink = true;                               // blink the active point
	pRP_target->m_bIsActive = true;                            // and active

	g_blink_rect = pRP_target->CurrentRect_in_DC;               // set up global blinker

	m_bArrival = false;

	//    Update the RouteProperties Dialog, if currently shown
	if( ( NULL != pRoutePropDialog ) && ( pRoutePropDialog->IsShown() ) ) {
		if( pRoutePropDialog->m_pRoute == pA ) {
			if( pRoutePropDialog->m_pEnroutePoint ) pRoutePropDialog->m_pEnroutePoint =
				pActivePoint;
			pRoutePropDialog->SetRouteAndUpdate( pA );
			pRoutePropDialog->UpdateProperties();
		}
	}

	wxString msg_id( _T("OCPN_WPT_ACTIVATED") );
	g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );

	return true;
}

bool Routeman::ActivateNextPoint( Route *pr, bool skipped )
{
	wxJSONValue v;
	if( pActivePoint ) {
		pActivePoint->m_bBlink = false;
		pActivePoint->m_bIsActive = false;

		v[_T("isSkipped")] = skipped;
		v[_T("GUID")] = pActivePoint->m_GUID;
		v[_T("WP_arrived")] = pActivePoint->GetName();
	}
	int n_index_active = pActiveRoute->GetIndexOf( pActivePoint );
	if( ( n_index_active + 1 ) <= pActiveRoute->GetnPoints() ) {
		pActiveRouteSegmentBeginPoint = pActivePoint;

		pActiveRoute->m_pRouteActivePoint = pActiveRoute->GetPoint( n_index_active + 1 );

		pActivePoint = pActiveRoute->GetPoint( n_index_active + 1 );
		v[_T("Next_WP")] = pActivePoint->GetName();
		v[_T("GUID")] = pActivePoint->m_GUID;

		pActivePoint->m_bBlink = true;
		pActivePoint->m_bIsActive = true;
		g_blink_rect = pActivePoint->CurrentRect_in_DC;               // set up global blinker

		m_bArrival = false;

		//    Update the RouteProperties Dialog, if currently shown
		if( ( NULL != pRoutePropDialog ) && ( pRoutePropDialog->IsShown() ) ) {
			if( pRoutePropDialog->m_pRoute == pr ) {
				if( pRoutePropDialog->m_pEnroutePoint ) pRoutePropDialog->m_pEnroutePoint =
					pActivePoint;
				pRoutePropDialog->SetRouteAndUpdate( pr );
				pRoutePropDialog->UpdateProperties();
			}
		}

		wxString msg_id( _T("OCPN_WPT_ARRIVED") );
		g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );

		return true;
	}

	return false;
}

bool Routeman::UpdateProgress()
{
	bool bret_val = false;

	if( pActiveRoute ) {
		//      Update bearing, range, and crosstrack error

		//  Bearing is calculated as Mercator Sailing, i.e. a  cartographic "bearing"
		double north, east;
		toSM( pActivePoint->m_lat, pActivePoint->m_lon, gLat, gLon, &east, &north );
		double a = atan( north / east );
		if( fabs( pActivePoint->m_lon - gLon ) < 180. ) {
			if( pActivePoint->m_lon > gLon ) CurrentBrgToActivePoint = 90. - ( a * 180 / PI );
			else
				CurrentBrgToActivePoint = 270. - ( a * 180 / PI );
		} else {
			if( pActivePoint->m_lon > gLon ) CurrentBrgToActivePoint = 270. - ( a * 180 / PI );
			else
				CurrentBrgToActivePoint = 90. - ( a * 180 / PI );
		}

		//      Calculate range using Great Circle Formula

		double d5 = DistGreatCircle( gLat, gLon, pActivePoint->m_lat, pActivePoint->m_lon );
		CurrentRngToActivePoint = d5;

		//      Get the XTE vector, normal to current segment
		vector2D va, vb, vn;

		double brg1, dist1, brg2, dist2;
		DistanceBearingMercator( pActivePoint->m_lat, pActivePoint->m_lon,
				pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon, &brg1,
				&dist1 );
		vb.x = dist1 * sin( brg1 * PI / 180. );
		vb.y = dist1 * cos( brg1 * PI / 180. );

		DistanceBearingMercator( pActivePoint->m_lat, pActivePoint->m_lon, gLat, gLon, &brg2,
				&dist2 );
		va.x = dist2 * sin( brg2 * PI / 180. );
		va.y = dist2 * cos( brg2 * PI / 180. );

		double sdelta = vGetLengthOfNormal( &va, &vb, &vn );             // NM
		CurrentXTEToActivePoint = sdelta;

		//    Calculate the distance to the arrival line, which is perpendicular to the current route segment
		//    Taking advantage of the calculated normal from current position to route segment vn
		vector2D vToArriveNormal;
		vSubtractVectors( &va, &vn, &vToArriveNormal );

		CurrentRangeToActiveNormalCrossing = vVectorMagnitude( &vToArriveNormal );

		//          Compute current segment course
		//          Using simple Mercater projection
		double x1, y1, x2, y2;
		toSM( pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon,
				pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon, &x1,
				&y1 );

		toSM( pActivePoint->m_lat, pActivePoint->m_lon, pActiveRouteSegmentBeginPoint->m_lat,
				pActiveRouteSegmentBeginPoint->m_lon, &x2, &y2 );

		double e1 = atan2( ( x2 - x1 ), ( y2 - y1 ) );
		CurrentSegmentCourse = e1 * 180 / PI;
		if( CurrentSegmentCourse < 0 ) CurrentSegmentCourse += 360;

		//      Compute XTE direction
		double h = atan( vn.y / vn.x );
		if( vn.x > 0 ) CourseToRouteSegment = 90. - ( h * 180 / PI );
		else
			CourseToRouteSegment = 270. - ( h * 180 / PI );

		h = CurrentBrgToActivePoint - CourseToRouteSegment;
		if( h < 0 ) h = h + 360;

		if( h > 180 ) XTEDir = 1;
		else
			XTEDir = -1;

		//      Determine Arrival

		bool bDidArrival = false;

		if( CurrentRangeToActiveNormalCrossing <= pActiveRoute->GetRouteArrivalRadius() ) {
			m_bArrival = true;
			UpdateAutopilot();

			bDidArrival = true;

			if( !ActivateNextPoint( pActiveRoute, false ) )            // at the end?
			{
				Route *pthis_route = pActiveRoute;
				DeactivateRoute( true );                  // this is an arrival
				if( pthis_route->m_bDeleteOnArrival ) {
					pConfig->DeleteConfigRoute( pthis_route );
					DeleteRoute( pthis_route );
					if( pRoutePropDialog ) {
						pRoutePropDialog->SetRouteAndUpdate( NULL );
						pRoutePropDialog->UpdateProperties();
					}
					if( pRouteManagerDialog ) pRouteManagerDialog->UpdateRouteListCtrl();

				}
			}

		}

		if( !bDidArrival )                                        // Only once on arrival
			UpdateAutopilot();

		bret_val = true;                                        // a route is active
	}

	m_bDataValid = true;

	return bret_val;
}

bool Routeman::DeactivateRoute( bool b_arrival )
{
	if( pActivePoint ) {
		pActivePoint->m_bBlink = false;
		pActivePoint->m_bIsActive = false;
	}

	if( pActiveRoute ) {
		pActiveRoute->m_bRtIsActive = false;
		pActiveRoute->m_pRouteActivePoint = NULL;
	}

	wxJSONValue v;
	if( !b_arrival ) {
		v[_T("Route_deactivated")] = pActiveRoute->m_RouteNameString;
		v[_T("GUID")] = pActiveRoute->m_GUID;
		wxString msg_id( _T("OCPN_RTE_DEACTIVATED") );
		g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
	} else {
		v[_T("GUID")] = pActiveRoute->m_GUID;
		v[_T("Route_ended")] = pActiveRoute->m_RouteNameString;
		wxString msg_id( _T("OCPN_RTE_ENDED") );
		g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
	}

	pActiveRoute = NULL;

	if( pRouteActivatePoint ) delete pRouteActivatePoint;
	pRouteActivatePoint = NULL;

	console->pCDI->ClearBackground();

	console->Show( false );

	m_bDataValid = false;

	return true;
}

bool Routeman::UpdateAutopilot()
{
	//Send all known Autopilot messages upstream

	//RMB
	{

		m_NMEA0183.TalkerID = _T("EC");

		SENTENCE snt;
		m_NMEA0183.Rmb.IsDataValid = NTrue;
		m_NMEA0183.Rmb.CrossTrackError = CurrentXTEToActivePoint;

		if( XTEDir < 0 ) m_NMEA0183.Rmb.DirectionToSteer = Left;
		else
			m_NMEA0183.Rmb.DirectionToSteer = Right;

		m_NMEA0183.Rmb.To = pActivePoint->GetName().Truncate( 6 );
		m_NMEA0183.Rmb.From = pActiveRouteSegmentBeginPoint->GetName().Truncate( 6 );

		if( pActivePoint->m_lat < 0. ) m_NMEA0183.Rmb.DestinationPosition.Latitude.Set(
				-pActivePoint->m_lat, _T("S") );
		else
			m_NMEA0183.Rmb.DestinationPosition.Latitude.Set( pActivePoint->m_lat, _T("N") );

		if( pActivePoint->m_lon < 0. ) m_NMEA0183.Rmb.DestinationPosition.Longitude.Set(
				-pActivePoint->m_lon, _T("W") );
		else
			m_NMEA0183.Rmb.DestinationPosition.Longitude.Set( pActivePoint->m_lon, _T("E") );

		m_NMEA0183.Rmb.RangeToDestinationNauticalMiles = CurrentRngToActivePoint;
		m_NMEA0183.Rmb.BearingToDestinationDegreesTrue = CurrentBrgToActivePoint;
		m_NMEA0183.Rmb.DestinationClosingVelocityKnots = gSog;

		if( m_bArrival ) m_NMEA0183.Rmb.IsArrivalCircleEntered = NTrue;
		else
			m_NMEA0183.Rmb.IsArrivalCircleEntered = NFalse;

		m_NMEA0183.Rmb.Write( snt );

		g_pMUX->SendNMEAMessage( snt.Sentence );
	}

	// RMC
	{

		m_NMEA0183.TalkerID = _T("EC");

		SENTENCE snt;
		m_NMEA0183.Rmc.IsDataValid = NTrue;

		if( gLat < 0. ) m_NMEA0183.Rmc.Position.Latitude.Set( -gLat, _T("S") );
		else
			m_NMEA0183.Rmc.Position.Latitude.Set( gLat, _T("N") );

		if( gLon < 0. ) m_NMEA0183.Rmc.Position.Longitude.Set( -gLon, _T("W") );
		else
			m_NMEA0183.Rmc.Position.Longitude.Set( gLon, _T("E") );

		m_NMEA0183.Rmc.SpeedOverGroundKnots = gSog;
		m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue = gCog;

		if( !wxIsNaN(gVar) ) {
			if( gVar < 0. ) {
				m_NMEA0183.Rmc.MagneticVariation = -gVar;
				m_NMEA0183.Rmc.MagneticVariationDirection = West;
			} else {
				m_NMEA0183.Rmc.MagneticVariation = gVar;
				m_NMEA0183.Rmc.MagneticVariationDirection = East;
			}
		} else
			m_NMEA0183.Rmc.MagneticVariation = 361.; // A signal to NMEA converter, gVAR is unknown

		wxDateTime now = wxDateTime::Now();
		wxDateTime utc = now.ToUTC();
		wxString time = utc.Format( _T("%H%M%S") );
		m_NMEA0183.Rmc.UTCTime = time;

		wxString date = utc.Format( _T("%d%m%y") );
		m_NMEA0183.Rmc.Date = date;

		m_NMEA0183.Rmc.Write( snt );

		g_pMUX->SendNMEAMessage( snt.Sentence );
	}

	// APB
	{
		m_NMEA0183.TalkerID = _T("EC");

		SENTENCE snt;

		m_NMEA0183.Apb.IsLoranBlinkOK = NTrue;
		m_NMEA0183.Apb.IsLoranCCycleLockOK = NTrue;

		m_NMEA0183.Apb.CrossTrackErrorMagnitude = CurrentXTEToActivePoint;

		if( XTEDir < 0 ) m_NMEA0183.Apb.DirectionToSteer = Left;
		else
			m_NMEA0183.Apb.DirectionToSteer = Right;

		m_NMEA0183.Apb.CrossTrackUnits = _T("N");

		if( m_bArrival )
			m_NMEA0183.Apb.IsArrivalCircleEntered = NTrue;
		else
			m_NMEA0183.Apb.IsArrivalCircleEntered = NFalse;

		//  We never pass the perpendicular, since we declare arrival before reaching this point
		m_NMEA0183.Apb.IsPerpendicular = NFalse;

		double brg1, dist1;
		DistanceBearingMercator( pActivePoint->m_lat, pActivePoint->m_lon,
				pActiveRouteSegmentBeginPoint->m_lat, pActiveRouteSegmentBeginPoint->m_lon,
				&brg1,
				&dist1 );

		m_NMEA0183.Apb.BearingOriginToDestination = brg1;
		m_NMEA0183.Apb.BearingOriginToDestinationUnits = _("T");

		m_NMEA0183.Apb.To = pActivePoint->GetName().Truncate( 6 );

		m_NMEA0183.Apb.BearingPresentPositionToDestination = CurrentBrgToActivePoint;
		m_NMEA0183.Apb.BearingPresentPositionToDestinationUnits = _("T");

		m_NMEA0183.Apb.To = pActivePoint->GetName().Truncate( 6 );

		m_NMEA0183.Apb.HeadingToSteer = CurrentBrgToActivePoint;
		m_NMEA0183.Apb.HeadingToSteerUnits = _("T");

		m_NMEA0183.Apb.Write( snt );
		g_pMUX->SendNMEAMessage( snt.Sentence );
	}


	return true;
}

bool Routeman::DoesRouteContainSharedPoints( Route *pRoute )
{
	if( pRoute ) {
		// walk the route, looking at each point to see if it is used by another route
		// or is isolated
		wxRoutePointListNode *pnode = ( pRoute->pRoutePointList )->GetFirst();
		while( pnode ) {
			RoutePoint *prp = pnode->GetData();

			// check all other routes to see if this point appears in any other route
			wxArrayPtrVoid *pRA = GetRouteArrayContaining( prp );

			if( pRA ) {
				for( unsigned int ir = 0; ir < pRA->GetCount(); ir++ ) {
					Route *pr = (Route *) pRA->Item( ir );
					if( pr == pRoute)
						continue;               // self
					else
						return true;
				}
			}

			if( pnode ) pnode = pnode->GetNext();
		}

		//      Now walk the route again, looking for isolated type shared waypoints
		pnode = ( pRoute->pRoutePointList )->GetFirst();
		while( pnode ) {
			RoutePoint *prp = pnode->GetData();
			if( prp->m_bKeepXRoute == true )
				return true;

			if( pnode ) pnode = pnode->GetNext();
		}
	}

	return false;
}



void Routeman::DeleteRoute( Route *pRoute )
{
	if( pRoute ) {
		::wxBeginBusyCursor();

		if( GetpActiveRoute() == pRoute ) DeactivateRoute();

		if( pRoute->m_bIsInLayer ) return;

		//    Remove the route from associated lists
		pSelect->DeleteAllSelectableRouteSegments( pRoute );
		pRouteList->DeleteObject( pRoute );

		// walk the route, tentatively deleting/marking points used only by this route
		wxRoutePointListNode *pnode = ( pRoute->pRoutePointList )->GetFirst();
		while( pnode ) {
			RoutePoint *prp = pnode->GetData();

			// check all other routes to see if this point appears in any other route
			Route *pcontainer_route = FindRouteContainingWaypoint( prp );

			if( pcontainer_route == NULL && prp->m_bIsInRoute ) {
				prp->m_bIsInRoute = false;          // Take this point out of this (and only) route
				if( !prp->m_bKeepXRoute ) {
					//    This does not need to be done with navobj.xml storage, since the waypoints are stored with the route
					//                              pConfig->DeleteWayPoint(prp);

					pSelect->DeleteSelectablePoint( prp, Select::TYPE_ROUTEPOINT );

					// Remove all instances of this point from the list.
					wxRoutePointListNode *pdnode = pnode;
					while( pdnode ) {
						pRoute->pRoutePointList->DeleteNode( pdnode );
						pdnode = pRoute->pRoutePointList->Find( prp );
					}

					pnode = NULL;
					delete prp;
				} else {
					prp->m_bDynamicName = false;
					prp->m_bIsolatedMark = true;        // This has become an isolated mark
					prp->m_bKeepXRoute = false;         // and is no longer part of a route
				}

			}
			if( pnode ) pnode = pnode->GetNext();
			else
				pnode = pRoute->pRoutePointList->GetFirst();                // restart the list
		}

		delete pRoute;

		::wxEndBusyCursor();

	}
}

void Routeman::DeleteAllRoutes( void )
{
	::wxBeginBusyCursor();

	//    Iterate on the RouteList
	wxRouteListNode *node = pRouteList->GetFirst();
	while( node ) {
		Route *proute = node->GetData();

		if( proute->m_bIsInLayer ) {
			node = node->GetNext();
			continue;
		}

		if( !proute->m_bIsTrack ) {
			pConfig->m_bIsImporting = true;
			pConfig->DeleteConfigRoute( proute );
			DeleteRoute( proute );
			node = pRouteList->GetFirst();                   // Route
			pConfig->m_bIsImporting = false;
		} else
			node = node->GetNext();
	}

	::wxEndBusyCursor();

}

void Routeman::DeleteAllTracks( void )
{
	::wxBeginBusyCursor();

	//    Iterate on the RouteList
	wxRouteListNode *node = pRouteList->GetFirst();
	while( node ) {
		Route *proute = node->GetData();

		if( proute->m_bIsInLayer ) {
			node = node->GetNext();
			continue;
		}

		if( proute->m_bIsTrack ) {
			pConfig->m_bIsImporting = true;
			pConfig->DeleteConfigRoute( proute );
			DeleteTrack( proute );
			node = pRouteList->GetFirst();                   // Route
			pConfig->m_bIsImporting = false;
		} else
			node = node->GetNext();
	}

	::wxEndBusyCursor();

}

void Routeman::DeleteTrack( Route *pRoute )
{
	if( pRoute ) {
		if( pRoute->m_bIsInLayer ) return;

		::wxBeginBusyCursor();

		wxProgressDialog *pprog = NULL;
		int count = pRoute->pRoutePointList->GetCount();
		if( count > 200) {
			pprog = new wxProgressDialog( _("OpenCPN Track Delete"), _T("0/0"), count, NULL,
					wxPD_APP_MODAL | wxPD_SMOOTH |
					wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME );
			pprog->SetSize( 400, wxDefaultCoord );
			pprog->Centre();

		}

		//    Remove the route from associated lists
		pSelect->DeleteAllSelectableTrackSegments( pRoute );
		pRouteList->DeleteObject( pRoute );

		// walk the route, tentatively deleting/marking points used only by this route
		int ic = 0;
		wxRoutePointListNode *pnode = ( pRoute->pRoutePointList )->GetFirst();
		while( pnode ) {
			if(pprog) {
				wxString msg;
				msg.Printf(_T("%d/%d"), ic, count);
				pprog->Update( ic, msg );
				ic++;
			}

			RoutePoint *prp = pnode->GetData();


			// check all other routes to see if this point appears in any other route
			Route *pcontainer_route = NULL; //FindRouteContainingWaypoint(prp);

			if( pcontainer_route == NULL ) {
				prp->m_bIsInRoute = false;          // Take this point out of this (and only) route
				if( !prp->m_bKeepXRoute ) {
					pConfig->m_bIsImporting = true;
					pConfig->DeleteWayPoint( prp );
					pSelect->DeleteSelectablePoint( prp, Select::TYPE_ROUTEPOINT );
					pConfig->m_bIsImporting = false;

					pRoute->pRoutePointList->DeleteNode( pnode );
					/*
					// Remove all instances of this point from the list.
					wxRoutePointListNode *pdnode = pnode;
					while(pdnode)
					{
					pRoute->pRoutePointList->DeleteNode(pdnode);
					pdnode = pRoute->pRoutePointList->Find(prp);
					}
					 */
					pnode = NULL;
					delete prp;
				}

			}
			if( pnode ) pnode = pnode->GetNext();
			else
				pnode = pRoute->pRoutePointList->GetFirst();                // restart the list
		}

		if( (Track *) pRoute == g_pActiveTrack ) {
			g_pActiveTrack = NULL;
			m_pparent_app->TrackOff();
		}

		delete pRoute;

		::wxEndBusyCursor();

		if( pprog)
			delete pprog;
	}
}

void Routeman::SetColorScheme( ColorScheme cs )
{
	m_pActiveRoutePointPen = wxThePenList->FindOrCreatePen( wxColour( 0, 0, 255 ), g_route_line_width, wxSOLID );
	m_pRoutePointPen = wxThePenList->FindOrCreatePen( wxColour( 0, 0, 255 ), g_route_line_width, wxSOLID );

	//    Or in something like S-52 compliance

	m_pRoutePen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T("UINFB") ), g_route_line_width,
			wxSOLID );
	m_pSelectedRoutePen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T("UINFO") ), g_route_line_width, wxSOLID );
	m_pActiveRoutePen = wxThePenList->FindOrCreatePen( GetGlobalColor( _T("UARTE") ), g_route_line_width, wxSOLID );

	m_pRouteBrush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("UINFB") ), wxSOLID );
	m_pSelectedRouteBrush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("UINFO") ), wxSOLID );
	m_pActiveRouteBrush = wxTheBrushList->FindOrCreateBrush( GetGlobalColor( _T("PLRTE") ), wxSOLID );
}

wxString Routeman::GetRouteReverseMessage( void )
{
	return wxString(
			_("Waypoints can be renamed to reflect the new order, the names will be '001', '002' etc.\n\nDo you want to rename the waypoints?") );
}

Route *Routeman::FindRouteByGUID(wxString &guid)
{
	Route *pRoute = NULL;
	wxRouteListNode *node1 = pRouteList->GetFirst();
	while( node1 ) {
		pRoute = node1->GetData();

		if( pRoute->m_GUID == guid )
			break;
		node1 = node1->GetNext();
	}

	return pRoute;
}

