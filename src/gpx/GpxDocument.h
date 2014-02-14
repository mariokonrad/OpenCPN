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

#ifndef __GPX__GPXDOCUMENT__H__
#define __GPX__GPXDOCUMENT__H__

#include <wx/string.h>
#include <tinyxml/tinyxml.h>

namespace gpx {

class GpxDocument : public TiXmlDocument
{
public:
	GpxDocument();
	virtual ~GpxDocument();

	bool LoadFile(const wxString& filename);
	bool SaveFile(const wxString& filename);
	void AddCustomNamespace(const wxString& name, const wxString& url);

	// RFC4122 version 4 compliant random UUIDs generator.
	static wxString GetUUID(void);

private:
	static int GetRandomNumber(int min, int max);
	void PopulateEmptyDocument(const wxString& creator);
	void SeedRandom();
};

}

#endif
