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
#include <Track.h>
#include <Select.h>

#include <global/OCPN.h>

#include <navigation/RouteManager.h>
#include <navigation/WaypointManager.h>

extern Select* pSelect;

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

	navigation::WaypointManager& waypointmanager = global::OCPN::get().waypointman();

	pugi::xml_node objects = this->child("gpx");

	for (pugi::xml_node object = objects.first_child(); object; object = object.next_sibling()) {
		if (!strcmp(object.name(), "wpt")) {
			RoutePoint* pWp
				= GPXLoadWaypoint1(object, _T("circle"), _T(""), false, false, false, 0);

			if (pWp) {
				pWp->m_bIsolatedMark = true;
				RoutePoint* pExisting
					= waypointmanager.WaypointExists(pWp->GetName(), pWp->get_position());

				pugi::xml_node xchild = object.child("extensions");
				pugi::xml_node child = xchild.child("opencpn:action");

				if (!strcmp(child.first_child().value(), "add")) {
					if (!pExisting) {
						waypointmanager.push_back(pWp);
					}
					pSelect->AddSelectableRoutePoint(pWp->get_position(), pWp);
				} else if (!strcmp(child.first_child().value(), "update")) {
					if (pExisting) {
						waypointmanager.remove(pExisting);
					}
					waypointmanager.push_back(pWp);
					pSelect->AddSelectableRoutePoint(pWp->get_position(), pWp);
				} else if (!strcmp(child.first_child().value(), "delete")) {
					if (pExisting) {
						waypointmanager.DestroyWaypoint(pExisting);
					}
				} else
					delete pWp;
			}
		} else {
			navigation::RouteManager& routemanager = global::OCPN::get().routeman();
			if (!strcmp(object.name(), "trk")) {
				Track* pTrack = GPXLoadTrack1(object, false, false, false, 0);

				if (pTrack) {
					pugi::xml_node xchild = object.child("extensions");
					pugi::xml_node child = xchild.child("opencpn:action");

					Route* pExisting = routemanager.RouteExists(pTrack->guid());
					if (!strcmp(child.first_child().value(), "update")) {
						if (pExisting) {
							pExisting->set_name(pTrack->get_name());
							pExisting->set_startString(pTrack->get_startString());
							pExisting->set_endString(pTrack->get_endString());
						}
					} else if (!strcmp(child.first_child().value(), "delete")) {
						if (pExisting)
							routemanager.DeleteTrack(pExisting);
					} else
						delete pTrack;
				}
			} else {
				if (!strcmp(object.name(), "rte")) {
					Route* pRoute = GPXLoadRoute1(object, true, false, false, 0);

					if (pRoute) {
						pugi::xml_node xchild = object.child("extensions");
						pugi::xml_node child = xchild.child("opencpn:action");

						if (!strcmp(child.first_child().value(), "add")) {
							InsertRouteA(pRoute);
						} else if (!strcmp(child.first_child().value(), "update")) {
							UpdateRouteA(pRoute);
						} else if (!strcmp(child.first_child().value(), "delete")) {
							Route* pExisting = routemanager.RouteExists(pRoute->guid());
							if (pExisting)
								routemanager.DeleteRoute(pExisting);
						} else
							delete pRoute;
					}
				}
			}
		}
	}

	return true;
}

