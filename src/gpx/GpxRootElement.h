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

#ifndef __GPXROOTELEMENT__H__
#define __GPXROOTELEMENT__H__

#include <wx/string.h>
#include <tinyxml/tinyxml.h>
#include "GpxWptElement.h"
#include "GpxRteElement.h"
#include "GpxTrkElement.h"

class GpxMetadataElement;

class GpxRootElement : public TiXmlElement
{
	public:
		GpxRootElement(
				const wxString & creator,
				GpxMetadataElement * metadata = NULL,
				ListOfGpxWpts * waypoints = NULL,
				ListOfGpxRoutes * routes = NULL,
				ListOfGpxTracks * tracks = NULL,
				GpxExtensionsElement * extensions = NULL);

		void AddWaypoint(GpxWptElement * waypoint);
		void AddRoute(GpxRteElement * route);
		void AddTrack(GpxTrkElement * track);
		void SetMetadata(GpxMetadataElement * metadata);
		void RemoveMetadata();
		void SetExtensions(GpxExtensionsElement * extensions);
		void RemoveExtensions();
	private:
		GpxWptElement * first_waypoint;
		GpxWptElement * last_waypoint;
		GpxRteElement * first_route;
		GpxRteElement * last_route;
		GpxTrkElement * first_track;
		GpxTrkElement * last_track;
		GpxMetadataElement * my_metadata;
		GpxExtensionsElement * my_extensions;
};

#endif
