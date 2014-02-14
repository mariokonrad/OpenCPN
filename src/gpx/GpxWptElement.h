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

#ifndef __GPX__GPXWPTELEMENT__H__
#define __GPX__GPXWPTELEMENT__H__

#include <wx/string.h>
#include <wx/datetime.h>
#include <tinyxml/tinyxml.h>
#include <gpx/GpxLinkElement.h>

namespace gpx {

// Waypoint types
#define GPX_WPT_WAYPOINT (char*) "wpt"
#define GPX_WPT_ROUTEPOINT (char*) "rtept"
#define GPX_WPT_TRACKPOINT (char*) "trkpt"

class GpxExtensionsElement;

enum GpxFixType {
	fix_undefined,
	fix_none,
	fix_2d,
	fix_3d,
	fix_dgps,
	fix_pps
};

// FIXME: constructor has way too many parameters

class GpxWptElement : public TiXmlElement
{
public:
	GpxWptElement(char* waypoint_type, double lat, double lon, double ele = 0,
				  wxDateTime* time = NULL, double magvar = 0, double geoidheight = -1,
				  const wxString& name = _T(""), const wxString& cmt = _T(""),
				  const wxString& desc = _T(""), const wxString& src = _T(""),
				  ListOfGpxLinks* links = NULL, const wxString& sym = _T(""),
				  const wxString& type = _T(""), GpxFixType fixtype = fix_undefined, int sat = -1,
				  double hdop = -1, double vdop = -1, double pdop = -1, double ageofgpsdata = -1,
				  int dgpsid = -1, GpxExtensionsElement* extensions = NULL);

	void SetSimpleExtension(const wxString& name, const wxString& value);

private:
	void SetProperty(const wxString& name, const wxString& value);

	wxString FixTypeToStr(GpxFixType fixtype);
};

WX_DECLARE_LIST(GpxWptElement, ListOfGpxWpts);

}

#endif
