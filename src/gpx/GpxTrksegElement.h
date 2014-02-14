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

#ifndef __GPX__GPXTRKSEGELEMENT__H__
#define __GPX__GPXTRKSEGELEMENT__H__

#include <tinyxml/tinyxml.h>
#include <gpx/GpxWptElement.h>

namespace gpx {

class GpxExtensionsElement;

class GpxTrksegElement : public TiXmlElement
{
public:
	GpxTrksegElement(ListOfGpxWpts* waypoints = NULL, GpxExtensionsElement* extensions = NULL);

	void AppendTrkPoint(GpxWptElement* trkpt);
};

WX_DECLARE_LIST(GpxTrksegElement, ListOfGpxTrksegs);

}

#endif
