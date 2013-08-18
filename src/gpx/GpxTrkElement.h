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

#ifndef __GPXTRKELEMENT__H__
#define __GPXTRKELEMENT__H__

#include <wx/string.h>
#include <tinyxml/tinyxml.h>
#include "GpxLinkElement.h"
#include "GpxTrksegElement.h"

class GpxExtensionsElement;

class GpxTrkElement : public TiXmlElement
{
	public:
		GpxTrkElement(
				const wxString & name = _T(""),
				const wxString & cmt = _T(""),
				const wxString & desc = _T(""),
				const wxString & src = _T(""),
				ListOfGpxLinks * links = NULL,
				int number = -1,
				const wxString & type = _T(""),
				GpxExtensionsElement * extensions = NULL,
				ListOfGpxTrksegs * segments = NULL);

		void AppendTrkSegment(GpxTrksegElement * trkseg);
		void SetSimpleExtension(const wxString & name, const wxString & value);

	private:
		void SetProperty(const wxString & name, const wxString & value);
};

WX_DECLARE_LIST(GpxTrkElement, ListOfGpxTracks);

#endif