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

#ifndef __SELECT__H__
#define __SELECT__H__

#include <SelectItem.h>

class Route;
class RoutePoint;

class Select
{
public:
	Select();
	~Select();

	void SetSelectPixelRadius(int radius);

	bool AddSelectableRoutePoint(const geo::Position& pos, RoutePoint* pRoutePointAdd);
	bool AddSelectableRouteSegment(const geo::Position& pos1, const geo::Position& pos2,
								   RoutePoint* pRoutePointAdd1, RoutePoint* pRoutePointAdd2,
								   Route* pRoute);

	bool AddSelectableTrackSegment(const geo::Position& pos1, const geo::Position& pos2,
								   RoutePoint* pRoutePointAdd1, RoutePoint* pRoutePointAdd2,
								   Route* pRoute);

	SelectItem* FindSelection(const geo::Position& pos, unsigned long fseltype);
	SelectableItemList FindSelectionList(const geo::Position& pos, unsigned long fseltype);

	bool DeleteAllSelectableRouteSegments(Route*);
	bool DeleteAllSelectableTrackSegments(Route*);
	bool DeleteAllSelectableRoutePoints(Route*);
	bool AddAllSelectableRouteSegments(Route* pr);
	bool AddAllSelectableTrackSegments(Route* pr);
	bool AddAllSelectableRoutePoints(Route* pr);
	bool UpdateSelectableRouteSegments(const RoutePoint* prp);
	bool DeletePointSelectableTrackSegments(RoutePoint* pr);
	bool IsSegmentSelected(float a, float b, float c, float d, const geo::Position& pos);
	bool IsSelectableSegmentSelected(const geo::Position& pos, SelectItem* pFindSel);

	// Generic Point Support
	// e.g. Tides/Currents and AIS Targets
	SelectItem* AddSelectablePoint(const geo::Position& pos, const void* data, SelectItem::Type fseltype);
	bool DeleteAllPoints(void);
	bool DeleteSelectablePoint(void* data, unsigned long SeltypeToDelete);
	bool ModifySelectablePoint(const geo::Position& pos, void* data, unsigned long fseltype);

	// Delete all selectable points in list by type
	bool DeleteAllSelectableTypePoints(unsigned long SeltypeToDelete);

private:
	void CalcSelectRadius();

	SelectableItemList select_items;
	int pixelRadius;
	float selectRadius;
};

#endif
