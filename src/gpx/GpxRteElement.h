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

#ifndef __GPX__GPXRTEELEMENT__H__
#define __GPX__GPXRTEELEMENT__H__

#include <wx/string.h>
#include <tinyxml/tinyxml.h>
#include <gpx/GpxWptElement.h>
#include <gpx/GpxLinkElement.h>

namespace gpx {

class GpxExtensionsElement;

// FIXME: constructor has way too many parameters

class GpxRteElement : public TiXmlElement
{
public:
	GpxRteElement(const wxString& name = _T(""), const wxString& cmt = _T(""),
				  const wxString& desc = _T(""), const wxString& src = _T(""),
				  ListOfGpxLinks* links = NULL, int number = -1, const wxString& type = _T(""),
				  GpxExtensionsElement* extensions = NULL, ListOfGpxWpts* waypoints = NULL);

	void AppendRtePoint(GpxWptElement* rtept);
	void SetSimpleExtension(const wxString& name, const wxString& value);

private:
	void SetProperty(const wxString& name, const wxString& value);
};

WX_DECLARE_LIST(GpxRteElement, ListOfGpxRoutes);

}

#endif
