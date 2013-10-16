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

#include <wx/wx.h>

#include <Select.h>
#include <MicrosoftCompatibility.h>
#include <ChartCanvas.h>
#include <Vector2D.h>

#include <geo/GeoRef.h>

extern ChartCanvas * cc1;

Select::Select()
{
	pSelectList = new SelectableItemList;
	pixelRadius = 8;

	int w;
	int h;
	wxDisplaySize(&w, &h);
	if (h > 800)
		pixelRadius = 10;
	if (h > 1024)
		pixelRadius = 12;
}

Select::~Select()
{
	pSelectList->DeleteContents(true);
	pSelectList->clear();
	delete pSelectList;
}

void Select::SetSelectPixelRadius(int radius)
{
	pixelRadius = radius;
}

SelectableItemList * Select::GetSelectList()
{
	return pSelectList;
}

bool Select::AddSelectableRoutePoint(float slat, float slon, RoutePoint * pRoutePointAdd)
{
	SelectItem * pSelItem = new SelectItem;
	pSelItem->m_slat = slat;
	pSelItem->m_slon = slon;
	pSelItem->m_seltype = TYPE_ROUTEPOINT;
	pSelItem->m_bIsSelected = false;
	pSelItem->m_pData1 = pRoutePointAdd;

	if (pRoutePointAdd->m_bIsInLayer)
		pSelectList->Append(pSelItem);
	else
		pSelectList->Insert(pSelItem);

	return true;
}

bool Select::AddSelectableRouteSegment(
		float slat1,
		float slon1,
		float slat2,
		float slon2,
		RoutePoint * pRoutePointAdd1,
		RoutePoint * pRoutePointAdd2,
		Route *pRoute)
{
	SelectItem *pSelItem = new SelectItem;
	pSelItem->m_slat = slat1;
	pSelItem->m_slon = slon1;
	pSelItem->m_slat2 = slat2;
	pSelItem->m_slon2 = slon2;
	pSelItem->m_seltype = TYPE_ROUTESEGMENT;
	pSelItem->m_bIsSelected = false;
	pSelItem->m_pData1 = pRoutePointAdd1;
	pSelItem->m_pData2 = pRoutePointAdd2;
	pSelItem->m_pData3 = pRoute;

	if (pRoute->m_bIsInLayer)
		pSelectList->push_back(pSelItem);
	else
		pSelectList->Insert( pSelItem );

	return true;
}

bool Select::DeleteAllSelectableRouteSegments( Route *pr )
{
	SelectItem *pFindSel;

	//    Iterate on the select list
	wxSelectableItemListNode *node = pSelectList->GetFirst();

	while( node ) {
		pFindSel = node->GetData();
		if( pFindSel->m_seltype == TYPE_ROUTESEGMENT ) {
			if ((Route *) pFindSel->m_pData3 == pr) {
				delete pFindSel;
				pSelectList->DeleteNode( node );   //delete node;
				node = pSelectList->GetFirst();     // reset the top node
				goto got_next_outer_node;
			}
		}

		node = node->GetNext();
got_next_outer_node:
		continue;
	}

	return true;
}

bool Select::DeleteAllSelectableRoutePoints(Route * pr)
{
	// Iterate on the select list
	wxSelectableItemListNode * node = pSelectList->GetFirst();
	while (node) {
		SelectItem * pFindSel = node->GetData();
		if (pFindSel->m_seltype == TYPE_ROUTEPOINT) {
			RoutePoint * ps = (RoutePoint *) pFindSel->m_pData1;

			// inner loop iterates on the route's point list
			wxRoutePointListNode * pnode = pr->pRoutePointList->GetFirst();
			while (pnode) {
				RoutePoint * prp = pnode->GetData();

				if (prp == ps) {
					delete pFindSel;
					pSelectList->DeleteNode(node);
					node = pSelectList->GetFirst();

					goto got_next_outer_node;
				}
				pnode = pnode->GetNext();
			}
		}
		node = node->GetNext();
got_next_outer_node:
		continue;
	}
	return true;
}

bool Select::AddAllSelectableRoutePoints( Route *pr )
{
	if( pr->pRoutePointList->GetCount() ) {
		wxRoutePointListNode *node = ( pr->pRoutePointList )->GetFirst();
		while( node ) {
			RoutePoint *prp = node->GetData();
			AddSelectableRoutePoint( prp->m_lat, prp->m_lon, prp );
			node = node->GetNext();
		}
		return true;
	} else
		return false;
}

bool Select::AddAllSelectableRouteSegments( Route *pr )
{
	if (pr->pRoutePointList->GetCount()) {
		wxRoutePointListNode *node = ( pr->pRoutePointList )->GetFirst();

		RoutePoint *prp0 = node->GetData();
		float slat1 = prp0->m_lat;
		float slon1 = prp0->m_lon;

		node = node->GetNext();

		while( node ) {
			RoutePoint *prp = node->GetData();
			float slat2 = prp->m_lat;
			float slon2 = prp->m_lon;

			AddSelectableRouteSegment( slat1, slon1, slat2, slon2, prp0, prp, pr );

			slat1 = slat2;
			slon1 = slon2;
			prp0 = prp;

			node = node->GetNext();
		}
		return true;
	} else
		return false;
}

bool Select::AddAllSelectableTrackSegments( Route *pr )
{
	if( pr->pRoutePointList->GetCount() ) {
		wxRoutePointListNode *node = ( pr->pRoutePointList )->GetFirst();

		RoutePoint *prp0 = node->GetData();
		float slat1 = prp0->m_lat;
		float slon1 = prp0->m_lon;

		node = node->GetNext();

		while( node ) {
			RoutePoint *prp = node->GetData();
			float slat2 = prp->m_lat;
			float slon2 = prp->m_lon;

			AddSelectableTrackSegment(slat1, slon1, slat2, slon2, prp0, prp, pr);

			slat1 = slat2;
			slon1 = slon2;
			prp0 = prp;

			node = node->GetNext();
		}
		return true;
	} else
		return false;
}

bool Select::UpdateSelectableRouteSegments( RoutePoint *prp )
{
	bool ret = false;

	wxSelectableItemListNode *node = pSelectList->GetFirst();
	while (node) {
		SelectItem *pFindSel = node->GetData();
		if (pFindSel->m_seltype == TYPE_ROUTESEGMENT) {
			if (pFindSel->m_pData1 == prp) {
				pFindSel->m_slat = prp->m_lat;
				pFindSel->m_slon = prp->m_lon;
				ret = true;
			} else if (pFindSel->m_pData2 == prp) {
				pFindSel->m_slat2 = prp->m_lat;
				pFindSel->m_slon2 = prp->m_lon;
				ret = true;
			}
		}
		node = node->GetNext();
	}

	return ret;
}

SelectItem *Select::AddSelectablePoint(float slat, float slon, const void *pdata, int fseltype)
{
	SelectItem * pSelItem = new SelectItem;
	if (pSelItem) {
		pSelItem->m_slat = slat;
		pSelItem->m_slon = slon;
		pSelItem->m_seltype = fseltype;
		pSelItem->m_bIsSelected = false;
		pSelItem->m_pData1 = pdata;
		pSelectList->push_back(pSelItem);
	}
	return pSelItem;
}

bool Select::DeleteAllPoints(void)
{
	pSelectList->DeleteContents(true);
	pSelectList->clear();
	return true;
}

bool Select::DeleteSelectablePoint(void * pdata, int SeltypeToDelete)
{
	if (NULL == pdata)
		return false;

	wxSelectableItemListNode *node = pSelectList->GetFirst();
	while( node ) {
		SelectItem * pFindSel = node->GetData();
		if (pFindSel->m_seltype == SeltypeToDelete) {
			if (pdata == pFindSel->m_pData1) {
				delete pFindSel;
				delete node;
				return true;
			}
		}
		node = node->GetNext();
	}
	return false;
}

bool Select::DeleteAllSelectableTypePoints(int SeltypeToDelete)
{
	wxSelectableItemListNode *node = pSelectList->GetFirst();
	while( node ) {
		SelectItem * pFindSel = node->GetData();
		if( pFindSel->m_seltype == SeltypeToDelete ) {
			delete pFindSel;
			delete node;
			node = pSelectList->GetFirst();
			continue;
		}

		node = node->GetNext();
	}
	return true;
}

bool Select::ModifySelectablePoint(float lat, float lon, void *data, int SeltypeToModify)
{
	for (SelectableItemList::iterator i = pSelectList->begin(); i != pSelectList->end(); ++i) {
		SelectItem * pFindSel = *i;
		if (pFindSel->m_seltype == SeltypeToModify) {
			if (data == pFindSel->m_pData1) {
				pFindSel->m_slat = lat;
				pFindSel->m_slon = lon;
				return true;
			}
		}
	}
	return false;
}

bool Select::AddSelectableTrackSegment(
		float slat1,
		float slon1,
		float slat2,
		float slon2,
		RoutePoint * pRoutePointAdd1,
		RoutePoint * pRoutePointAdd2,
		Route * pRoute)
{
	SelectItem *pSelItem = new SelectItem;
	pSelItem->m_slat = slat1;
	pSelItem->m_slon = slon1;
	pSelItem->m_slat2 = slat2;
	pSelItem->m_slon2 = slon2;
	pSelItem->m_seltype = TYPE_TRACKSEGMENT;
	pSelItem->m_bIsSelected = false;
	pSelItem->m_pData1 = pRoutePointAdd1;
	pSelItem->m_pData2 = pRoutePointAdd2;
	pSelItem->m_pData3 = pRoute;

	if (pRoute->m_bIsInLayer)
		pSelectList->Append(pSelItem);
	else
		pSelectList->Insert(pSelItem);

	return true;
}

bool Select::DeleteAllSelectableTrackSegments(Route * pr)
{
	SelectItem *pFindSel;

	//    Iterate on the select list
	wxSelectableItemListNode *node = pSelectList->GetFirst();

	while( node ) {
		pFindSel = node->GetData();
		if( pFindSel->m_seltype == TYPE_TRACKSEGMENT ) {

			if( (Route *) pFindSel->m_pData3 == pr ) {
				delete pFindSel;
				pSelectList->DeleteNode( node );   //delete node;

				node = pSelectList->GetFirst();     // reset the top node
				continue;
			}
		}
		node = node->GetNext();
	}
	return true;
}

bool Select::DeletePointSelectableTrackSegments(RoutePoint * pr)
{
	SelectItem *pFindSel;

	//    Iterate on the select list
	wxSelectableItemListNode *node = pSelectList->GetFirst();

	while( node ) {
		pFindSel = node->GetData();
		if( pFindSel->m_seltype == TYPE_TRACKSEGMENT ) {

			if( (RoutePoint *) pFindSel->m_pData1 == pr || (RoutePoint *) pFindSel->m_pData2 == pr ) {
				delete pFindSel;
				pSelectList->DeleteNode( node );   //delete node;

				node = pSelectList->GetFirst();     // reset the top node
				continue;
			}
		}
		node = node->GetNext();
	}
	return true;
}

bool Select::IsSegmentSelected(float a, float b, float c, float d, float slat, float slon)
{
	double adder = 0.0;

	if( ( c * d ) < 0.0) {
		//    Arrange for points to be increasing longitude, c to d
		double dist;
		double brg;
		geo::DistanceBearingMercator( a, c, b, d, &brg, &dist );
		if( brg < 180.0)             // swap points?
		{
			double tmp;
			tmp = c;
			c = d;
			d = tmp;
			tmp = a;
			a = b;
			b = tmp;
		}
		if( d < 0.0)     // idl?
		{
			d += 360.0;
			if( slon < 0.0)
				adder = 360.0;
		}
	}

	//    As a course test, use segment bounding box test
	if( ( slat >= ( fmin ( a,b ) - selectRadius ) ) && ( slat <= ( fmax ( a,b ) + selectRadius ) )
			&& ( ( slon + adder ) >= ( fmin ( c,d ) - selectRadius ) )
			&& ( ( slon + adder ) <= ( fmax ( c,d ) + selectRadius ) ) ) {
		//    Use vectors to do hit test....
		Vector2D va;
		Vector2D vb;
		Vector2D vn;

		//    Assuming a Mercator projection
		double ap, cp;
		geo::toSM( a, c, 0., 0., &cp, &ap );
		double bp, dp;
		geo::toSM( b, d, 0., 0., &dp, &bp );
		double slatp, slonp;
		geo::toSM( slat, slon + adder, 0., 0., &slonp, &slatp );

		va.x = slonp - cp;
		va.y = slatp - ap;
		vb.x = dp - cp;
		vb.y = bp - ap;

		double delta = vGetLengthOfNormal( &va, &vb, &vn );
		if (fabs(delta) < (selectRadius * 1852 * 60))
			return true;
	}
	return false;
}

void Select::CalcSelectRadius()
{
	selectRadius = pixelRadius / ( cc1->GetCanvasTrueScale() * 1852 * 60 );
}

SelectItem * Select::FindSelection(float slat, float slon, int fseltype)
{
	CalcSelectRadius();

	for (SelectableItemList::iterator i = pSelectList->begin(); i != pSelectList->end(); ++i) {
		SelectItem * pFindSel = *i;
		if (pFindSel->m_seltype == fseltype) {
			switch (fseltype) {
				case TYPE_ROUTEPOINT:
				case TYPE_TIDEPOINT:
				case TYPE_CURRENTPOINT:
				case TYPE_AISTARGET:
					if ((fabs(slat - pFindSel->m_slat) < selectRadius) && (fabs(slon - pFindSel->m_slon) < selectRadius))
						return pFindSel;
					break;

				case TYPE_ROUTESEGMENT:
				case TYPE_TRACKSEGMENT:
					if (IsSegmentSelected(
							pFindSel->m_slat,
							pFindSel->m_slat2,
							pFindSel->m_slon,
							pFindSel->m_slon2,
							slat, slon))
						return pFindSel;
					break;

				default:
					break;
			}
		}
	}

	return NULL;
}

bool Select::IsSelectableSegmentSelected(float slat, float slon, SelectItem * pFindSel)
{
	CalcSelectRadius();
	return IsSegmentSelected(
			pFindSel->m_slat,
			pFindSel->m_slat2,
			pFindSel->m_slon,
			pFindSel->m_slon2,
			slat,
			slon);
}

SelectableItemList Select::FindSelectionList(float slat, float slon, int fseltype)
{
	SelectableItemList ret_list;

	CalcSelectRadius();

	for (SelectableItemList::iterator i = pSelectList->begin(); i != pSelectList->end(); ++i) {
		SelectItem * pFindSel = *i;
		if (pFindSel->m_seltype != fseltype)
			continue;

		switch (fseltype) {
			case TYPE_ROUTEPOINT:
			case TYPE_TIDEPOINT:
			case TYPE_CURRENTPOINT:
			case TYPE_AISTARGET:
				if ((fabs(slat - pFindSel->m_slat) < selectRadius) && (fabs(slon - pFindSel->m_slon) < selectRadius)) {
					ret_list.push_back(pFindSel);
				}
				break;

			case TYPE_ROUTESEGMENT:
			case TYPE_TRACKSEGMENT:
				if (IsSegmentSelected(
							pFindSel->m_slat,
							pFindSel->m_slat2,
							pFindSel->m_slon,
							pFindSel->m_slon2,
							slat, slon))
					ret_list.push_back(pFindSel);
				break;

			default:
				break;
		}
	}

	return ret_list;
}

