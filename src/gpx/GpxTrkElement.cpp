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

#include "GpxTrkElement.h"
#include <gpx/GpxExtensionsElement.h>
#include <gpx/GpxSimpleElement.h>
#include <wx/listimpl.cpp>

namespace gpx {

WX_DEFINE_LIST(ListOfGpxTracks);

GpxTrkElement::GpxTrkElement(const wxString& name, const wxString& cmt, const wxString& desc,
							 const wxString& src, ListOfGpxLinks* links, int number,
							 const wxString& type, GpxExtensionsElement* extensions,
							 ListOfGpxTrksegs* segments)
	: TiXmlElement("trk")
{
	if (!name.IsEmpty())
		SetProperty(wxString(_T("name")), name);
	if (!cmt.IsEmpty())
		SetProperty(wxString(_T("cmt")), cmt);
	if (!desc.IsEmpty())
		SetProperty(wxString(_T("desc")), desc);
	if (!src.IsEmpty())
		SetProperty(wxString(_T("src")), src);
	if (links) {
		for (ListOfGpxLinks::iterator link = links->begin(); link != links->end(); ++link) {
			LinkEndChild(*link);
		}
	}
	if (number != -1)
		SetProperty(wxString(_T("number")), wxString::Format(_T("%u"), number));
	if (!type.IsEmpty())
		SetProperty(wxString(_T("type")), type);
	if (extensions)
		LinkEndChild(extensions);
	if (segments) {
		for (ListOfGpxTrksegs::iterator seg = segments->begin(); seg != segments->end(); ++seg) {
			AppendTrkSegment(*seg);
		}
	}
}

void GpxTrkElement::AppendTrkSegment(GpxTrksegElement* trkseg)
{
	// FIXME: can be reused for route and track segment
	LinkEndChild(trkseg);
}

void GpxTrkElement::SetProperty(const wxString& name, const wxString& value)
{
	// FIXME: doesn't care about order so it can be absolutely wrong, have to redo this code if it
	// has to be used by something else than the constructor
	// then it can be made public
	// FIXME: can be reused for route and track
	GpxSimpleElement* element = new GpxSimpleElement(name, value);
	TiXmlElement* curelement = FirstChildElement();
	bool found = false;
	while (curelement) {
		if ((const char*)curelement->Value() == (const char*)name.ToUTF8()) {
			ReplaceChild(curelement, *element);
			element->Clear();
			delete element;
			break;
		}
		curelement = curelement->NextSiblingElement();
	}
	if (!found)
		LinkEndChild(element);
}

void GpxTrkElement::SetSimpleExtension(const wxString& name, const wxString& value)
{
	// FIXME: if the extensions don't exist, we should create them
	TiXmlElement* exts = FirstChildElement("extensions");
	if (exts) {
		TiXmlElement* ext = exts->FirstChildElement(name.ToUTF8());
		if (ext)
			exts->ReplaceChild(ext, GpxSimpleElement(name, value));
		else
			exts->LinkEndChild(new GpxSimpleElement(name, value));
	}
}

}

