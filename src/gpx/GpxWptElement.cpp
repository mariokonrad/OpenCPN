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

#include "GpxWptElement.h"
#include "GpxExtensionsElement.h"
#include "GpxSimpleElement.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ListOfGpxWpts);

GpxWptElement::GpxWptElement(
		char * waypoint_type,
		double lat,
		double lon,
		double ele,
		wxDateTime * time,
		double magvar,
		double geoidheight,
		const wxString & name,
		const wxString & cmt,
		const wxString & desc,
		const wxString & src,
		ListOfGpxLinks * links,
		const wxString & sym,
		const wxString & type,
		GpxFixType fixtype,
		int sat,
		double hdop,
		double vdop,
		double pdop,
		double ageofgpsdata,
		int dgpsid,
		GpxExtensionsElement * extensions)
	: TiXmlElement(waypoint_type)
{
	SetAttribute("lat", wxString::Format(_T("%.9f"), lat).ToUTF8());
	SetAttribute("lon", wxString::Format(_T("%.9f"), lon).ToUTF8());

	if (ele != 0)
		SetProperty(wxString(_T("ele")), wxString::Format(_T("%f"), ele));

	if (time)
		if (time->IsValid())
			SetProperty(wxString(_T("time")), time->FormatISODate().Append(_T("T")).Append(time->FormatISOTime()).Append(_T("Z")));
	if (magvar != 0)
		SetProperty(wxString(_T("magvar")), wxString::Format(_T("%f"), magvar));
	if (geoidheight != -1)
		SetProperty(wxString(_T("geoidheight")), wxString::Format(_T("%f"), geoidheight));
	if (!name.IsEmpty())
		SetProperty(wxString(_T("name")), name);
	if (!cmt.IsEmpty())
		SetProperty(wxString(_T("cmt")), cmt);
	if (!desc.IsEmpty())
		SetProperty(wxString(_T("desc")), desc);
	if (!src.IsEmpty())
		SetProperty(wxString(_T("src")), src);
	if (links) {
		wxListOfGpxLinksNode *link = links->GetFirst();
		while (link)
		{
			LinkEndChild(link->GetData());
			link = link->GetNext();
		}
	}
	if (!sym.IsEmpty() /*&& (sym != _T("empty"))*/) //"empty" is a valid symbol for us, we need to preserve it, otherwise it would be non existent and replaced by a circle on next load...
		SetProperty(wxString(_T("sym")), sym);
	if (!type.IsEmpty())
		SetProperty(wxString(_T("type")), type);
	if (fixtype != fix_undefined)
		SetProperty(wxString(_T("fix")), FixTypeToStr(fixtype));
	if (sat != -1)
		SetProperty(wxString(_T("sat")), wxString::Format(_T("%u"), sat));
	if (hdop != -1)
		SetProperty(wxString(_T("hdop")), wxString::Format(_T("%f"), hdop));
	if (vdop != -1)
		SetProperty(wxString(_T("vdop")), wxString::Format(_T("%f"), vdop));
	if (pdop != -1)
		SetProperty(wxString(_T("pdop")), wxString::Format(_T("%f"), pdop));
	if (ageofgpsdata != -1)
		SetProperty(wxString(_T("ageofgpsdata")), wxString::Format(_T("%f"), ageofgpsdata));
	if (dgpsid != -1)
		SetProperty(wxString(_T("dgpsid")), wxString::Format(_T("%u"), dgpsid));
	if (extensions)
		LinkEndChild(extensions);
}

wxString GpxWptElement::FixTypeToStr(GpxFixType fixtype)
{
	switch(fixtype)
	{
		case fix_none:
			return wxString(_T("none"));
		case fix_2d:
			return wxString(_T("2d"));
		case fix_3d:
			return wxString(_T("3d"));
		case fix_dgps:
			return wxString(_T("dgps"));
		case fix_pps:
			return wxString(_T("pps"));
		default:
			return wxString(_T(""));
	}
}

void GpxWptElement::SetSimpleExtension(const wxString &name, const wxString &value)
{
	//FIXME: if the extensions don't exist, we should create them
	TiXmlElement * exts = FirstChildElement("extensions");
	if (exts) {
		TiXmlElement * ext = exts->FirstChildElement(name.ToUTF8());
		if (ext)
			exts->ReplaceChild(ext, GpxSimpleElement(name, value));
		else
			exts->LinkEndChild(new GpxSimpleElement(name, value));
	}
}

void GpxWptElement::SetProperty(const wxString &name, const wxString &value)
{
	//FIXME: doesn't care about order so it can be absolutely wrong, have to redo this code if it has to be used by something else than the constructor
	//then it can be made public
	GpxSimpleElement *element = new GpxSimpleElement(name, value);
	TiXmlElement *curelement = FirstChildElement();
	bool found = false;
	while(curelement)
	{
		if((const char *)curelement->Value() == (const char *)name.ToUTF8())
		{
			ReplaceChild(curelement, *element);
			element->Clear();
			delete element;
			found = true;
			break;
		}
		curelement = curelement->NextSiblingElement();
	}
	if (!found)
		LinkEndChild(element);
}

