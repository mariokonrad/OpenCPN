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

#ifndef __SELECT_H__
#define __SELECT_H__

#include "SelectItem.h"
#include "RoutePoint.h"
#include "Route.h"

class Select
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
		Select();
		~Select();

		void SetSelectPixelRadius(int radius);

		bool AddSelectableRoutePoint( float slat, float slon, RoutePoint *pRoutePointAdd );
		bool AddSelectableRouteSegment( float slat1, float slon1, float slat2, float slon2,
				RoutePoint *pRoutePointAdd1, RoutePoint *pRoutePointAdd2, Route *pRoute );

		bool AddSelectableTrackSegment( float slat1, float slon1, float slat2, float slon2,
				RoutePoint *pRoutePointAdd1, RoutePoint *pRoutePointAdd2, Route *pRoute );

		SelectItem *FindSelection( float slat, float slon, int fseltype );
		SelectableItemList FindSelectionList( float slat, float slon, int fseltype );

		bool DeleteAllSelectableRouteSegments( Route * );
		bool DeleteAllSelectableTrackSegments( Route * );
		bool DeleteAllSelectableRoutePoints( Route * );
		bool AddAllSelectableRouteSegments( Route *pr );
		bool AddAllSelectableTrackSegments( Route *pr );
		bool AddAllSelectableRoutePoints( Route *pr );
		bool UpdateSelectableRouteSegments( RoutePoint *prp );
		bool DeletePointSelectableTrackSegments( RoutePoint *pr );
		bool IsSegmentSelected( float a, float b, float c, float d, float slat, float slon );
		bool IsSelectableSegmentSelected( float slat, float slon, SelectItem *pFindSel );

		//    Generic Point Support
		//      e.g. Tides/Currents and AIS Targets
		SelectItem *AddSelectablePoint(float slat, float slon, const void *data, int fseltype);
		bool DeleteAllPoints( void );
		bool DeleteSelectablePoint( void *data, int SeltypeToDelete );
		bool ModifySelectablePoint( float slat, float slon, void *data, int fseltype );

		//    Delete all selectable points in list by type
		bool DeleteAllSelectableTypePoints( int SeltypeToDelete );

		SelectableItemList * GetSelectList();

	private:
		void CalcSelectRadius();

		SelectableItemList * pSelectList;
		int pixelRadius;
		float selectRadius;
};

#endif
