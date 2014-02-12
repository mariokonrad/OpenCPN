/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#ifndef __NAVIGATION__WAYPOINTMANAGER__H__
#define __NAVIGATION__WAYPOINTMANAGER__H__

#include <wx/string.h>
#include <wx/imaglist.h>
#include <global/ColorScheme.h>
#include <RoutePoint.h>

namespace geo { class Position; }

class wxBitmap;

namespace navigation {

/// Interface for waypoint managers.
class WaypointManager
{
public:
	virtual ~WaypointManager() {}

	// waypoints
	virtual void DeleteAllWaypoints(bool b_delete_used) = 0;
	virtual void DestroyWaypoint(RoutePoint* pRp, bool b_update_changeset = true) = 0;
	virtual void deleteWayPointOnLayer(int layer_id) = 0;
	virtual RoutePoint* GetNearbyWaypoint(const geo::Position& pos, double radius_meters) = 0;
	virtual RoutePoint* GetOtherNearbyWaypoint(const geo::Position& pos, double radius_meters,
									   const wxString& guid) = 0;
	virtual RoutePoint* WaypointExists(const wxString& name, const geo::Position& pos) = 0;
	virtual bool SharedWptsExist() const = 0;
	virtual void setWayPointVisibilityOnLayer(int layer_id, bool visible) = 0;
	virtual void setWayPointNameVisibilityOnLayer(int layer_id, bool visible) = 0;
	virtual void setWayPointListingVisibilityOnLayer(int layer_id, bool visible) = 0;
	virtual void push_back(RoutePoint*) = 0;
	virtual void remove(RoutePoint*) = 0;
	virtual RoutePoint* find(const wxString& guid) = 0;
	virtual bool contains(const RoutePoint* point) const = 0;
	virtual const RoutePointList& waypoints() const = 0;
	virtual RoutePointList& waypoints() = 0; // FIXME: temporary

	// icon
	virtual wxBitmap* GetIconBitmap(const wxString& icon_key) = 0;
	virtual wxBitmap* GetIconBitmap(int index) = 0;
	virtual wxString GetIconDescription(int index) const = 0;
	virtual int GetIconIndex(const wxBitmap* pbm) const = 0;
	virtual wxString GetIconKey(int index) const = 0;
	virtual int GetNumIcons(void) const = 0;
	virtual bool DoesIconExist(const wxString& icon_key) const = 0;
	virtual wxImageList* Getpmarkicon_image_list(void) = 0;
	virtual void ProcessIcon(wxBitmap pimage, const wxString& key, const wxString& description) = 0;
	virtual int GetXIconIndex(const wxBitmap* pbm) const = 0;

	// gui
	virtual void SetColorScheme(global::ColorScheme cs) = 0; // FIXME: this should be an observer pattern
	virtual void ClearRoutePointFonts(void) = 0;
};

}

#endif
