/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef KML_H
#define KML_H

#include <vector>
#include "tinyxml/tinyxml.h"
#include <wx/string.h>

#define KML_INSERT_EXTRADATA true

class Route;
class RoutePoint;
class Track;

enum KmlPastebufferType {
	KML_PASTE_WAYPOINT,
	KML_PASTE_ROUTE,
	KML_PASTE_TRACK,
	KML_PASTE_ROUTE_TRACK,
	KML_PASTE_INVALID,
	KML_COPY_STANDARD,
	KML_COPY_EXTRADATA
};

/// Kml parsing
///
/// see http://en.wikipedia.org/wiki/Keyhole_Markup_Language
class Kml
{
	private:
		class dPoint
		{
			public:
				double x,y,z;
		};
		typedef std::vector<dPoint> dPointList;

	public:
		Kml();
		~Kml();
		KmlPastebufferType ParsePasteBuffer();
		Route* GetParsedRoute() { return parsedRoute; }
		Track* GetParsedTrack() { return parsedTrack; }
		RoutePoint* GetParsedRoutePoint() { return parsedRoutePoint; }

		static wxString MakeKmlFromRoute(Route * route, bool insertSeqNames = false);
		static wxString MakeKmlFromTrack(Track * track);
		static wxString MakeKmlFromWaypoint(RoutePoint * routepoint);
		static void CopyWaypointToClipboard(RoutePoint * routepoint);
		static void CopyRouteToClipboard(Route * route);
		static void CopyTrackToClipboard(Track * route);

	private:
		KmlPastebufferType ParseOnePlacemarkPoint(TiXmlNode* node, wxString& name);
		KmlPastebufferType ParseTrack(TiXmlNode* node, wxString& name);
		int ParseCoordinates(TiXmlNode* node, dPointList& points);
		static TiXmlElement * StandardHead(TiXmlDocument& xmlDoc, wxString name);
		static std::string PointPlacemark(TiXmlElement* document, const RoutePoint * routepoint);

		wxString kmlText;
		RoutePoint * parsedRoutePoint;
		Route * parsedRoute;
		Track * parsedTrack;
		static bool insertQtVlmExtendedData;
		static int seqCounter;
};

#endif
