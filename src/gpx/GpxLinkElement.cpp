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

#include "GpxLinkElement.h"
#include "GpxSimpleElement.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ListOfGpxLinks);

GpxLinkElement::GpxLinkElement(
		const wxString & uri,
		const wxString & description,
		const wxString & mime_type)
	: TiXmlElement("link")
{
	SetAttribute("href", uri.ToUTF8()); //TODO: some checks?
	if(!description.IsEmpty()) {
		GpxSimpleElement * g = new GpxSimpleElement(wxString(_T("text")), description);
		LinkEndChild(g);
	}
	if(!mime_type.IsEmpty())
		LinkEndChild(new GpxSimpleElement(wxString(_T("type")), mime_type));
}

