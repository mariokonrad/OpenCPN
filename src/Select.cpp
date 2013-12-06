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

#include <algorithm>

extern ChartCanvas* cc1;

Select::Select()
	: pixelRadius(8)
{
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
}

void Select::SetSelectPixelRadius(int radius)
{
	pixelRadius = radius;
}

bool Select::AddSelectableRoutePoint(const Position& pos, RoutePoint* pRoutePointAdd)
{
	SelectItem* pSelItem = new SelectItem;
	pSelItem->m_slat = pos.lat();
	pSelItem->m_slon = pos.lon();
	pSelItem->m_seltype = SelectItem::TYPE_ROUTEPOINT;
	pSelItem->m_bIsSelected = false;
	pSelItem->m_pData1 = pRoutePointAdd;

	if (pRoutePointAdd->m_bIsInLayer)
		select_items.push_back(pSelItem);
	else
		select_items.push_front(pSelItem);

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
	pSelItem->m_seltype = SelectItem::TYPE_ROUTESEGMENT;
	pSelItem->m_bIsSelected = false;
	pSelItem->m_pData1 = pRoutePointAdd1;
	pSelItem->route_point = pRoutePointAdd2;
	pSelItem->route = pRoute;

	if (pRoute->m_bIsInLayer)
		select_items.push_back(pSelItem);
	else
		select_items.push_front(pSelItem);

	return true;
}

bool Select::DeleteAllSelectableRouteSegments(Route* pr)
{
	// FIXME: refactor, use std algorithms
	SelectableItemList::iterator i = select_items.begin();
	while (i != select_items.end()) {
		SelectItem* item = *i;
		if (item->m_seltype == SelectItem::TYPE_ROUTESEGMENT) {
			if (item->route == pr) {
				delete item;
				select_items.erase(i);
				i = select_items.begin();
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
	SelectableItemList::iterator i = select_items.begin();
	while (i != select_items.end()) {
		SelectItem* item = *i;
		if (item->m_seltype == SelectItem::TYPE_ROUTEPOINT) {
			RoutePoint* ps = const_cast<RoutePoint*>(reinterpret_cast<const RoutePoint*>(item->m_pData1));

			RoutePointList::iterator j
				= std::find(pr->pRoutePointList->begin(), pr->pRoutePointList->end(), ps);
			if (j != pr->pRoutePointList->end()) {
				delete item;
				select_items.erase(i);
				i = select_items.begin();
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

	RoutePointList* points = pr->pRoutePointList;

	for (RoutePointList::iterator i = points->begin(); i != points->end(); ++i) {
		AddSelectableRoutePoint((*i)->get_position(), *i);
	}

	return true;
}

bool Select::AddAllSelectableRouteSegments(Route* pr)
{
	// FIXME: almost code duplication of Select::AddAllSelectableTrackSegments(Route* pr)

	if (pr->pRoutePointList->size() == 0)
		return false;

	RoutePointList::iterator i = pr->pRoutePointList->begin();

	RoutePoint* prp0 = *i;
	double slat1 = prp0->latitude();
	double slon1 = prp0->longitude();

	++i;
	while (i != pr->pRoutePointList->end()) {
		RoutePoint* prp = *i;
		double slat2 = prp->latitude();
		double slon2 = prp->longitude();

		AddSelectableRouteSegment(slat1, slon1, slat2, slon2, prp0, prp, pr);

		slat1 = slat2;
		slon1 = slon2;
		prp0 = prp;

		++i;
	}
	return true;
}

bool Select::AddAllSelectableTrackSegments(Route* pr)
{
	// FIXME: almost code duplication of Select::AddAllSelectableRouteSegments(Route* pr)

	if (pr->pRoutePointList->size() == 0)
		return false;

	RoutePointList::iterator i = pr->pRoutePointList->begin();

	RoutePoint* prp0 = *i;
	double slat1 = prp0->latitude();
	double slon1 = prp0->longitude();

	++i;
	while (i != pr->pRoutePointList->end()) {
		RoutePoint* prp = *i;
		double slat2 = prp->latitude();
		double slon2 = prp->longitude();

		AddSelectableTrackSegment(slat1, slon1, slat2, slon2, prp0, prp, pr);

		slat1 = slat2;
		slon1 = slon2;
		prp0 = prp;

		++i;
	}
	return true;
}

bool Select::UpdateSelectableRouteSegments(const RoutePoint* prp)
{
	bool ret = false;

	for (SelectableItemList::iterator i = select_items.begin(); i != select_items.end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype == SelectItem::TYPE_ROUTESEGMENT) {
			if (item->m_pData1 == prp) {
				item->m_slat = prp->latitude();
				item->m_slon = prp->longitude();
				ret = true;
			} else if (item->route_point == prp) {
				item->m_slat2 = prp->latitude();
				item->m_slon2 = prp->longitude();
				ret = true;
			}
		}
	}

	return ret;
}

SelectItem* Select::AddSelectablePoint(const Position& pos, const void* pdata,
									   SelectItem::Type fseltype)
{
	SelectItem* pSelItem = new SelectItem;
	if (pSelItem) {
		pSelItem->m_slat = pos.lat();
		pSelItem->m_slon = pos.lon();
		pSelItem->m_seltype = fseltype;
		pSelItem->m_bIsSelected = false;
		pSelItem->m_pData1 = pdata;
		select_items.push_back(pSelItem);
	}
	return pSelItem;
}

bool Select::DeleteAllPoints(void)
{
	for (SelectableItemList::iterator i = select_items.begin(); i != select_items.end(); ++i)
		delete *i;
	select_items.clear();
	return true;
}

bool Select::DeleteSelectablePoint(void* pdata, unsigned long SeltypeToDelete)
{
	if (NULL == pdata)
		return false;

	// FIXME: refactor, use std algorithms
	for (SelectableItemList::iterator i = select_items.begin(); i != select_items.end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype == SeltypeToDelete) {
			if (pdata == item->m_pData1) {
				delete item;
				select_items.erase(i);
				return true;
			}
		}
	}
	return false;
}

bool Select::DeleteAllSelectableTypePoints(unsigned long SeltypeToDelete)
{
	// FIXME: refactor, use std algorithms
	SelectableItemList::iterator i = select_items.begin();
	while (i != select_items.end()) {
		SelectItem* item = *i;
		if (item->m_seltype == SeltypeToDelete) {
			delete item;
			select_items.erase(i);
			i = select_items.begin();
			continue;
		}
		++i;
	}
	return true;
}

bool Select::ModifySelectablePoint(const Position& pos, void* data, unsigned long SeltypeToModify)
{
	// FIXME: refactor, use std algorithms
	for (SelectableItemList::iterator i = select_items.begin(); i != select_items.end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype == SeltypeToModify) {
			if (data == item->m_pData1) {
				item->m_slat = pos.lat();
				item->m_slon = pos.lon();
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
	pSelItem->m_seltype = SelectItem::TYPE_TRACKSEGMENT;
	pSelItem->m_bIsSelected = false;
	pSelItem->m_pData1 = pRoutePointAdd1;
	pSelItem->route_point = pRoutePointAdd2;
	pSelItem->route = pRoute;

	if (pRoute->m_bIsInLayer)
		select_items.push_back(pSelItem);
	else
		select_items.push_front(pSelItem);

	return true;
}

bool Select::DeleteAllSelectableTrackSegments(Route* pr)
{
	// FIXME: refactor, use std algorithms
	SelectableItemList::iterator i = select_items.begin();
	while (i != select_items.end()) {
		SelectItem* item = *i;
		if (item->m_seltype == SelectItem::TYPE_TRACKSEGMENT) {
			if (item->route == pr) {
				delete item;
				select_items.erase(i);
				i = select_items.begin();
				continue;
			}
		}
		++i;
	}
	return true;
}

bool Select::DeletePointSelectableTrackSegments(RoutePoint* pr)
{
	SelectableItemList::iterator i = select_items.begin();
	while (i != select_items.end()) {
		SelectItem* item = *i;
		if (item->m_seltype == SelectItem::TYPE_TRACKSEGMENT) {
			if ((RoutePoint*)item->m_pData1 == pr || item->route_point == pr) {
				delete item;
				select_items.erase(i);
				i = select_items.begin();
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

SelectItem* Select::FindSelection(const Position& pos, unsigned long fseltype)
{
	CalcSelectRadius();

	for (SelectableItemList::iterator i = select_items.begin(); i != select_items.end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype == fseltype) {
			switch (fseltype) {
				case SelectItem::TYPE_ROUTEPOINT:
				case SelectItem::TYPE_TIDEPOINT:
				case SelectItem::TYPE_CURRENTPOINT:
				case SelectItem::TYPE_AISTARGET:
					if ((fabs(pos.lat()- item->m_slat) < selectRadius)
						&& (fabs(pos.lon()- item->m_slon) < selectRadius))
						return item;
					break;

				case SelectItem::TYPE_ROUTESEGMENT:
				case SelectItem::TYPE_TRACKSEGMENT:
					if (IsSegmentSelected(item->m_slat, item->m_slat2, item->m_slon,
										  item->m_slon2, pos.lat(), pos.lon()))
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

SelectableItemList Select::FindSelectionList(const Position& pos, unsigned long fseltype)
{
	SelectableItemList ret_list;

	CalcSelectRadius();
	for (SelectableItemList::iterator i = select_items.begin(); i != select_items.end(); ++i) {
		SelectItem* item = *i;
		if (item->m_seltype != fseltype)
			continue;

		switch (fseltype) {
			case SelectItem::TYPE_ROUTEPOINT:
			case SelectItem::TYPE_TIDEPOINT:
			case SelectItem::TYPE_CURRENTPOINT:
			case SelectItem::TYPE_AISTARGET:
				if ((fabs(pos.lat() - item->m_slat) < selectRadius)
					&& (fabs(pos.lon() - item->m_slon) < selectRadius))
					ret_list.push_back(item);
				break;

			case SelectItem::TYPE_ROUTESEGMENT:
			case SelectItem::TYPE_TRACKSEGMENT:
				if (IsSegmentSelected(item->m_slat, item->m_slat2, item->m_slon, item->m_slon2,
									  pos.lat(), pos.lon()))
					ret_list.push_back(item);
				break;

			default:
				break;
		}
	}

	return ret_list;
}

