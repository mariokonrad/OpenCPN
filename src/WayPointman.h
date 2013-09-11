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

#include "chart1.h"

namespace ocpnStyle { class Style; }

class RoutePoint;
class RoutePointList;

class WayPointman
{
	public:
		WayPointman();
		~WayPointman();
		wxBitmap *GetIconBitmap(const wxString & icon_key);
		int GetIconIndex(const wxBitmap * pbm);
		int GetXIconIndex(const wxBitmap * pbm);
		int GetNumIcons(void) const;
		wxString CreateGUID(RoutePoint * pRP);
		RoutePoint * GetNearbyWaypoint(double lat, double lon, double radius_meters);
		RoutePoint * GetOtherNearbyWaypoint(double lat, double lon, double radius_meters, const wxString & guid);
		void SetColorScheme(ColorScheme cs);
		bool SharedWptsExist();
		void DeleteAllWaypoints(bool b_delete_used);
		RoutePoint * FindRoutePointByGUID(const wxString & guid);
		void DestroyWaypoint(RoutePoint * pRp, bool b_update_changeset = true);
		void ClearRoutePointFonts(void);
		void ProcessIcons(ocpnStyle::Style * style);
		bool DoesIconExist(const wxString & icon_key) const;
		wxBitmap * GetIconBitmap(int index);
		wxString * GetIconDescription(int index);
		wxString * GetIconKey(int index);
		wxImageList * Getpmarkicon_image_list(void);
		void ProcessIcon(wxBitmap pimage, const wxString & key, const wxString & description);

		RoutePointList * m_pWayPointList;

	private:
		wxBitmap * CreateDimBitmap(wxBitmap * pBitmap, double factor);

		wxImageList * pmarkicon_image_list; // Current wxImageList, updated on colorscheme change
		int m_markicon_image_list_base_count;
		wxArrayPtrVoid * m_pIconArray;
		int m_nGUID;
};

#endif
