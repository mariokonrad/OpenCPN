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

#include "NavObjectChanges.h"
#include <RoutePoint.h>
#include <Routeman.h>
#include <Track.h>
#include <WayPointman.h>
#include <Select.h>

extern WayPointman* pWayPointMan;
extern Select* pSelect;
extern Routeman* g_pRouteMan;

NavObjectChanges::NavObjectChanges()
	: NavObjectCollection()
{
}

NavObjectChanges::~NavObjectChanges()
{
}

bool NavObjectChanges::AddRoute(Route* pr, const char* action)
{
	SetRootGPXNode();

	pugi::xml_node object = m_gpx_root.append_child("rte");
	GPXCreateRoute(object, pr);

	pugi::xml_node xchild = object.child("extensions");
	// FIXME  What if extensions do not exist?
	pugi::xml_node child = xchild.append_child("opencpn:action");
	child.append_child(pugi::node_pcdata).set_value(action);

	return true;
}

bool NavObjectChanges::AddTrack(Track* pr, const char* action)
{
	SetRootGPXNode();

	pugi::xml_node object = m_gpx_root.append_child("trk");
	GPXCreateTrk(object, pr);

	pugi::xml_node xchild = object.child("extensions");
	pugi::xml_node child = xchild.append_child("opencpn:action");
	child.append_child(pugi::node_pcdata).set_value(action);

	return true;
}

bool NavObjectChanges::AddWP(RoutePoint* pWP, const char* action)
{
	SetRootGPXNode();

	pugi::xml_node object = m_gpx_root.append_child("wpt");
	GPXCreateWpt(object, pWP, OPT_WPT);

	pugi::xml_node xchild = object.child("extensions");
	pugi::xml_node child = xchild.append_child("opencpn:action");
	child.append_child(pugi::node_pcdata).set_value(action);

	return true;
}

bool NavObjectChanges::ApplyChanges(void)
{
	// Let's reconstruct the unsaved changes

	pugi::xml_node objects = this->child("gpx");

	for (pugi::xml_node object = objects.first_child(); object; object = object.next_sibling()) {
		if (!strcmp(object.name(), "wpt")) {
			RoutePoint* pWp
				= GPXLoadWaypoint1(object, _T("circle"), _T(""), false, false, false, 0);

			if (pWp && pWayPointMan) {
				pWp->m_bIsolatedMark = true;
				RoutePoint* pExisting
					= pWayPointMan->WaypointExists(pWp->GetName(), Position(pWp->m_lat, pWp->m_lon));

				pugi::xml_node xchild = object.child("extensions");
				pugi::xml_node child = xchild.child("opencpn:action");

				if (!strcmp(child.first_child().value(), "add")) {
					if (!pExisting)
						pWayPointMan->push_back(pWp);
					pSelect->AddSelectableRoutePoint(pWp->m_lat, pWp->m_lon, pWp);
				} else if (!strcmp(child.first_child().value(), "update")) {
					if (pExisting)
						pWayPointMan->remove(pExisting);
					pWayPointMan->push_back(pWp);
					pSelect->AddSelectableRoutePoint(pWp->m_lat, pWp->m_lon, pWp);
				} else if (!strcmp(child.first_child().value(), "delete")) {
					if (pExisting)
						pWayPointMan->DestroyWaypoint(pExisting);
				} else
					delete pWp;
			}
		} else {
			if (!strcmp(object.name(), "trk")) {
				Track* pTrack = GPXLoadTrack1(object, false, false, false, 0);

				if (pTrack && g_pRouteMan) {
					pugi::xml_node xchild = object.child("extensions");
					pugi::xml_node child = xchild.child("opencpn:action");

					Route* pExisting = g_pRouteMan->RouteExists(pTrack->m_GUID);
					if (!strcmp(child.first_child().value(), "update")) {
						if (pExisting) {
							pExisting->m_RouteNameString = pTrack->m_RouteNameString;
							pExisting->m_RouteStartString = pTrack->m_RouteStartString;
							pExisting->m_RouteEndString = pTrack->m_RouteEndString;
						}
					} else if (!strcmp(child.first_child().value(), "delete")) {
						if (pExisting)
							g_pRouteMan->DeleteTrack(pExisting);
					} else
						delete pTrack;
				}
			} else {
				if (!strcmp(object.name(), "rte")) {
					Route* pRoute = GPXLoadRoute1(object, true, false, false, 0);

					if (pRoute && g_pRouteMan) {
						pugi::xml_node xchild = object.child("extensions");
						pugi::xml_node child = xchild.child("opencpn:action");

						if (!strcmp(child.first_child().value(), "add")) {
							InsertRouteA(pRoute);
						} else if (!strcmp(child.first_child().value(), "update")) {
							UpdateRouteA(pRoute);
						} else if (!strcmp(child.first_child().value(), "delete")) {
							Route* pExisting = g_pRouteMan->RouteExists(pRoute->m_GUID);
							if (pExisting)
								g_pRouteMan->DeleteRoute(pExisting);
						} else
							delete pRoute;
					}
				}
			}
		}
	}

	return true;
}

