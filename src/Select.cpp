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

extern ChartCanvas* cc1;

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
	DeleteAllPoints();
	delete pSelectList;
	pSelectList = NULL;
}

void Select::SetSelectPixelRadius(int radius)
{
	pixelRadius = radius;
}

bool Select::AddSelectableRoutePoint(float slat, float slon, RoutePoint* pRoutePointAdd)
{
	SelectItem* pSelItem = new SelectItem;
	pSelItem->m_slat = slat;
	pSelItem->m_slon = slon;
	pSelItem->m_seltype = TYPE_ROUTEPOINT;
	pSelItem->m_bIsSelected = false;
	pSelItem->m_pData1 = pRoutePointAdd;

	if (pRoutePointAdd->m_bIsInLayer)
		pSelectList->push_back(pSelItem);
	else
		pSelectList->push_front(pSelItem);

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
	SelectItem* pSelItem = new SelectItem;
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
		pSelectList->push_front(pSelItem);

	return true;
}

bool Select::DeleteAllSelectableRouteSegments(Route* pr)
{
	// FIXME: refactor, use std algorithms
	SelectableItemList::iterator i = pSelectList->begin();
	while (i != pSelectList->end()) {
		SelectItem* item = *i;
		if (item->m_seltype == TYPE_ROUTESEGMENT) {
			if (reinterpret_cast<Route*>(item->m_pData3) == pr) {
				delete item;
				pSelectList->erase(i);
				i = pSelectList->begin();
				continue;
			}
		}
		++i;
	}
	return true;
}

bool Select::DeleteAllSelectableRoutePoints(Route* pr)
{
	// FIXME: refactor, use std algorithms
	SelectableItemList::iterator i = pSelectList->begin();
	while (i != pSelectList->end()) {
		SelectItem* item = *i;
		if (item->m_seltype == TYPE_ROUTEPOINT) {
			RoutePoint* ps = (RoutePoint*)item->m_pData1;

			// FIXME: replace with std::find
			RoutePointList::iterator j = pr->pRoutePointList->begin();
			for (; j != pr->pRoutePointList->end(); ++j) {
				if (*j == ps)
					break;
			}

			if (j != pr->pRoutePointList->end()) {
				delete item;
				pSelectList->erase(i);
				i = pSelectList->begin();
				continue;
			}
		}
		++i;
	}
	return true;
}

bool Select::AddAllSelectableRoutePoints(Route* pr)
{
	if (pr->pRoutePointList->size() == 0)
		return false;

	for (RoutePointList::iterator i = pr->pRoutePointList->begin(); i != pr->pRoutePointList->end();
		 ++i) {
		AddSelectableRoutePoint((*i)->m_lat, (*i)->m_lon, *i);
	}

	return true;
}

bool Select::AddAllSelectableRouteSegments(Route* pr)
{
	if (pr->pRoutePointList->size() == 0)
		return false;

	wxRoutePointListNode* node = (pr->pRoutePointList)->GetFirst();

	RoutePoint* prp0 = node->GetData();
	float slat1 = prp0->m_lat;
	float slon1 = prp0->m_lon;

	node = node->GetNext();

	while (node) {
		RoutePoint* prp = node->GetData();
		float slat2 = prp->m_lat;
		float slon2 = prp->m_lon;

		AddSelectableRouteSegment(slat1, slon1, slat2, slon2, prp0, prp, pr);

		slat1 = slat2;
		slon1 = slon2;
		prp0 = prp;

		node = node->GetNext();
	}
	return true;
}

bool Select::AddAllSelectableTrackSegments(Route* pr)
{
	if (pr->pRoutePointList->size() == 0)
		return false;

	wxRoutePointListNode* node = (pr->pRoutePointList)->GetFirst();

	RoutePoint* prp0 = node->GetData();
	float slat1 = prp0->m_lat;
	float slon1 = prp0->m_lon;

	node = node->GetNext();

	while (node) {
		RoutePoint* prp = node->GetData();
		float slat2 = prp->m_lat;
		float slon2 = prp->m_lon;

		AddSelectableTrackSegment(slat1, slon1, slat2, slon2, prp0, prp, pr);

		slat1 = slat2;
		slon1 = slon2;
		prp0 = prp;

		node = node->GetNext();
	}
	return true;
}

bool Select::UpdateSelectableRouteSegments(RoutePoint* prp)
{
	bool ret = false;

	for (SelectableItemList::iterator i = pSelectList->begin(); i != pSelectList->end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype == TYPE_ROUTESEGMENT) {
			if (item->m_pData1 == prp) {
				item->m_slat = prp->m_lat;
				item->m_slon = prp->m_lon;
				ret = true;
			} else if (item->m_pData2 == prp) {
				item->m_slat2 = prp->m_lat;
				item->m_slon2 = prp->m_lon;
				ret = true;
			}
		}
	}

	return ret;
}

SelectItem* Select::AddSelectablePoint(float slat, float slon, const void* pdata, int fseltype)
{
	SelectItem* pSelItem = new SelectItem;
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
	for (SelectableItemList::iterator i = pSelectList->begin(); i != pSelectList->end(); ++i)
		delete *i;
	pSelectList->clear();
	return true;
}

bool Select::DeleteSelectablePoint(void* pdata, int SeltypeToDelete)
{
	if (NULL == pdata)
		return false;

	// FIXME: refactor, use std algorithms
	for (SelectableItemList::iterator i = pSelectList->begin(); i != pSelectList->end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype == SeltypeToDelete) {
			if (pdata == item->m_pData1) {
				delete item;
				pSelectList->erase(i);
				return true;
			}
		}
	}
	return false;
}

bool Select::DeleteAllSelectableTypePoints(int SeltypeToDelete)
{
	// FIXME: refactor, use std algorithms
	SelectableItemList::iterator i = pSelectList->begin();
	while (i != pSelectList->end()) {
		SelectItem* item = *i;
		if (item->m_seltype == SeltypeToDelete) {
			delete item;
			pSelectList->erase(i);
			i = pSelectList->begin();
			continue;
		}
		++i;
	}
	return true;
}

bool Select::ModifySelectablePoint(float lat, float lon, void* data, int SeltypeToModify)
{
	// FIXME: refactor, use std algorithms
	for (SelectableItemList::iterator i = pSelectList->begin(); i != pSelectList->end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype == SeltypeToModify) {
			if (data == item->m_pData1) {
				item->m_slat = lat;
				item->m_slon = lon;
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
		pSelectList->push_back(pSelItem);
	else
		pSelectList->push_front(pSelItem);

	return true;
}

bool Select::DeleteAllSelectableTrackSegments(Route* pr)
{
	// FIXME: refactor, use std algorithms
	SelectableItemList::iterator i = pSelectList->begin();
	while (i != pSelectList->end()) {
		SelectItem* item = *i;
		if (item->m_seltype == TYPE_TRACKSEGMENT) {
			if (((Route*)item->m_pData3) == pr) {
				delete item;
				pSelectList->erase(i);
				i = pSelectList->begin();
				continue;
			}
		}
		++i;
	}
	return true;
}

bool Select::DeletePointSelectableTrackSegments(RoutePoint* pr)
{
	SelectableItemList::iterator i = pSelectList->begin();
	while (i != pSelectList->end()) {
		SelectItem* item = *i;
		if (item->m_seltype == TYPE_TRACKSEGMENT) {
			if ((RoutePoint*)item->m_pData1 == pr || (RoutePoint*)item->m_pData2 == pr) {
				delete item;
				pSelectList->erase(i);
				i = pSelectList->begin();
				continue;
			}
		}
		++i;
	}
	return true;
}

bool Select::IsSegmentSelected(float a, float b, float c, float d, float slat, float slon)
{
	double adder = 0.0;

	if ((c * d) < 0.0) {
		// Arrange for points to be increasing longitude, c to d
		double dist;
		double brg;
		geo::DistanceBearingMercator(a, c, b, d, &brg, &dist);
		if (brg < 180.0) { // swap points?
			double tmp;
			tmp = c;
			c = d;
			d = tmp;
			tmp = a;
			a = b;
			b = tmp;
		}
		if (d < 0.0) { // idl?
			d += 360.0;
			if (slon < 0.0)
				adder = 360.0;
		}
	}

	// As a course test, use segment bounding box test
	if ((slat >= (fmin(a, b) - selectRadius)) && (slat <= (fmax(a, b) + selectRadius))
		&& ((slon + adder) >= (fmin(c, d) - selectRadius))
		&& ((slon + adder) <= (fmax(c, d) + selectRadius))) {
		// Use vectors to do hit test....
		Vector2D va;
		Vector2D vb;
		Vector2D vn;

		// Assuming a Mercator projection
		double ap;
		double cp;
		geo::toSM(a, c, 0.0, 0.0, &cp, &ap);
		double bp;
		double dp;
		geo::toSM(b, d, 0.0, 0.0, &dp, &bp);
		double slatp;
		double slonp;
		geo::toSM(slat, slon + adder, 0.0, 0.0, &slonp, &slatp);

		va.x = slonp - cp;
		va.y = slatp - ap;
		vb.x = dp - cp;
		vb.y = bp - ap;

		double delta = vGetLengthOfNormal(&va, &vb, &vn);
		if (fabs(delta) < (selectRadius * 1852 * 60))
			return true;
	}
	return false;
}

void Select::CalcSelectRadius()
{
	selectRadius = pixelRadius / (cc1->GetCanvasTrueScale() * 1852 * 60);
}

SelectItem* Select::FindSelection(float slat, float slon, int fseltype)
{
	CalcSelectRadius();

	for (SelectableItemList::iterator i = pSelectList->begin(); i != pSelectList->end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype == fseltype) {
			switch (fseltype) {
				case TYPE_ROUTEPOINT:
				case TYPE_TIDEPOINT:
				case TYPE_CURRENTPOINT:
				case TYPE_AISTARGET:
					if ((fabs(slat - item->m_slat) < selectRadius)
						&& (fabs(slon - item->m_slon) < selectRadius))
						return item;
					break;

				case TYPE_ROUTESEGMENT:
				case TYPE_TRACKSEGMENT:
					if (IsSegmentSelected(item->m_slat, item->m_slat2, item->m_slon,
										  item->m_slon2, slat, slon))
						return item;
					break;

				default:
					break;
			}
		}
	}

	return NULL;
}

bool Select::IsSelectableSegmentSelected(float slat, float slon, SelectItem* item)
{
	CalcSelectRadius();
	return IsSegmentSelected(item->m_slat, item->m_slat2, item->m_slon, item->m_slon2, slat, slon);
}

SelectableItemList Select::FindSelectionList(float slat, float slon, int fseltype)
{
	SelectableItemList ret_list;

	CalcSelectRadius();
	for (SelectableItemList::iterator i = pSelectList->begin(); i != pSelectList->end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype != fseltype)
			continue;

		switch (fseltype) {
			case TYPE_ROUTEPOINT:
			case TYPE_TIDEPOINT:
			case TYPE_CURRENTPOINT:
			case TYPE_AISTARGET:
				if ((fabs(slat - item->m_slat) < selectRadius)
					&& (fabs(slon - item->m_slon) < selectRadius)) {
					ret_list.push_back(item);
				}
				break;

			case TYPE_ROUTESEGMENT:
			case TYPE_TRACKSEGMENT:
				if (IsSegmentSelected(item->m_slat, item->m_slat2, item->m_slon, item->m_slon2,
									  slat, slon))
					ret_list.push_back(item);
				break;

			default:
				break;
		}
	}

	return ret_list;
}

