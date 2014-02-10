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

#include "NavObjectCollection.h"

#include <RoutePoint.h>
#include <Routeman.h>
#include <WayPointman.h>
#include <Track.h>
#include <Select.h>
#include <Config.h>

#include <gpx/ParseGPXDateTime.h>
#include <gpx/GpxDocument.h>

extern WayPointman* pWayPointMan;
extern Routeman* g_pRouteMan;
extern Config* pConfig;

extern RouteList* pRouteList;
extern Select* pSelect;

// Bitfield definitions controlling the GPX nodes output for Route.Track objects
#define RT_OUT_ACTION_ADD 1 << 1          //  opencpn:action node support
#define RT_OUT_ACTION_DEL 1 << 2
#define RT_OUT_ACTION_UPD 1 << 3

NavObjectCollection::NavObjectCollection()
	: pugi::xml_document()
{}

NavObjectCollection::~NavObjectCollection()
{}

RoutePoint* NavObjectCollection::GPXLoadWaypoint1(
		pugi::xml_node & wpt_node,
		wxString def_symbol_name,
		wxString GUID,
		bool b_fullviz,
		bool b_layer,
		bool b_layerviz,
		int layer_id
		)
{
	bool bviz = false;
	bool bviz_name = false;
	bool bauto_name = false;
	bool bshared = false;
	bool b_propvizname = false;
	bool b_propviz = false;

	wxString SymString = def_symbol_name; // default icon
	wxString NameString;
	wxString DescString;
	wxString TypeString;
	wxString GuidString = GUID; // default
	wxString TimeString;
	wxDateTime dt;
	RoutePoint* pWP = NULL;

	Hyperlinks linklist;

	double rlat = wpt_node.attribute("lat").as_double();
	double rlon = wpt_node.attribute("lon").as_double();

	for (pugi::xml_node child = wpt_node.first_child(); child != 0; child = child.next_sibling()) {
		const char* pcn = child.name();
		if (!strcmp(pcn, "sym")) {
			SymString = wxString::FromUTF8(child.first_child().value());
		} else if (!strcmp(pcn, "time")) {
			TimeString = wxString::FromUTF8(child.first_child().value());
		} else if (!strcmp(pcn, "name")) {
			NameString = wxString::FromUTF8(child.first_child().value());
		} else if (!strcmp(pcn, "desc")) {
			DescString = wxString::FromUTF8(child.first_child().value());
		} else if (!strcmp(pcn, "type")) {
			TypeString = wxString::FromUTF8(child.first_child().value());
		} else if (!strcmp(pcn, "link")) {
			// Read hyperlink
			wxString HrefString;
			wxString HrefTextString;
			wxString HrefTypeString;
			HrefString = wxString::FromUTF8(child.first_attribute().value());

			for (pugi::xml_node child1 = child.first_child(); child1;
				 child1 = child1.next_sibling()) {
				wxString LinkString = wxString::FromUTF8(child1.name());

				if (LinkString == _T("text"))
					HrefTextString = wxString::FromUTF8(child1.first_child().value());
				if (LinkString == _T("type"))
					HrefTypeString = wxString::FromUTF8(child1.first_child().value());
			}

			linklist.push_back(Hyperlink(HrefTextString, HrefString, HrefTypeString));
		} else if (!strcmp(pcn, "extensions")) {
			// OpenCPN Extensions....
			for (pugi::xml_node ext_child = child.first_child(); ext_child;
				 ext_child = ext_child.next_sibling()) {
				wxString ext_name = wxString::FromUTF8(ext_child.name());
				if (ext_name == _T("opencpn:guid")) {
					GuidString = wxString::FromUTF8(ext_child.first_child().value());
				} else if (ext_name == _T("opencpn:viz")) {
					b_propviz = true;
					wxString s = wxString::FromUTF8(ext_child.first_child().value());
					long v = 0;
					if (s.ToLong(&v))
						bviz = (v != 0);
				} else if (ext_name == _T("opencpn:viz_name")) {
					b_propvizname = true;
					wxString s = wxString::FromUTF8(ext_child.first_child().value());
					long v = 0;
					if (s.ToLong(&v))
						bviz_name = (v != 0);
				} else if (ext_name == _T("opencpn:auto_name")) {
					wxString s = wxString::FromUTF8(ext_child.first_child().value());
					long v = 0;
					if (s.ToLong(&v))
						bauto_name = (v != 0);
				} else if (ext_name == _T ("opencpn:shared")) {
					wxString s = wxString::FromUTF8(ext_child.first_child().value());
					long v = 0;
					if (s.ToLong(&v))
						bshared = (v != 0);
				}
			}
		}
	}

	// Create waypoint
	if (b_layer) {
		if (GuidString.IsEmpty())
			GuidString = _T("LayGUID");
	}
	// do not add to global WP list yet...
	pWP = new RoutePoint(geo::Position(rlat, rlon), SymString, NameString, GuidString, false);
	pWP->set_description(DescString);
	pWP->m_bIsolatedMark = bshared; // This is an isolated mark

	if (b_propvizname)
		pWP->m_bShowName = bviz_name;
	else if (b_fullviz)
		pWP->m_bShowName = true;
	else
		pWP->m_bShowName = false;

	if (b_propviz)
		pWP->m_bIsVisible = bviz;
	else if (b_fullviz)
		pWP->m_bIsVisible = true;

	if (b_layer) {
		pWP->m_bIsInLayer = true;
		pWP->set_layer_ID(layer_id);
		pWP->m_bIsVisible = b_layerviz;
		pWP->SetListed(false);
	}

	pWP->m_bKeepXRoute = bshared;
	pWP->m_bDynamicName = bauto_name;

	if (TimeString.Len()) {
		pWP->set_time_string(TimeString);
	}

	if (linklist.size()) {
		pWP->m_HyperlinkList = linklist;
	}

	return pWP;
}

Track * NavObjectCollection::GPXLoadTrack1(
		pugi::xml_node & trk_node,
		bool b_fullviz,
		bool b_layer,
		bool b_layerviz,
		int layer_id )
{
	wxString RouteName;
	wxString DescString;
	unsigned short int GPXSeg;
	bool b_propviz = false;
	bool b_viz = true;
	Track* pTentTrack = NULL;
	Hyperlinks linklist;

	wxString Name = wxString::FromUTF8(trk_node.name());
	if (Name == _T ( "trk" )) {
		pTentTrack = new Track();
		GPXSeg = 0;
		RoutePoint* pWp = NULL;

		for (pugi::xml_node tschild = trk_node.first_child(); tschild;
			 tschild = tschild.next_sibling()) {
			wxString ChildName = wxString::FromUTF8(tschild.name());
			if (ChildName == _T("trkseg")) {
				GPXSeg += 1;

				// Official GPX spec calls for trkseg to have children trkpt
				for (pugi::xml_node tpchild = tschild.first_child(); tpchild;
					 tpchild = tpchild.next_sibling()) {
					wxString tpChildName = wxString::FromUTF8(tpchild.name());
					if (tpChildName == _T("trkpt")) {
						pWp = GPXLoadWaypoint1(tpchild, _T("empty"), _T("noGUID"), false, b_layer,
											   b_layerviz, layer_id);
						pWp->m_bIsolatedMark = false;
						pTentTrack->AddPoint(pWp, false, true); // defer BBox calculation
						pWp->m_bIsInRoute = false; // Hack
						pWp->m_bIsInTrack = true;
						pWp->m_GPXTrkSegNo = GPXSeg;
						pWayPointMan->push_back(pWp);
					}
				}
			} else if (ChildName == _T("name")) {
				RouteName = wxString::FromUTF8(tschild.first_child().value());
			} else if (ChildName == _T("desc")) {
				DescString = wxString::FromUTF8(tschild.first_child().value());
			} else if (ChildName.EndsWith(_T("TrackExtension"))) { // Parse GPXX color
				for (pugi::xml_node gpxx_child = tschild.first_child(); gpxx_child;
					 gpxx_child = gpxx_child.next_sibling()) {
					wxString gpxx_name = wxString::FromUTF8(gpxx_child.name());
					if (gpxx_name.EndsWith(_T("DisplayColor")))
						pTentTrack->m_Colour = wxString::FromUTF8(gpxx_child.first_child().value());
				}
			} else if (ChildName == _T("link")) {
				wxString HrefString;
				wxString HrefTextString;
				wxString HrefTypeString;
				HrefString = wxString::FromUTF8(tschild.first_attribute().value());

				for (pugi::xml_node child1 = tschild.first_child(); child1;
					 child1 = child1.next_sibling()) {
					wxString LinkString = wxString::FromUTF8(child1.name());

					if (LinkString == _T ( "text" ))
						HrefTextString = wxString::FromUTF8(child1.first_child().value());
					if (LinkString == _T ( "type" ))
						HrefTypeString = wxString::FromUTF8(child1.first_child().value());
				}

				linklist.push_back(Hyperlink(HrefTextString, HrefString, HrefTypeString));
			} else if (ChildName == _T("extensions")) {
				for (pugi::xml_node ext_child = tschild.first_child(); ext_child;
					 ext_child = ext_child.next_sibling()) {
					wxString ext_name = wxString::FromUTF8(ext_child.name());
					if (ext_name == _T ( "opencpn:start" )) {
						pTentTrack->set_startString(wxString::FromUTF8(ext_child.first_child().value()));
					} else if (ext_name == _T ( "opencpn:end" )) {
						pTentTrack->set_endString(wxString::FromUTF8(ext_child.first_child().value()));
					} else if (ext_name == _T ( "opencpn:viz" )) {
						wxString viz = wxString::FromUTF8(ext_child.first_child().value());
						b_propviz = true;
						b_viz = (viz == _T("1"));
					} else if (ext_name == _T ( "opencpn:style" )) {
						for (pugi::xml_attribute attr = ext_child.first_attribute(); attr;
							 attr = attr.next_attribute()) {
							if (!strcmp(attr.name(), "style")) {
								pTentTrack->m_style = attr.as_int();
							} else if (!strcmp(attr.name(), "width")) {
								pTentTrack->m_width = attr.as_int();
							}
						}
					} else if (ext_name == _T("opencpn:guid")) {
						pTentTrack->set_guid(wxString::FromUTF8(ext_child.first_child().value()));
					} else if (ext_name.EndsWith(_T("TrackExtension"))) { // Parse
						// GPXX
						// color
						for (pugi::xml_node gpxx_child = ext_child.first_child(); gpxx_child;
							 gpxx_child = gpxx_child.next_sibling()) {
							wxString gpxx_name = wxString::FromUTF8(gpxx_child.name());
							if (gpxx_name.EndsWith(_T ( "DisplayColor" )))
								pTentTrack->m_Colour
									= wxString::FromUTF8(gpxx_child.first_child().value());
						}
					}
				} // extensions
			}
		}

		pTentTrack->set_name(RouteName);
		pTentTrack->set_description(DescString);

		if (b_propviz) {
			pTentTrack->SetVisible(b_viz);
		} else {
			if (b_fullviz)
				pTentTrack->SetVisible();
		}

		if (b_layer) {
			pTentTrack->SetVisible(b_layerviz);
			pTentTrack->m_bIsInLayer = true;
			pTentTrack->m_LayerID = layer_id;
			pTentTrack->SetListed(false);
		}
	}

	if (linklist.size()) {
		pTentTrack->m_HyperlinkList = linklist;
	}
	return pTentTrack;
}


Route * NavObjectCollection::GPXLoadRoute1(
		pugi::xml_node & wpt_node,
		bool b_fullviz,
		bool b_layer,
		bool b_layerviz,
		int layer_id )
{
	wxString RouteName;
	wxString DescString;
	bool b_propviz = false;
	bool b_viz = true;
	Route* pTentRoute = NULL;
	Hyperlinks linklist;

	wxString Name = wxString::FromUTF8(wpt_node.name());
	if (Name == _T("rte")) {
		pTentRoute = new Route();
		RoutePoint* pWp = NULL;

		for (pugi::xml_node tschild = wpt_node.first_child(); tschild;
			 tschild = tschild.next_sibling()) {
			wxString ChildName = wxString::FromUTF8(tschild.name());

			if (ChildName == _T("rtept")) {
				pWp = GPXLoadWaypoint1(tschild, _T("square"), _T(""), b_fullviz, b_layer,
									   b_layerviz, layer_id);
				RoutePoint* erp = pWayPointMan->find(pWp->guid());
				if (erp != NULL)
					pWp = erp;
				pTentRoute->AddPoint(pWp, false, true); // defer BBox calculation
				pWp->m_bIsInRoute = true; // Hack
				pWp->m_bIsInTrack = false;
				if (erp == NULL)
					pWayPointMan->push_back(pWp);
			} else if (ChildName == _T("name")) {
				RouteName = wxString::FromUTF8(tschild.first_child().value());
			} else if (ChildName == _T("desc")) {
				DescString = wxString::FromUTF8(tschild.first_child().value());
			} else if (ChildName.EndsWith(_T("RouteExtension"))) { // Parse GPXX color
				for (pugi::xml_node gpxx_child = tschild.first_child(); gpxx_child;
					 gpxx_child = gpxx_child.next_sibling()) {
					wxString gpxx_name = wxString::FromUTF8(gpxx_child.name());
					if (gpxx_name.EndsWith(_T ( "DisplayColor" )))
						pTentRoute->m_Colour = wxString::FromUTF8(gpxx_child.first_child().value());
				}
			} else if (ChildName == _T("link")) {
				wxString HrefString;
				wxString HrefTextString;
				wxString HrefTypeString;
				HrefString = wxString::FromUTF8(tschild.first_attribute().value());

				for (pugi::xml_node child1 = tschild.first_child(); child1;
					 child1 = child1.next_sibling()) {
					wxString LinkString = wxString::FromUTF8(child1.name());

					if (LinkString == _T("text"))
						HrefTextString = wxString::FromUTF8(child1.first_child().value());
					if (LinkString == _T("type"))
						HrefTypeString = wxString::FromUTF8(child1.first_child().value());
				}

				linklist.push_back(Hyperlink(HrefTextString, HrefString, HrefTypeString));
			} else if (ChildName == _T("extensions")) {
				for (pugi::xml_node ext_child = tschild.first_child(); ext_child;
					 ext_child = ext_child.next_sibling()) {
					wxString ext_name = wxString::FromUTF8(ext_child.name());

					if (ext_name == _T("opencpn:start")) {
						pTentRoute->set_startString(wxString::FromUTF8(ext_child.first_child().value()));
					} else if (ext_name == _T("opencpn:end")) {
						pTentRoute->set_endString(wxString::FromUTF8(ext_child.first_child().value()));
					} else if (ext_name == _T("opencpn:viz")) {
						wxString viz = wxString::FromUTF8(ext_child.first_child().value());
						b_propviz = true;
						b_viz = (viz == _T("1"));
					} else if (ext_name == _T("opencpn:style")) {
						for (pugi::xml_attribute attr = ext_child.first_attribute(); attr;
							 attr = attr.next_attribute()) {
							if (!strcmp(attr.name(), "style"))
								pTentRoute->m_style = attr.as_int();
							else if (!strcmp(attr.name(), "width"))
								pTentRoute->m_width = attr.as_int();
						}
					} else if (ext_name == _T("opencpn:guid")) {
						pTentRoute->set_guid(wxString::FromUTF8(ext_child.first_child().value()));
					} else if (ext_name == _T("opencpn:planned_speed")) {
						pTentRoute->m_PlannedSpeed = atof(ext_child.first_child().value());
					} else if (ext_name == _T("opencpn:planned_departure")) {
						ParseGPXDateTime(pTentRoute->m_PlannedDeparture,
										 wxString::FromUTF8(ext_child.first_child().value()));
					} else if (ext_name == _T("opencpn:time_display")) {
						pTentRoute->m_TimeDisplayFormat
							= wxString::FromUTF8(ext_child.first_child().value());
					}
				} // extensions
			}
		}

		pTentRoute->set_name(RouteName);
		pTentRoute->set_description(DescString);

		if (b_propviz) {
			pTentRoute->SetVisible(b_viz);
		} else {
			if (b_fullviz)
				pTentRoute->SetVisible();
		}

		if (b_layer) {
			pTentRoute->SetVisible(b_layerviz);
			pTentRoute->m_bIsInLayer = true;
			pTentRoute->m_LayerID = layer_id;
			pTentRoute->SetListed(false);
		}
	}
	if (linklist.size()) {
		pTentRoute->m_HyperlinkList = linklist;
	}
	return pTentRoute;
}

bool NavObjectCollection::GPXCreateWpt(
		pugi::xml_node node,
		const RoutePoint* pr,
		unsigned int flags)
{
	wxString s;
	pugi::xml_node child;
	pugi::xml_attribute attr;

	s.Printf(_T("%.9f"), pr->latitude());
	node.append_attribute("lat") = s.mb_str();
	s.Printf(_T("%.9f"), pr->longitude());
	node.append_attribute("lon") = s.mb_str();

	if (flags & OUT_TIME) {
		child = node.append_child("time");
		if (pr->get_time_string().size())
			child.append_child(pugi::node_pcdata).set_value(pr->get_time_string().mb_str());
		else {
			wxString t = pr->GetCreateTime()
							 .FormatISODate()
							 .Append(_T("T"))
							 .Append(pr->GetCreateTime().FormatISOTime())
							 .Append(_T("Z"));
			child.append_child(pugi::node_pcdata).set_value(t.mb_str());
		}
	}

	if ((!pr->GetName().IsEmpty() && (flags & OUT_NAME)) || (flags & OUT_NAME_FORCE)) {
		wxCharBuffer buffer = pr->GetName().ToUTF8();
		if (buffer.data()) {
			child = node.append_child("name");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	if ((!pr->get_description().IsEmpty() && (flags & OUT_DESC)) || (flags & OUT_DESC_FORCE)) {
		wxCharBuffer buffer = pr->get_description().ToUTF8();
		if (buffer.data()) {
			child = node.append_child("desc");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	// Hyperlinks
	if (flags & OUT_HYPERLINKS) {
		const Hyperlinks& linklist = pr->m_HyperlinkList;
		for (Hyperlinks::const_iterator i = linklist.begin(); i != linklist.end(); ++i) {

			pugi::xml_node child_link = node.append_child("link");
			wxCharBuffer buffer = i->url().ToUTF8();
			if (buffer.data())
				child_link.append_attribute("href") = buffer.data();

			buffer = i->desc().ToUTF8();
			if (buffer.data()) {
				child = child_link.append_child("text");
				child.append_child(pugi::node_pcdata).set_value(buffer.data());
			}

			buffer = i->type().ToUTF8();
			if (buffer.data() && (strlen(buffer.data()) > 0)) {
				child = child_link.append_child("type");
				child.append_child(pugi::node_pcdata).set_value(buffer.data());
			}
		}
	}

	if (flags & OUT_SYM_FORCE) {
		child = node.append_child("sym");
		if (!pr->icon_name().IsEmpty()) {
			child.append_child(pugi::node_pcdata).set_value(pr->icon_name().mb_str());
		} else {
			child.append_child("empty");
		}
	}

	if (flags & OUT_TYPE) {
		child = node.append_child("type");
		child.append_child(pugi::node_pcdata).set_value("WPT");
	}

	if (false || (flags & OUT_GUID) || (flags & OUT_VIZ) || (flags & OUT_VIZ_NAME)
		|| (flags & OUT_SHARED) || (flags & OUT_AUTO_NAME)) {

		pugi::xml_node child_ext = node.append_child("extensions");

		if (!pr->guid().IsEmpty() && (flags & OUT_GUID)) {
			child = child_ext.append_child("opencpn:guid");
			child.append_child(pugi::node_pcdata).set_value(pr->guid().mb_str());
		}

		if ((flags & OUT_VIZ) && !pr->m_bIsVisible) {
			child = child_ext.append_child("opencpn:viz");
			child.append_child(pugi::node_pcdata).set_value("0");
		}

		if ((flags & OUT_VIZ_NAME) && pr->m_bShowName) {
			child = child_ext.append_child("opencpn:viz_name");
			child.append_child(pugi::node_pcdata).set_value("1");
		}

		if ((flags & OUT_AUTO_NAME) && pr->m_bDynamicName) {
			child = child_ext.append_child("opencpn:auto_name");
			child.append_child(pugi::node_pcdata).set_value("1");
		}
		if ((flags & OUT_SHARED) && pr->m_bKeepXRoute) {
			child = child_ext.append_child("opencpn:shared");
			child.append_child(pugi::node_pcdata).set_value("1");
		}
	}

	return true;
}

bool NavObjectCollection::GPXCreateTrk(
		pugi::xml_node node,
		Route *pRoute)
{
	pugi::xml_node child;

	if (pRoute->get_name().Len()) {
		wxCharBuffer buffer = pRoute->get_name().ToUTF8();
		if (buffer.data()) {
			child = node.append_child("name");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	if (pRoute->get_description().Len()) {
		wxCharBuffer buffer = pRoute->get_description().ToUTF8();
		if (buffer.data()) {
			child = node.append_child("desc");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	// Hyperlinks
	const Hyperlinks& linklist = pRoute->m_HyperlinkList;
	for (Hyperlinks::const_iterator i = linklist.begin(); i != linklist.end(); ++i) {

		pugi::xml_node child_link = node.append_child("link");
		wxCharBuffer buffer = i->url().ToUTF8();
		if (buffer.data())
			child_link.append_attribute("href") = buffer.data();

		buffer = i->desc().ToUTF8();
		if (buffer.data()) {
			child = child_link.append_child("text");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}

		buffer = i->type().ToUTF8();
		if (buffer.data() && (strlen(buffer.data()) > 0)) {
			child = child_link.append_child("type");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	pugi::xml_node child_ext = node.append_child("extensions");

	child = child_ext.append_child("opencpn:guid");
	child.append_child(pugi::node_pcdata).set_value(pRoute->guid().mb_str());

	child = child_ext.append_child("opencpn:viz");
	child.append_child(pugi::node_pcdata).set_value(pRoute->IsVisible() == true ? "1" : "0");

	if (pRoute->get_startString().Len()) {
		wxCharBuffer buffer = pRoute->get_startString().ToUTF8();
		if (buffer.data()) {
			child = child_ext.append_child("opencpn:start");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	if (pRoute->get_endString().Len()) {
		wxCharBuffer buffer = pRoute->get_endString().ToUTF8();
		if (buffer.data()) {
			child = child_ext.append_child("opencpn:end");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	if (pRoute->m_width != Route::STYLE_UNDEFINED || pRoute->m_style != Route::STYLE_UNDEFINED) {
		child = child_ext.append_child("opencpn:style");

		if (pRoute->m_width != Route::STYLE_UNDEFINED)
			child.append_attribute("width") = pRoute->m_width;
		if (pRoute->m_style != Route::STYLE_UNDEFINED)
			child.append_attribute("style") = pRoute->m_style;
	}

	if (pRoute->m_Colour != wxEmptyString) {
		pugi::xml_node gpxx_ext = node.append_child("gpxx:TrackExtension");
		child = gpxx_ext.append_child("gpxx:DisplayColor");
		child.append_child(pugi::node_pcdata).set_value(pRoute->m_Colour.mb_str());
	}

	RoutePointList& routePointList = pRoute->routepoints();
	RoutePointList::const_iterator route_point = routePointList.begin();

	unsigned short int GPXTrkSegNo1 = 1;
	do {
		unsigned short int GPXTrkSegNo2 = GPXTrkSegNo1;

		pugi::xml_node seg = node.append_child("trkseg");

		while ((route_point != routePointList.end()) && (GPXTrkSegNo2 == GPXTrkSegNo1)) {
			GPXCreateWpt(seg.append_child("trkpt"), *route_point, OPT_TRACKPT);
			++route_point;
			if (route_point != routePointList.end()) {
				GPXTrkSegNo2 = (*route_point)->m_GPXTrkSegNo;
			}
		}
		GPXTrkSegNo1 = GPXTrkSegNo2;
	} while (route_point != routePointList.end());

	return true;
}

bool NavObjectCollection::GPXCreateRoute(
		pugi::xml_node node,
		Route* pRoute)
{
	pugi::xml_node child;

	if (pRoute->get_name().Len()) {
		wxCharBuffer buffer = pRoute->get_name().ToUTF8();
		if (buffer.data()) {
			child = node.append_child("name");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	if (pRoute->get_description().Len()) {
		wxCharBuffer buffer = pRoute->get_description().ToUTF8();
		if (buffer.data()) {
			child = node.append_child("desc");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	// Hyperlinks
	const Hyperlinks& linklist = pRoute->m_HyperlinkList;
	for (Hyperlinks::const_iterator i = linklist.begin(); i != linklist.end(); ++i) {

		pugi::xml_node child_link = node.append_child("link");
		wxCharBuffer buffer = i->url().ToUTF8();
		if (buffer.data())
			child_link.append_attribute("href") = buffer.data();

		buffer = i->desc().ToUTF8();
		if (buffer.data()) {
			child = child_link.append_child("text");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}

		buffer = i->type().ToUTF8();
		if (buffer.data() && (strlen(buffer.data()) > 0)) {
			child = child_link.append_child("type");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	pugi::xml_node child_ext = node.append_child("extensions");

	child = child_ext.append_child("opencpn:guid");
	child.append_child(pugi::node_pcdata).set_value(pRoute->guid().mb_str());

	child = child_ext.append_child("opencpn:viz");
	child.append_child(pugi::node_pcdata).set_value(pRoute->IsVisible() == true ? "1" : "0");

	if (pRoute->get_startString().Len()) {
		wxCharBuffer buffer = pRoute->get_startString().ToUTF8();
		if (buffer.data()) {
			child = child_ext.append_child("opencpn:start");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	if (pRoute->get_endString().Len()) {
		wxCharBuffer buffer = pRoute->get_endString().ToUTF8();
		if (buffer.data()) {
			child = child_ext.append_child("opencpn:end");
			child.append_child(pugi::node_pcdata).set_value(buffer.data());
		}
	}

	if (pRoute->m_PlannedSpeed != Route::DEFAULT_SPEED) {
		child = child_ext.append_child("opencpn:planned_speed");
		wxString s;
		s.Printf(_T("%.2f"), pRoute->m_PlannedSpeed);
		child.append_child(pugi::node_pcdata).set_value(s.mb_str());
	}

	if (pRoute->m_PlannedDeparture.IsValid()) {
		child = child_ext.append_child("opencpn:planned_departure");
		wxString t = pRoute->m_PlannedDeparture.FormatISODate()
						 .Append(_T("T"))
						 .Append(pRoute->m_PlannedDeparture.FormatISOTime())
						 .Append(_T("Z"));
		child.append_child(pugi::node_pcdata).set_value(t.mb_str());
	}

	if (pRoute->m_TimeDisplayFormat != RTE_TIME_DISP_UTC) {
		child = child_ext.append_child("opencpn:time_display");
		child.append_child(pugi::node_pcdata).set_value(pRoute->m_TimeDisplayFormat.mb_str());
	}

	if (pRoute->m_width != Route::STYLE_UNDEFINED || pRoute->m_style != Route::STYLE_UNDEFINED) {
		child = child_ext.append_child("opencpn:style");

		if (pRoute->m_width != Route::STYLE_UNDEFINED)
			child.append_attribute("width") = pRoute->m_width;
		if (pRoute->m_style != Route::STYLE_UNDEFINED)
			child.append_attribute("style") = pRoute->m_style;
	}

	if (pRoute->m_Colour != wxEmptyString) {
		pugi::xml_node gpxx_ext = node.append_child("gpxx:RouteExtension");
		child = gpxx_ext.append_child("gpxx:DisplayColor");
		child.append_child(pugi::node_pcdata).set_value(pRoute->m_Colour.mb_str());
	}

	const RoutePointList& list = pRoute->routepoints();
	for (RoutePointList::const_iterator i = list.begin(); i != list.end(); ++i) {
		GPXCreateWpt(node.append_child("rtept"), *i, OPT_ROUTEPT);
	}

	return true;
}

void NavObjectCollection::InsertRouteA(Route* pTentRoute)
{
	if (!pTentRoute)
		return;

	bool bAddroute = true;
	// If the route has only 1 point, don't load it.
	if (pTentRoute->GetnPoints() < 2)
		bAddroute = false;

	// TODO  All this trouble for a tentative route.......Should make some Route methods????
	if (bAddroute) {
		// We are importing a different route with the same guid, so let's generate it a new guid
		if (g_pRouteMan->RouteExists(pTentRoute->guid())) {
			pTentRoute->set_guid(GpxDocument::GetUUID());
			// Now also change guids for the routepoints
			for (RoutePointList::iterator node = pTentRoute->routepoints().begin();
				 node != pTentRoute->routepoints().end(); ++node) {
				(*node)->set_guid(GpxDocument::GetUUID());
				// FIXME: !!!! the shared waypoint gets part of both the routes -> not goood at all
			}
		}

		pRouteList->push_back(pTentRoute);
		pTentRoute->RebuildGUIDList(); // ensure the GUID list is intact

		// Do the (deferred) calculation of BBox
		pTentRoute->CalculateBBox();

		// Add the selectable points and segments

		int ip = 0;
		geo::Position prev_pos;
		RoutePoint* prev_pConfPoint = NULL;

		for (RoutePointList::iterator node = pTentRoute->routepoints().begin();
			 node != pTentRoute->routepoints().end(); ++node) {
			RoutePoint* prp = *node;

			if (ip) {
				pSelect->AddSelectableRouteSegment(prev_pos, prp->get_position(),
												   prev_pConfPoint, prp,
												   pTentRoute);
			}
			pSelect->AddSelectableRoutePoint(prp->get_position(), prp);
			prev_pos = prp->get_position();
			prev_pConfPoint = prp;
			ip++;
		}
	} else {
		// walk the route, deleting points used only by this route
		for (RoutePointList::iterator node = pTentRoute->routepoints().begin();
			 node != pTentRoute->routepoints().end(); ++node) {
			RoutePoint* prp = *node;

			// check all other routes to see if this point appears in any other route
			Route* pcontainer_route = g_pRouteMan->FindRouteContainingWaypoint(prp);

			if (pcontainer_route == NULL) {
				prp->m_bIsInRoute = false; // Take this point out of this (and only) track/route
				if (!prp->m_bKeepXRoute) {
					pConfig->DeleteWayPoint(prp);
					delete prp;
				}
			}
		}
		delete pTentRoute;
	}
}

void NavObjectCollection::InsertTrack(Route* pTentTrack)
{
	if (!pTentTrack)
		return;

	bool bAddtrack = true;
	// If the track has only 1 point, don't load it.
	// This usually occurs if some points were discarded as being co-incident.
	if (pTentTrack->GetnPoints() < 2)
		bAddtrack = false;

	// TODO  All this trouble for a tentative route.......Should make some Route methods????
	if (bAddtrack) {
		pRouteList->push_back(pTentTrack);

		// Do the (deferred) calculation of Track BBox
		pTentTrack->CalculateBBox();

		// Add the selectable points and segments

		int ip = 0;
		geo::Position prev_pos;
		RoutePoint* prev_pConfPoint = NULL;

		for (RoutePointList::iterator node = pTentTrack->routepoints().begin();
			 node != pTentTrack->routepoints().end(); ++node) {
			RoutePoint* prp = *node;

			if (ip)
				pSelect->AddSelectableTrackSegment(prev_pos, prp->get_position(), prev_pConfPoint,
												   prp, pTentTrack);

			prev_pos = prp->get_position();
			prev_pConfPoint = prp;
			ip++;
		}
	} else {
		// walk the route, deleting points used only by this route
		for (RoutePointList::iterator node = pTentTrack->routepoints().begin();
			 node != pTentTrack->routepoints().end(); ++node) {
			RoutePoint* prp = *node;

			// check all other routes to see if this point appears in any other route
			Route* pcontainer_route = g_pRouteMan->FindRouteContainingWaypoint(prp);

			if (pcontainer_route == NULL) {
				prp->m_bIsInRoute = false; // Take this point out of this (and only) track/route
				if (!prp->m_bKeepXRoute) {
					pConfig->DeleteWayPoint(prp);
					delete prp;
				}
			}
		}
		delete pTentTrack;
	}
}

void NavObjectCollection::UpdateRouteA(Route* pTentRoute)
{
	Route* rt = g_pRouteMan->RouteExists(pTentRoute->guid());
	if (rt) {
		for (RoutePointList::iterator node = pTentRoute->routepoints().begin();
			 node != pTentRoute->routepoints().end(); ++node) {
			RoutePoint* prp = *node;
			RoutePoint* ex_rp = rt->GetPoint(prp->guid());
			if (ex_rp) {
				ex_rp->set_position(prp->get_position());
				ex_rp->set_icon_name(prp->icon_name());
				ex_rp->set_description(prp->get_description());
				ex_rp->SetName(prp->GetName());
			} else {
				pSelect->AddSelectableRoutePoint(prp->get_position(), prp);
			}
		}
	} else {
		InsertRouteA(pTentRoute);
	}
}

bool NavObjectCollection::CreateNavObjGPXPoints(void)
{
	// Iterate over the Routepoint list, creating Nodes for
	// Routepoints that are not in any Route
	// as indicated by isolated mark indicator (false)

	const RoutePointList& waypoints = pWayPointMan->waypoints();
	RoutePointList::const_iterator end = waypoints.end();
	for (RoutePointList::const_iterator i = waypoints.begin(); i != end; ++i) {
		const RoutePoint* point = *i;
		if (point->is_isolated() && !point->m_bIsInLayer && !point->m_btemp) {
			GPXCreateWpt(m_gpx_root.append_child("wpt"), point, OPT_WPT);
		}
	}

	return true;
}

bool NavObjectCollection::CreateNavObjGPXRoutes(void)
{
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* route = *i;
		if (!route->m_bIsTrack && !(route->m_bIsInLayer) && (!route->m_btemp))
			GPXCreateRoute(m_gpx_root.append_child("rte"), route);
	}
	return true;
}

bool NavObjectCollection::CreateNavObjGPXTracks(void)
{
	for (RouteList::iterator i = pRouteList->begin(); i != pRouteList->end(); ++i) {
		Route* track = *i;
		if (track->routepoints().size()) {
			if (track->m_bIsTrack && (!track->m_bIsInLayer) && (!track->m_btemp))
				GPXCreateTrk(m_gpx_root.append_child("trk"), track);
		}
	}
	return true;
}

bool NavObjectCollection::CreateAllGPXObjects()
{
	SetRootGPXNode();

	CreateNavObjGPXPoints();
	CreateNavObjGPXRoutes();
	CreateNavObjGPXTracks();

	return true;
}

bool NavObjectCollection::AddGPXRoute(Route* pRoute)
{
	SetRootGPXNode();
	GPXCreateRoute(m_gpx_root.append_child("rte"), pRoute);
	return true;
}

bool NavObjectCollection::AddGPXTrack(Track* pTrk)
{
	SetRootGPXNode();
	GPXCreateTrk(m_gpx_root.append_child("trk"), pTrk);
	return true;
}

bool NavObjectCollection::AddGPXWaypoint(const RoutePoint* pWP)
{
	SetRootGPXNode();
	GPXCreateWpt(m_gpx_root.append_child("wpt"), pWP, OPT_WPT);
	return true;
}

bool NavObjectCollection::AddGPXRoutesList(RouteList* pRoutes)
{
	SetRootGPXNode();

	for (RouteList::iterator route = pRoutes->begin(); route != pRoutes->end(); ++route) {
		Route* pRData = *route;
		if (!pRData->m_bIsTrack) {
			AddGPXRoute(pRData);
		} else {
			AddGPXTrack((Track*)pRData);
		}
	}

	return true;
}

bool NavObjectCollection::AddGPXPointsList(RoutePointList* pRoutePoints)
{
	SetRootGPXNode();
	for (RoutePointList::const_iterator i = pRoutePoints->begin(); i != pRoutePoints->end(); ++i) {
		AddGPXWaypoint(*i);
	}
	return true;
}

void NavObjectCollection::SetRootGPXNode(void)
{
	if(!strlen(m_gpx_root.name())) {
		m_gpx_root = append_child("gpx");
		m_gpx_root.append_attribute("version") = "1.1";
		m_gpx_root.append_attribute("creator") = "OpenCPN";
		m_gpx_root.append_attribute("xmlns:xsi") = "http://www.w3.org/2001/XMLSchema-instance";
		m_gpx_root.append_attribute("xmlns") = "http://www.topografix.com/GPX/1/1";
		m_gpx_root.append_attribute("xmlns:gpxx")
			= "http://www.garmin.com/xmlschemas/GpxExtensions/v3";
		m_gpx_root.append_attribute("xsi:schemaLocation")
			= "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd";
		m_gpx_root.append_attribute("xmlns:opencpn") = "http://www.opencpn.org";
	}
}

bool NavObjectCollection::SaveFile(const wxString filename)
{
	save_file(filename.fn_str(), "  ");
	return true;
}

bool NavObjectCollection::LoadAllGPXObjects()
{
	pugi::xml_node objects = this->child("gpx");

	for (pugi::xml_node object = objects.first_child(); object; object = object.next_sibling()) {
		if (!strcmp(object.name(), "wpt")) {
			RoutePoint* pWp
				= GPXLoadWaypoint1(object, _T("circle"), _T(""), false, false, false, 0);
			pWp->m_bIsolatedMark = true; // This is an isolated mark

			if (pWp) {
				RoutePoint* pExisting
					= pWayPointMan->WaypointExists(pWp->GetName(), pWp->get_position());
				if (!pExisting) {
					if (NULL != pWayPointMan)
						pWayPointMan->push_back(pWp);
					pSelect->AddSelectableRoutePoint(pWp->get_position(), pWp);
				} else
					delete pWp;
			}
		} else if (!strcmp(object.name(), "trk")) {
			Track* pTrack = GPXLoadTrack1(object, false, false, false, 0);
			InsertTrack(pTrack);
		} else if (!strcmp(object.name(), "rte")) {
			Route* pRoute = GPXLoadRoute1(object, false, false, false, 0);
			InsertRouteA(pRoute);
		}
	}

	return true;
}

int NavObjectCollection::LoadAllGPXObjectsAsLayer(int layer_id, bool b_layerviz)
{
	if (!pWayPointMan)
		return 0;

	int n_obj = 0;
	pugi::xml_node objects = this->child("gpx");

	for (pugi::xml_node object = objects.first_child(); object; object = object.next_sibling()) {
		if (!strcmp(object.name(), "wpt")) {
			RoutePoint* pWp
				= GPXLoadWaypoint1(object, _T("circle"), _T(""), true, true, b_layerviz, layer_id);
			pWp->m_bIsolatedMark = true; // This is an isolated mark

			if (pWp) {
				pWayPointMan->push_back(pWp);
				pSelect->AddSelectableRoutePoint(pWp->get_position(), pWp);
				n_obj++;
			} else
				delete pWp;
		} else {
			if (!strcmp(object.name(), "trk")) {
				Track* pTrack = GPXLoadTrack1(object, false, true, b_layerviz, layer_id);
				n_obj++;
				InsertTrack(pTrack);
			} else {
				if (!strcmp(object.name(), "rte")) {
					Route* pRoute = GPXLoadRoute1(object, true, true, b_layerviz, layer_id);
					n_obj++;
					InsertRouteA(pRoute);
				}
			}
		}
	}

	return n_obj;
}

