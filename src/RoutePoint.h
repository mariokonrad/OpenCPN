/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __ROUTEPOINT_H__
#define __ROUTEPOINT_H__

#include <Hyperlink.h>
#include <geo/Position.h>

#include <vector>

#include <wx/string.h>
#include <wx/datetime.h>
#include <wx/gdicmn.h>
#include <wx/gauge.h>

class ocpnDC;
class wxDC;

class RoutePoint
{
public:
	class SameGUID
	{
	public:
		SameGUID(const wxString& guid);
		bool operator()(const RoutePoint* point) const;

	private:
		wxString guid;
	};

	// Route segment to the current routepoint.
	class Segment
	{
	public:
		Segment();

		double length; // length of the segment in nautical miles
		double vmg;
		wxDateTime etd; // estimated time to destination
	};

public:
	RoutePoint(const geo::Position& pos, const wxString& icon_ident, const wxString& name,
			   const wxString& pGUID = _T(""), bool bAddToList = true);
	RoutePoint(const RoutePoint& orig);
	RoutePoint();
	~RoutePoint(void);

	void Draw(ocpnDC& dc, wxPoint* rpn = NULL);
	void ReLoadIcon(void);

	const wxDateTime& GetCreateTime(void) const;
	void SetCreateTime(wxDateTime dt);

	const geo::Position& get_position() const;
	void set_position(const geo::Position& pos);
	double latitude() const;
	double longitude() const;

	void CalculateDCRect(wxDC& dc, wxRect& prect); // FIXME: this 'calculation' uses Draw, having side effects

	bool IsSame(const RoutePoint* pOtherRP) const;
	bool IsVisible() const;
	bool IsListed() const;
	bool IsNameShown() const;
	void SetVisible(bool viz = true);
	void SetListed(bool viz = true);
	void SetNameShown(bool viz = true);
	const wxString& GetName(void) const;

	void SetName(const wxString& name);

	void set_description(const wxString& desc);
	const wxString& get_description(void) const;

	void SetCourse(double course);
	double GetCourse() const;

	void SetDistance(double distance);
	double GetDistance() const;

	bool SendToGPS(const wxString& com_name, wxGauge* pProgress); // FIXME: this class does way too much

	const wxString& get_time_string() const;
	void set_time_string(const wxString& time_string);

	void clear_font();
	int get_layer_ID() const;
	void set_layer_ID(int);

	bool is_in_route() const;
	bool is_in_track() const;

	bool is_isolated() const;

	const wxString& guid() const;
	void set_guid(const wxString&);

	const wxString& icon_name() const;
	void set_icon_name(const wxString&);

	bool is_blink() const;
	void set_blink(bool);

	bool is_visible() const;
	void set_visible(bool);

	bool is_show_name() const;
	void set_show_name(bool);

	bool is_dynamic_name() const;
	void set_dynamic_name(bool);

	const Hyperlinks& get_hyperlinks() const;
	void add_link(const Hyperlink&);

	// FIXME: move attributes to private

	Segment segment; // segment from the last route point to this one, invalid for the starting point

	bool m_bPtIsSelected;
	bool m_bIsBeingEdited;

	bool m_bIsInRoute; // FIXME: resolve this
	bool m_bIsInTrack; // FIXME: resolve this

	bool m_bIsolatedMark; // This is an isolated mark

	// This is a mark which is part of a route/track and is also an isolated mark,
	// so should not be deleted with route
	bool m_bKeepXRoute;

	bool m_bIsListed;
	bool m_bIsActive;

	wxBitmap* m_pbmIcon;
	wxRect CurrentRect_in_DC;
	int m_NameLocationOffsetX;
	int m_NameLocationOffsetY;
	int m_GPXTrkSegNo;
	bool m_bIsInLayer;

	double m_routeprop_course; // course from this waypoint to the next waypoint if in a route.
	double m_routeprop_distance; // distance from this waypoint to the next waypoint if in a route.

	bool m_btemp;

	Hyperlinks m_HyperlinkList; // FIXME: move this to private
private:
	void CalculateNameExtents(void);

	geo::Position position;
	int m_LayerID;

	bool m_bBlink;
	bool m_bIsVisible; // true if should be drawn, false if invisible
	bool m_bShowName;
	bool m_bDynamicName;
	wxString m_MarkName;
	wxDateTime m_CreateTimeX;
	wxString m_timestring;
	wxString m_MarkDescription;
	wxString m_IconName;
	wxString m_GUID;

	wxFont* m_pMarkFont;
	wxColour m_FontColor;
	wxSize m_NameExtents;
};

typedef std::vector<RoutePoint*> RoutePointList;

#endif
