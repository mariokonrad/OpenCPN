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

#ifndef __GPXMETADATAELEMENT__H__
#define __GPXMETADATAELEMENT__H__

#include <wx/string.h>
#include <wx/datetime.h>
#include <tinyxml/tinyxml.h>

class GpxPersonElement;
class GpxCopyrightElement;
class GpxLinkElement;
class GpxBoundsElement;
class GpxExtensionsElement;

class GpxMetadataElement : public TiXmlElement
{
	public:
		GpxMetadataElement(
			const wxString & name,
			const wxString & desc,
			GpxPersonElement * author,
			GpxCopyrightElement * copyright,
			GpxLinkElement * link,
			wxDateTime * time,
			const wxString & keywords,
			GpxBoundsElement * bounds,
			GpxExtensionsElement * extensions);
};

#endif
