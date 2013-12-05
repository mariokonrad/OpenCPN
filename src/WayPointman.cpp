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

#include "WayPointman.h"
#include <RoutePoint.h>
#include <Routeman.h>
#include <Select.h>
#include <StyleManager.h>
#include <Style.h>
#include <Config.h>
#include <UserColors.h>
#include <MarkIcon.h>

#include <global/OCPN.h>
#include <global/System.h>

#include <algorithm>

#include <wx/imaglist.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/dcmemory.h>

extern ocpnStyle::StyleManager* g_StyleManager;
extern RoutePoint* pAnchorWatchPoint1;
extern RoutePoint* pAnchorWatchPoint2;
extern Routeman* g_pRouteMan;
extern Config* pConfig;
extern Select* pSelect;

WayPointman::WayPointman()
{
	ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
	ProcessIcons(style);

	// Load user defined icons.

	wxString UserIconPath = global::OCPN::get().sys().data().private_data_dir;
	wxChar sep = wxFileName::GetPathSeparator();
	if (UserIconPath.Last() != sep)
		UserIconPath.Append(sep);
	UserIconPath.Append(_T("UserIcons"));

	if (wxDir::Exists(UserIconPath)) {
		wxArrayString FileList;

		wxDir dir(UserIconPath);
		int n_files = dir.GetAllFiles(UserIconPath, &FileList);

		for (int ifile = 0; ifile < n_files; ifile++) {
			wxString name = FileList.Item(ifile);

			wxFileName fn(name);
			wxString iconname = fn.GetName();
			wxBitmap icon1;

			if (fn.GetExt().Lower() == _T("xpm")) {
				if (icon1.LoadFile(name, wxBITMAP_TYPE_XPM)) {
					ProcessIcon(icon1, iconname, iconname);
				}
			}
			if (fn.GetExt().Lower() == _T("png")) {
				if (icon1.LoadFile(name, wxBITMAP_TYPE_PNG)) {
					ProcessIcon(icon1, iconname, iconname);
				}
			}
		}
	}

	m_nGUID = 0;
}

WayPointman::~WayPointman()
{
	// FIXME: resource handling mess:
	// Two step here, since the RoutePoint dtor also touches the
	// RoutePoint list.
	// Copy the master RoutePoint list to a temporary list,
	// then clear and delete objects from the temp list
	RoutePointList temp_list;
	for (RoutePointList::iterator i = points.begin(); i != points.end(); ++i) {
		temp_list.push_back(*i);
	}
	for (RoutePointList::iterator i = temp_list.begin(); i != temp_list.end(); ++i)
		delete *i;
	temp_list.clear();

	points.clear();

	for (Icons::iterator i = icons.begin(); i != icons.end(); ++i) {
		MarkIcon* pmi = *i;
		delete pmi->bitmap; // FIXME: let MarkIcon handle its own resources
		delete pmi;
	}
	icons.clear();
	icon_image_list.RemoveAll();
}

void WayPointman::ProcessIcons( ocpnStyle::Style* style )
{
	ProcessIcon(style->GetIcon(_T("empty")), _T("empty"), _T("Empty"));
	ProcessIcon(style->GetIcon(_T("airplane")), _T("airplane"), _T("Airplane"));
	ProcessIcon(style->GetIcon(_T("anchorage")), _T("anchorage"), _T("Anchorage"));
	ProcessIcon(style->GetIcon(_T("anchor")), _T("anchor"), _T("Anchor"));
	ProcessIcon(style->GetIcon(_T("boarding")), _T("boarding"), _T("Boarding Location"));
	ProcessIcon(style->GetIcon(_T("boundary")), _T("boundary"), _T("Boundary Mark"));
	ProcessIcon(style->GetIcon(_T("bouy1")), _T("bouy1"), _T("Bouy Type A"));
	ProcessIcon(style->GetIcon(_T("bouy2")), _T("bouy2"), _T("Bouy Type B"));
	ProcessIcon(style->GetIcon(_T("campfire")), _T("campfire"), _T("Campfire"));
	ProcessIcon(style->GetIcon(_T("camping")), _T("camping"), _T("Camping Spot"));
	ProcessIcon(style->GetIcon(_T("coral")), _T("coral"), _T("Coral"));
	ProcessIcon(style->GetIcon(_T("fishhaven")), _T("fishhaven"), _T("Fish Haven"));
	ProcessIcon(style->GetIcon(_T("fishing")), _T("fishing"), _T("Fishing Spot"));
	ProcessIcon(style->GetIcon(_T("fish")), _T("fish"), _T("Fish"));
	ProcessIcon(style->GetIcon(_T("float")), _T("float"), _T("Float"));
	ProcessIcon(style->GetIcon(_T("food")), _T("food"), _T("Food"));
	ProcessIcon(style->GetIcon(_T("fuel")), _T("fuel"), _T("Fuel"));
	ProcessIcon(style->GetIcon(_T("greenlite")), _T("greenlite"), _T("Green Light"));
	ProcessIcon(style->GetIcon(_T("kelp")), _T("kelp"), _T("Kelp"));
	ProcessIcon(style->GetIcon(_T("light")), _T("light1"), _T("Light Type A"));
	ProcessIcon(style->GetIcon(_T("light1")), _T("light"), _T("Light Type B"));
	ProcessIcon(style->GetIcon(_T("litevessel")), _T("litevessel"), _T("Light Vessel"));
	ProcessIcon(style->GetIcon(_T("mob")), _T("mob"), _T("MOB"));
	ProcessIcon(style->GetIcon(_T("mooring")), _T("mooring"), _T("Mooring Bouy"));
	ProcessIcon(style->GetIcon(_T("oilbouy")), _T("oilbouy"), _T("Oil Bouy"));
	ProcessIcon(style->GetIcon(_T("platform")), _T("platform"), _T("Platform"));
	ProcessIcon(style->GetIcon(_T("redgreenlite")), _T("redgreenlite"), _T("Red/Green Light"));
	ProcessIcon(style->GetIcon(_T("redlite")), _T("redlite"), _T("Red Light"));
	ProcessIcon(style->GetIcon(_T("rock1")), _T("rock1"), _T("Rock (exposed)"));
	ProcessIcon(style->GetIcon(_T("rock2")), _T("rock2"), _T("Rock, (awash)"));
	ProcessIcon(style->GetIcon(_T("sand")), _T("sand"), _T("Sand"));
	ProcessIcon(style->GetIcon(_T("scuba")), _T("scuba"), _T("Scuba"));
	ProcessIcon(style->GetIcon(_T("shoal")), _T("shoal"), _T("Shoal"));
	ProcessIcon(style->GetIcon(_T("snag")), _T("snag"), _T("Snag"));
	ProcessIcon(style->GetIcon(_T("square")), _T("square"), _T("Square"));
	ProcessIcon(style->GetIcon(_T("triangle")), _T("triangle"), _T("Triangle"));
	ProcessIcon(style->GetIcon(_T("diamond")), _T("diamond"), _T("Diamond"));
	ProcessIcon(style->GetIcon(_T("circle")), _T("circle"), _T("Circle"));
	ProcessIcon(style->GetIcon(_T("wreck1")), _T("wreck1"), _T("Wreck A"));
	ProcessIcon(style->GetIcon(_T("wreck2")), _T("wreck2"), _T("Wreck B"));
	ProcessIcon(style->GetIcon(_T("xmblue")), _T("xmblue"), _T("Blue X"));
	ProcessIcon(style->GetIcon(_T("xmgreen")), _T("xmgreen"), _T("Green X"));
	ProcessIcon(style->GetIcon(_T("xmred")), _T("xmred"), _T("Red X"));
	ProcessIcon(style->GetIcon(_T("activepoint")), _T("activepoint"), _T("Active WP"));
}

void WayPointman::ProcessIcon(wxBitmap pimage, const wxString& key, const wxString& description)
{
	MarkIcon* pmi;

	bool newIcon = true;

	for (Icons::iterator i = icons.begin(); i != icons.end(); ++i) {
		pmi = *i;
		if (pmi->name.IsSameAs(key)) {
			newIcon = false;
			delete pmi->bitmap;
			break;
		}
	}

	if (newIcon) {
		pmi = new MarkIcon(key, description);
		icons.push_back(pmi);
	}

	pmi->bitmap = new wxBitmap(pimage);
}

// This method cannot be const nor return a const reference to the image list
// because the wxWidgets crap is taking a plain pointer for wxListCtrl::SetImageList...
wxImageList* WayPointman::Getpmarkicon_image_list(void)
{
	// First find the largest bitmap size
	int w = 0;
	int h = 0;

	for (Icons::iterator i = icons.begin(); i != icons.end(); ++i) {
		w = wxMax(w, (*i)->bitmap->GetWidth());
		h = wxMax(h, (*i)->bitmap->GetHeight());

		// toh, 10.09.29
		// User defined icons won't be displayed in the list if they are larger than 32x32 pixels
		// (why???)
		// Work-around: limit size
		if (w > 32)
			w = 32;
		if (h > 32)
			h = 32;
	}

	// Build an image list large enough

	icon_image_list.RemoveAll();
	icon_image_list.Create(w, h);

	// Add the icons
	for (Icons::iterator i = icons.begin(); i != icons.end(); ++i) {
		wxImage icon_image = (*i)->bitmap->ConvertToImage();

		// toh, 10.09.29
		// After limiting size user defined icons will be cut off
		// Work-around: rescale in one or both directions
		int h0 = icon_image.GetHeight();
		int w0 = icon_image.GetWidth();

		wxImage icon_larger;
		if (h0 <= h && w0 <= w) {
			// Resize & Center smaller icons in the bitmap, so menus won't look so weird.
			icon_larger = icon_image.Resize(wxSize(w, h), wxPoint((w - w0) / 2, (h - h0) / 2));
		} else {
			// rescale in one or two directions to avoid cropping, then resize to fit to cell
			int h1 = h;
			int w1 = w;
			if (h0 > h)
				w1 = wxRound((double)w0 * ((double)h / (double)h0));

			else if (w0 > w)
				h1 = wxRound((double)h0 * ((double)w / (double)w0));

			icon_larger = icon_image.Rescale(w1, h1);
			icon_larger = icon_larger.Resize(wxSize(w, h), wxPoint(0, 0));
		}

		icon_image_list.Add(icon_larger);
	}

	m_markicon_image_list_base_count = icon_image_list.GetImageCount();

	// Create and add "x-ed out" icons,
	// Being careful to preserve (some) transparency
	for (unsigned int ii = 0; ii < icons.size(); ii++) {

		wxImage img = icon_image_list.GetBitmap(ii).ConvertToImage();
		img.ConvertAlphaToMask(128);

		unsigned char r;
		unsigned char g;
		unsigned char b;
		img.GetOrFindMaskColour(&r, &g, &b);
		wxColour unused_color(r, g, b);

		wxBitmap bmp0(img);

		wxBitmap bmp(w, h, -1);
		wxMemoryDC mdc(bmp);
		mdc.SetBackground(wxBrush(unused_color));
		mdc.Clear();
		mdc.DrawBitmap(bmp0, 0, 0);
		wxPen red(GetGlobalColor(_T( "URED" )), 2);
		mdc.SetPen(red);
		int xm = bmp.GetWidth();
		int ym = bmp.GetHeight();
		mdc.DrawLine(2, 2, xm - 2, ym - 2);
		mdc.DrawLine(xm - 2, 2, 2, ym - 2);
		mdc.SelectObject(wxNullBitmap);

		wxMask* pmask = new wxMask(bmp, unused_color);
		bmp.SetMask(pmask);

		icon_image_list.Add(bmp);
	}

	return &icon_image_list;
}

wxBitmap* WayPointman::CreateDimBitmap(wxBitmap* pBitmap, double factor)
{
	wxImage img = pBitmap->ConvertToImage();
	int sx = img.GetWidth();
	int sy = img.GetHeight();

	wxImage new_img(img);

	for (int i = 0; i < sx; i++) {
		for (int j = 0; j < sy; j++) {
			if (!img.IsTransparent(i, j)) {
				new_img.SetRGB(i, j, (unsigned char)(img.GetRed(i, j) * factor),
							   (unsigned char)(img.GetGreen(i, j) * factor),
							   (unsigned char)(img.GetBlue(i, j) * factor));
			}
		}
	}

	wxBitmap* pret = new wxBitmap(new_img);

	return pret;
}

void WayPointman::push_back(RoutePoint* route_point)
{
	if (!route_point)
		return;

	points.push_back(route_point);
}

void WayPointman::remove(RoutePoint* route_point)
{
	if (!route_point)
		return;

	points.erase(std::find(points.begin(), points.end(), route_point));
}

RoutePoint* WayPointman::find(const wxString& guid)
{
	RoutePointList::iterator i = find_if(points.begin(), points.end(), RoutePoint::SameGUID(guid));
	return i == points.end() ? NULL : *i;
}

bool WayPointman::contains(const RoutePoint* point) const
{
	return std::find(points.begin(), points.end(), point) != points.end();
}

void WayPointman::SetColorScheme(ColorScheme)
{
	ProcessIcons(g_StyleManager->GetCurrentStyle());

	for (RoutePointList::iterator i = points.begin(); i != points.end(); ++i) {
		(*i)->ReLoadIcon();
	}
}

bool WayPointman::DoesIconExist(const wxString& icon_key) const
{
	for (Icons::const_iterator i = icons.begin(); i != icons.end(); ++i) {
		if ((*i)->name.IsSameAs(icon_key))
			return true;
	}

	return false;
}

wxBitmap* WayPointman::GetIconBitmap(const wxString& icon_key)
{
	MarkIcon* pmi = NULL;
	unsigned int i;

	for (i = 0; i < icons.size(); ++i) {
		pmi = icons[i];
		if (pmi->name.IsSameAs(icon_key))
			break;
	}

	if (i == icons.size()) { // key not found
		for (i = 0; i < icons.size(); ++i) {
			pmi = icons[i];
			if (pmi->name.IsSameAs(_T("circle")))
				break;
		}
	}

	if (i == icons.size()) // not found again
		pmi = icons[0];

	return pmi ? pmi->bitmap : NULL;
}

wxBitmap * WayPointman::GetIconBitmap(int index)
{
	if (index < 0)
		return NULL;
	if (index >= static_cast<int>(icons.size()))
		return NULL;

	return icons[index]->bitmap;
}

wxString WayPointman::GetIconDescription(int index) const
{
	if (index < 0)
		return wxString();
	if (index >= static_cast<int>(icons.size()))
		return wxString();

	return icons[index]->description;
}

wxString WayPointman::GetIconKey(int index) const
{
	if (index < 0)
		return wxString();
	if (index >= static_cast<int>(icons.size()))
		return wxString();

	return icons[index]->name;
}

int WayPointman::GetIconIndex(const wxBitmap * pbm)
{
	for (Icons::iterator i = icons.begin(); i != icons.end(); ++i) {
		if ((*i)->bitmap == pbm)
			return i - icons.begin();
	}

	return -1;
}

int WayPointman::GetXIconIndex(const wxBitmap * pbm)
{
	for (unsigned int i = 0; i < icons.size(); i++ ) {
		if (icons[i]->bitmap == pbm)
			return i + m_markicon_image_list_base_count;
	}

	return -1;
}

bool WayPointman::within_distance(const RoutePoint* point, const Position& pos, double radius_meters) const
{
	double a = pos.lat() - point->latitude();
	double b = pos.lon() - point->longitude();
	double l = sqrt((a * a) + (b * b));

	return (l * 60.0 * 1852.0) < radius_meters;
}

RoutePoint* WayPointman::GetNearbyWaypoint(const Position& pos, double radius_meters)
{
	// Iterate on the RoutePoint list, checking distance

	for (RoutePointList::iterator i = points.begin(); i != points.end(); ++i) {
		RoutePoint* pr = *i;
		if (within_distance(pr, pos, radius_meters))
			return pr;
	}
	return NULL;
}

RoutePoint* WayPointman::GetOtherNearbyWaypoint(
		const Position& pos,
		double radius_meters,
		const wxString& guid)
{
	// Iterate on the RoutePoint list, checking distance

	for (RoutePointList::const_iterator i = points.begin(); i != points.end(); ++i) {
		RoutePoint* pr = *i;
		if (within_distance(pr, pos, radius_meters) && (pr->m_GUID == guid))
			return pr;
	}
	return NULL;
}

void WayPointman::ClearRoutePointFonts(void)
{
	// Iterate on the RoutePoint list, clearing Font pointers
	// This is typically done globally after a font switch

	for (RoutePointList::iterator i = points.begin(); i != points.end(); ++i) {
		(*i)->clear_font();
	}
}

bool WayPointman::SharedWptsExist()
{
	for (RoutePointList::const_iterator i = points.begin(); i != points.end(); ++i) {
		const RoutePoint* prp = *i;
		if (true
			&& prp->m_bKeepXRoute
			&& (false
				|| prp->m_bIsInRoute
				|| prp->m_bIsInTrack
				|| prp == pAnchorWatchPoint1
				|| prp == pAnchorWatchPoint2))
			return true;
	}
	return false;
}

void WayPointman::DeleteAllWaypoints(bool b_delete_used)
{
	// FIXME: altering container which is iterated through
	// Iterate on the RoutePoint list, deleting all
	RoutePointList::iterator i = points.begin();
	while (i != points.end()) {
		RoutePoint* prp = *i;

		// if argument is false, then only delete non-route waypoints
		if (!prp->m_bIsInLayer && (prp->m_IconName != _T("mob"))
			&& ((b_delete_used && prp->m_bKeepXRoute)
				|| ((!prp->m_bIsInRoute) && (!prp->m_bIsInTrack) && !(prp == pAnchorWatchPoint1)
					&& !(prp == pAnchorWatchPoint2)))) {
			DestroyWaypoint(prp);
			delete prp;
			// TODO: why not remove the entry from list
			i = points.begin();
		} else {
			++i;
		}
	}
	return;
}

void WayPointman::DestroyWaypoint(RoutePoint * route_point, bool b_update_changeset)
{
	if (!route_point)
		return;

	// Get a list of all routes containing this point
	// and remove the point from them all
	// FIXME: handling the list of route should be one in the route list manager, not here
	Routeman::RouteArray * route_array = g_pRouteMan->GetRouteArrayContaining(route_point); // FIXME: return a std container
	if (route_array) {

		for (Routeman::RouteArray::iterator i = route_array->begin(); i != route_array->end(); ++i) {
			Route * route = static_cast<Route *>(*i);
			route->RemovePoint(route_point);
		}

		// Scrub the routes, looking for one-point routes
		for (Routeman::RouteArray::iterator i = route_array->begin(); i != route_array->end(); ++i) {
			Route * route = static_cast<Route *>(*i);
			if (route->GetnPoints() < 2) {
				pConfig->disable_changeset_update();
				pConfig->DeleteConfigRoute(route);
				g_pRouteMan->DeleteRoute(route);
				pConfig->enable_changeset_update();
			}
		}

		delete route_array;
	}

	// Now it is safe to delete the point
	if (!b_update_changeset)
		pConfig->disable_changeset_update();

	pConfig->DeleteWayPoint(route_point);

	pConfig->enable_changeset_update();

	pSelect->DeleteSelectablePoint(route_point, SelectItem::TYPE_ROUTEPOINT);

	//TODO  FIXME
	// Some memory corruption occurs if the wp is deleted here.
	// To continue running OK, it is sufficient to simply remove the wp from the global list
	// This will leak, although called infrequently....
	//  12/15/10...Seems to occur only on MOB delete....

	this->remove(route_point);

	// The RoutePoint might be currently in use as an anchor watch point
	if (route_point == pAnchorWatchPoint1)
		pAnchorWatchPoint1 = NULL;
	if (route_point == pAnchorWatchPoint2)
		pAnchorWatchPoint2 = NULL;
}

int WayPointman::GetNumIcons(void) const
{
	return icons.size();
}

RoutePoint* WayPointman::WaypointExists(const wxString& name, const Position& pos)
{
	for (RoutePointList::iterator i = points.begin(); i != points.end(); ++i) {
		RoutePoint* pr = *i;
		if (name == pr->GetName()) {
			if (fabs(pos.lat() - pr->latitude()) < 1.e-6 && fabs(pos.lon() - pr->longitude()) < 1.e-6) {
				return pr;
			}
		}
	}
	return NULL;
}

void WayPointman::deleteWayPointOnLayer(int layer_id)
{
	// FIXME: container altering iterating, iterate through copy of list, only elements are interesting
	RoutePointList::iterator i = points.begin();
	while (i != points.end()) {
		RoutePointList::iterator next = i;
		++next;
		RoutePoint *rp = *i;
		if (rp && (rp->get_layer_ID() == layer_id)) {
			rp->m_bIsInLayer = false;
			rp->set_layer_ID(0);
			DestroyWaypoint(rp, false);
		}
		i = next;
	}
}

void WayPointman::setWayPointVisibilityOnLayer(int layer_id, bool visible)
{
	for (RoutePointList::iterator i = points.begin(); i != points.end(); ++i) {
		RoutePoint* rp = *i;
		if (rp && (rp->get_layer_ID() == layer_id)) {
			rp->SetVisible(visible);
		}
	}
}

void WayPointman::setWayPointNameVisibilityOnLayer(int layer_id, bool visible)
{
	for (RoutePointList::iterator i = points.begin(); i != points.end(); ++i) {
		RoutePoint* rp = *i;
		if (rp && (rp->get_layer_ID() == layer_id)) {
			rp->SetNameShown(visible);
		}
	}
}

void WayPointman::setWayPointListingVisibilityOnLayer(int layer_id, bool visible)
{
	for (RoutePointList::iterator i = points.begin(); i != points.end(); ++i) {
		RoutePoint* rp = *i;
		if (rp && !rp->m_bIsInTrack && rp->m_bIsolatedMark && (rp->get_layer_ID() == layer_id)) {
			rp->SetListed(visible);
		}
	}
}

const RoutePointList& WayPointman::waypoints() const
{
	return points;
}

RoutePointList& WayPointman::waypoints()
{
	return points;
}

