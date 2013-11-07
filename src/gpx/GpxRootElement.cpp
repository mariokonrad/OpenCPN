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

#include "GpxRootElement.h"
#include "GpxMetadataElement.h"
#include "GpxExtensionsElement.h"

GpxRootElement::GpxRootElement(
		const wxString & creator,
		GpxMetadataElement * metadata,
		ListOfGpxWpts * waypoints,
		ListOfGpxRoutes * routes,
		ListOfGpxTracks * tracks,
		GpxExtensionsElement * extensions)
	: TiXmlElement("gpx")
{
	my_extensions = NULL;
	my_metadata = NULL;
	first_waypoint = NULL;
	last_waypoint = NULL;
	first_route = NULL;
	last_route = NULL;
	first_track = NULL;
	last_track = NULL;

	SetAttribute ( "version", "1.1" );
	SetAttribute ( "creator", creator.ToUTF8() );
	SetAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
	SetAttribute( "xmlns", "http://www.topografix.com/GPX/1/1" );
	SetAttribute( "xmlns:gpxx", "http://www.garmin.com/xmlschemas/GpxExtensions/v3" );
	SetAttribute( "xsi:schemaLocation", "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd" );
	SetMetadata(metadata);
	if (waypoints) {
		for (ListOfGpxWpts::iterator waypoint = waypoints->begin(); waypoint != waypoints->end(); ++waypoint) {
			AddWaypoint(*waypoint);
		}
	}
	if (routes) {
		for (ListOfGpxRoutes::iterator route = routes->begin(); route != routes->end(); ++routes) {
			AddRoute(*route);
		}
	}
	if (tracks) {
		for (ListOfGpxTracks::iterator track = tracks->begin(); track != tracks->end(); ++track) {
			AddTrack(*track);
		}
	}
	SetExtensions(extensions);
}

void GpxRootElement::AddWaypoint(GpxWptElement *waypoint)
{
	bool b_was_cloned = true;

	if (last_waypoint)
		last_waypoint = (GpxWptElement *)InsertAfterChild(last_waypoint, *waypoint );
	else if (my_metadata)
		last_waypoint = (GpxWptElement *)InsertAfterChild(my_metadata, *waypoint);
	else if (first_route)
		last_waypoint = (GpxWptElement *)InsertBeforeChild(first_route, *waypoint);
	else if (first_track)
		last_waypoint = (GpxWptElement *)InsertBeforeChild(first_track, *waypoint);
	else if (my_extensions)
		last_waypoint = (GpxWptElement *)InsertBeforeChild(my_extensions, *waypoint);
	else
	{
		last_waypoint = (GpxWptElement *)LinkEndChild(waypoint);
		b_was_cloned = false;
	}

	if (!first_waypoint)
		first_waypoint = last_waypoint;

	if(b_was_cloned)
	{
		waypoint->Clear();
		delete waypoint;
	}
}

void GpxRootElement::AddRoute(GpxRteElement *route)
{
	bool b_was_cloned = true;

	if (last_route)
		last_route = (GpxRteElement *)InsertAfterChild(last_route, *route);
	else if (last_waypoint)
		last_route = (GpxRteElement *)InsertAfterChild(last_waypoint, *route);
	else if (my_metadata)
		last_route = (GpxRteElement *)InsertAfterChild(my_metadata, *route);
	else if (first_track)
		last_route = (GpxRteElement *)InsertBeforeChild(first_track, *route);
	else if (my_extensions)
		last_route = (GpxRteElement *)InsertBeforeChild(my_extensions, *route);
	else
	{
		last_route = (GpxRteElement *)LinkEndChild(route);
		b_was_cloned = false;
	}

	if (!first_route)
		first_route = last_route;

	if(b_was_cloned)
	{
		route->Clear();
		delete route;
	}
}

void GpxRootElement::AddTrack(GpxTrkElement *track)
{
	bool b_was_cloned = true;

	if (last_track)
		last_track = (GpxTrkElement *)InsertAfterChild(last_track, *track);
	else if (last_route)
		last_track = (GpxTrkElement *)InsertAfterChild(last_route, *track);
	else if (last_waypoint)
		last_track = (GpxTrkElement *)InsertAfterChild(last_waypoint, *track);
	else if (my_metadata)
		last_track = (GpxTrkElement *)InsertAfterChild(my_metadata, *track);
	else if (my_extensions)
		last_track = (GpxTrkElement *)InsertBeforeChild(my_extensions, *track);
	else
	{
		last_track = (GpxTrkElement *)LinkEndChild(track);
		b_was_cloned = false;
	}

	if (!first_track)
		first_track = last_track;

	if(b_was_cloned)
	{
		track->Clear();
		delete track;
	}
}

void GpxRootElement::SetMetadata(GpxMetadataElement *metadata)
{
	bool b_was_cloned = true;

	if (!metadata)
		RemoveMetadata();
	else
	{
		if(my_metadata)
			my_metadata = (GpxMetadataElement *)ReplaceChild(my_metadata, *metadata);
		else if (first_waypoint)
			my_metadata = (GpxMetadataElement *)InsertBeforeChild(first_waypoint, *metadata);
		else if (first_route)
			my_metadata = (GpxMetadataElement *)InsertBeforeChild(first_route, *metadata);
		else if (first_track)
			my_metadata = (GpxMetadataElement *)InsertBeforeChild(first_track, *metadata);
		else if (my_extensions)
			my_metadata = (GpxMetadataElement *)InsertBeforeChild(my_extensions, *metadata);
		else
		{
			b_was_cloned = false;
			my_metadata = (GpxMetadataElement *)LinkEndChild(metadata);
		}

		if(b_was_cloned)
		{
			metadata->Clear();
			delete metadata;
		}
	}
}

void GpxRootElement::RemoveMetadata()
{
	if(my_metadata)
		RemoveChild(my_metadata);
	delete my_metadata;
	my_metadata = NULL;
}

void GpxRootElement::SetExtensions(GpxExtensionsElement *extensions)
{
	if (!extensions)
		RemoveExtensions();
	else
	{
		if(!my_extensions)
			my_extensions = (GpxExtensionsElement *)LinkEndChild(extensions);
		else
		{
			my_extensions = (GpxExtensionsElement *)ReplaceChild(my_extensions, *extensions);
			extensions->Clear();
			delete extensions;
		}
	}
}

void GpxRootElement::RemoveExtensions()
{
	if(my_extensions)
		RemoveChild(my_extensions);
	delete my_extensions;
	my_extensions = NULL;
}

