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

#ifndef __WAYPOINTMAN__H__
#define __WAYPOINTMAN__H__

#include <wx/string.h>
#include <wx/imaglist.h>

#include <global/ColorScheme.h>
#include <RoutePoint.h>
#include <Position.h>

#include <vector>

namespace ocpnStyle { class Style; }

class RoutePoint;
class MarkIcon;
class wxBitmap;

class WayPointman
{
public:
	WayPointman();
	~WayPointman();
	wxBitmap* GetIconBitmap(const wxString& icon_key);
	int GetIconIndex(const wxBitmap* pbm);
	int GetXIconIndex(const wxBitmap* pbm);
	int GetNumIcons(void) const;
	RoutePoint* GetNearbyWaypoint(const Position& pos, double radius_meters);
	RoutePoint* GetOtherNearbyWaypoint(const Position& pos, double radius_meters,
									   const wxString& guid);
	void SetColorScheme(global::ColorScheme cs);
	bool SharedWptsExist();
	void DeleteAllWaypoints(bool b_delete_used);
	void DestroyWaypoint(RoutePoint* pRp, bool b_update_changeset = true);
	void ClearRoutePointFonts(void);
	void ProcessIcons(ocpnStyle::Style* style);
	bool DoesIconExist(const wxString& icon_key) const;
	wxBitmap* GetIconBitmap(int index);
	wxString GetIconDescription(int index) const;
	wxString GetIconKey(int index) const;
	wxImageList* Getpmarkicon_image_list(void);
	void ProcessIcon(wxBitmap pimage, const wxString& key, const wxString& description);

	RoutePoint* WaypointExists(const wxString& name, const Position& pos);

	void deleteWayPointOnLayer(int layer_id);
	void setWayPointVisibilityOnLayer(int layer_id, bool visible);
	void setWayPointNameVisibilityOnLayer(int layer_id, bool visible);
	void setWayPointListingVisibilityOnLayer(int layer_id, bool visible);

	void push_back(RoutePoint*);
	void remove(RoutePoint*);
	RoutePoint* find(const wxString& guid);
	bool contains(const RoutePoint* point) const;

	const RoutePointList& waypoints() const;
	RoutePointList& waypoints(); // FIXME: temporary

private:
	bool within_distance(const RoutePoint* point, const Position& pos, double radius_meters) const;

	typedef std::vector<MarkIcon*> Icons;

	wxBitmap* CreateDimBitmap(wxBitmap* pBitmap, double factor);

	RoutePointList points;

	int m_markicon_image_list_base_count;
	int m_nGUID;
	Icons icons;
	wxImageList icon_image_list;
};

#endif
