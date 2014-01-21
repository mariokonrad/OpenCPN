/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __SELECTITEM_H__
#define __SELECTITEM_H__

#include <geo/Position.h>
#include <list>

class Route;
class RoutePoint;

class SelectItem
{
public:
	enum Type
	{
		TYPE_UNKNOWN      = 0x0001,
		TYPE_ROUTEPOINT   = 0x0002,
		TYPE_ROUTESEGMENT = 0x0004,
		TYPE_TIDEPOINT    = 0x0008,
		TYPE_CURRENTPOINT = 0x0010,
		TYPE_ROUTECREATE  = 0x0020,
		TYPE_AISTARGET    = 0x0040,
		TYPE_MARKPOINT    = 0x0080,
		TYPE_TRACKSEGMENT = 0x0100
	};

public:
	SelectItem();
	~SelectItem();

	int GetUserData(void) const;
	void SetUserData(int data);

	geo::Position pos1;
	geo::Position pos2;
	unsigned long m_seltype; // bitcombination of Type
	bool m_bIsSelected;

	// (mis-)used as one of the following:
	// - RoutePoint
	// - IDX_entry
	// - long (AIS: mmsi)
	// at the moment written only by Select
	const void* m_pData1; // FIXME: void*

	RoutePoint* route_point;
	Route* route;

private:
	int user_data;
};

typedef std::list<SelectItem*> SelectableItemList;

#endif
