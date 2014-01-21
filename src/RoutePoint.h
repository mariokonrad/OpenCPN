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

	void CalculateDCRect(wxDC& dc, wxRect* prect);

	bool IsSame(const RoutePoint* pOtherRP) const;
	bool IsVisible() const;
	bool IsListed() const;
	bool IsNameShown() const;
	void SetVisible(bool viz = true);
	void SetListed(bool viz = true);
	void SetNameShown(bool viz = true);
	wxString GetName(void) const;
	wxString GetDescription(void) const;

	void SetName(const wxString& name);

	void SetCourse(double course);
	double GetCourse() const;

	void SetDistance(double distance);
	double GetDistance() const;

	bool SendToGPS(const wxString& com_name, wxGauge* pProgress);

	const wxString& get_time_string() const;
	void set_time_string(const wxString& time_string);

	void clear_font();
	int get_layer_ID() const;
	void set_layer_ID(int);


	// FIXME: move attributes to private

	double m_seg_len; // length in NMI to this point, undefined for starting point
	double m_seg_vmg;
	wxDateTime m_seg_etd;

	bool m_bPtIsSelected;
	bool m_bIsBeingEdited;

	bool m_bIsInRoute;
	bool m_bIsInTrack;

	bool m_bIsolatedMark; // This is an isolated mark

	bool m_bKeepXRoute; // This is a mark which is part of a route/track
	//  and is also an isolated mark, so should not be deleted with route

	bool m_bIsVisible; // true if should be drawn, false if invisible
	bool m_bIsListed;
	bool m_bIsActive;
	wxString m_MarkDescription;
	wxString m_GUID;
	wxString m_IconName;

	wxBitmap* m_pbmIcon;
	bool m_bBlink;
	bool m_bDynamicName;
	bool m_bShowName;
	wxRect CurrentRect_in_DC;
	int m_NameLocationOffsetX;
	int m_NameLocationOffsetY;
	int m_GPXTrkSegNo;
	bool m_bIsInLayer;

	double m_routeprop_course; // course from this waypoint to the next waypoint if in a route.
	double m_routeprop_distance; // distance from this waypoint to the next waypoint if in a route.

	Hyperlinks m_HyperlinkList;
	bool m_btemp;

private:
	void CalculateNameExtents(void);

	geo::Position position;
	int m_LayerID;

	wxString m_MarkName;
	wxDateTime m_CreateTimeX;
	wxString m_timestring;

	wxFont* m_pMarkFont;
	wxColour m_FontColor;
	wxSize m_NameExtents;
};

typedef std::vector<RoutePoint*> RoutePointList;

#endif
