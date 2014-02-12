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

#include <vector>
#include <navigation/WaypointManager.h>

namespace ocpnStyle { class Style; }

class MarkIcon;
class wxBitmap;

class WayPointman : public navigation::WaypointManager
{
public:
	WayPointman();
	virtual ~WayPointman();

	// waypoints
	virtual void DeleteAllWaypoints(bool b_delete_used);
	virtual void DestroyWaypoint(RoutePoint* pRp, bool b_update_changeset = true);
	virtual void deleteWayPointOnLayer(int layer_id);
	virtual RoutePoint* GetNearbyWaypoint(const geo::Position& pos, double radius_meters);
	virtual RoutePoint* GetOtherNearbyWaypoint(const geo::Position& pos, double radius_meters,
									   const wxString& guid);
	virtual RoutePoint* WaypointExists(const wxString& name, const geo::Position& pos);
	virtual bool SharedWptsExist() const;
	virtual void setWayPointVisibilityOnLayer(int layer_id, bool visible);
	virtual void setWayPointNameVisibilityOnLayer(int layer_id, bool visible);
	virtual void setWayPointListingVisibilityOnLayer(int layer_id, bool visible);
	virtual void push_back(RoutePoint*);
	virtual void remove(RoutePoint*);
	virtual RoutePoint* find(const wxString& guid);
	virtual bool contains(const RoutePoint* point) const;
	virtual const RoutePointList& waypoints() const;
	virtual RoutePointList& waypoints(); // FIXME: temporary

	// icon
	virtual wxBitmap* GetIconBitmap(const wxString& icon_key);
	virtual wxBitmap* GetIconBitmap(int index);
	virtual wxString GetIconDescription(int index) const;
	virtual int GetIconIndex(const wxBitmap* pbm) const;
	virtual wxString GetIconKey(int index) const;
	virtual int GetNumIcons(void) const;
	virtual bool DoesIconExist(const wxString& icon_key) const;
	virtual wxImageList* Getpmarkicon_image_list(void);
	virtual void ProcessIcon(wxBitmap pimage, const wxString& key, const wxString& description);
	virtual int GetXIconIndex(const wxBitmap* pbm) const;

	// gui
	virtual void ClearRoutePointFonts(void);
	virtual void SetColorScheme(global::ColorScheme cs);

	// special
	void ProcessIcons(ocpnStyle::Style& style);
	void initialize();
	void clean_points();

private:
	bool within_distance(const RoutePoint* point, const geo::Position& pos,
						 double radius_meters) const;

	typedef std::vector<MarkIcon*> Icons;

	wxBitmap* CreateDimBitmap(wxBitmap* pBitmap, double factor);

	RoutePointList points;

	int m_markicon_image_list_base_count;
	int m_nGUID;
	Icons icons;
	wxImageList icon_image_list;
};

#endif
