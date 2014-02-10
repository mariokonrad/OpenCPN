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
#include <ChartCanvas.h>
#include <RoutePoint.h>
#include <Route.h>

#include <windows/compatibility.h>

#include <util/Vector2D.h>

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

bool Select::AddSelectableRoutePoint(const geo::Position& pos, RoutePoint* pRoutePointAdd)
{
	SelectItem* pSelItem = new SelectItem(SelectItem::TYPE_ROUTEPOINT);
	pSelItem->pos1 = pos;
	pSelItem->m_pData1 = pRoutePointAdd;

	if (pRoutePointAdd->m_bIsInLayer)
		select_items.push_back(pSelItem);
	else
		select_items.push_front(pSelItem);

	return true;
}

bool Select::AddSelectableRouteSegment(
		const geo::Position& pos1,
		const geo::Position& pos2,
		RoutePoint * pRoutePointAdd1,
		RoutePoint * pRoutePointAdd2,
		Route *pRoute)
{
	SelectItem* pSelItem = new SelectItem(SelectItem::TYPE_ROUTESEGMENT);
	pSelItem->pos1 = pos1;
	pSelItem->pos2 = pos2;
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
		if (item->type() == SelectItem::TYPE_ROUTESEGMENT) {
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
		if (item->type() == SelectItem::TYPE_ROUTEPOINT) {
			RoutePoint* ps = const_cast<RoutePoint*>(reinterpret_cast<const RoutePoint*>(item->m_pData1));

			RoutePointList::iterator j
				= std::find(pr->routepoints().begin(), pr->routepoints().end(), ps);
			if (j != pr->routepoints().end()) {
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
	if (pr->routepoints().size() == 0)
		return false;

	const RoutePointList& points = pr->routepoints();

	for (RoutePointList::const_iterator i = points.begin(); i != points.end(); ++i) {
		AddSelectableRoutePoint((*i)->get_position(), *i);
	}

	return true;
}

bool Select::AddAllSelectableRouteSegments(Route* pr)
{
	// FIXME: almost code duplication of Select::AddAllSelectableTrackSegments(Route* pr)

	if (pr->routepoints().size() == 0)
		return false;

	RoutePointList::iterator i = pr->routepoints().begin();
	RoutePoint* prp0 = *i;
	++i;
	while (i != pr->routepoints().end()) {
		RoutePoint* prp = *i;
		AddSelectableRouteSegment(prp0->get_position(), prp->get_position(), prp0, prp, pr);
		prp0 = prp;
		++i;
	}
	return true;
}

bool Select::AddAllSelectableTrackSegments(Route* pr)
{
	// FIXME: almost code duplication of Select::AddAllSelectableRouteSegments(Route* pr)

	if (pr->routepoints().size() == 0)
		return false;

	RoutePointList::iterator i = pr->routepoints().begin();
	RoutePoint* prp0 = *i;
	++i;
	while (i != pr->routepoints().end()) {
		RoutePoint* prp = *i;
		AddSelectableTrackSegment(prp0->get_position(), prp->get_position(), prp0, prp, pr);
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
		if (item->type() == SelectItem::TYPE_ROUTESEGMENT) {
			if (item->m_pData1 == prp) {
				item->pos1 = prp->get_position();
				ret = true;
			} else if (item->route_point == prp) {
				item->pos2 = prp->get_position();
				ret = true;
			}
		}
	}

	return ret;
}

void Select::AddSelectablePoint(const geo::Position& pos, const void* pdata,
								SelectItem::Type fseltype, int user_data)
{
	SelectItem* pSelItem = new SelectItem(fseltype);
	pSelItem->pos1 = pos;
	pSelItem->m_pData1 = pdata;
	pSelItem->SetUserData(user_data);
	select_items.push_back(pSelItem);
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
		if (item->type() == SeltypeToDelete) {
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
		if (item->type() == SeltypeToDelete) {
			delete item;
			select_items.erase(i);
			i = select_items.begin();
			continue;
		}
		++i;
	}
	return true;
}

bool Select::ModifySelectablePoint(const geo::Position& pos, void* data, unsigned long SeltypeToModify)
{
	// FIXME: refactor, use std algorithms
	for (SelectableItemList::iterator i = select_items.begin(); i != select_items.end(); ++i) {
		SelectItem* item = *i;
		if (item->type() == SeltypeToModify) {
			if (data == item->m_pData1) {
				item->pos1 = pos;
				return true;
			}
		}
	}
	return false;
}

bool Select::AddSelectableTrackSegment(
		const geo::Position& pos1,
		const geo::Position& pos2,
		RoutePoint * pRoutePointAdd1,
		RoutePoint * pRoutePointAdd2,
		Route * pRoute)
{
	SelectItem *pSelItem = new SelectItem(SelectItem::TYPE_TRACKSEGMENT);
	pSelItem->pos1 = pos1;
	pSelItem->pos2 = pos2;
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
		if (item->type() == SelectItem::TYPE_TRACKSEGMENT) {
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
		if (item->type() == SelectItem::TYPE_TRACKSEGMENT) {
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

bool Select::IsSegmentSelected(float a, float b, float c, float d, const geo::Position& pos)
{
	double adder = 0.0;

	if ((c * d) < 0.0) {
		// Arrange for points to be increasing longitude, c to d
		double dist;
		double brg;
		geo::DistanceBearingMercator(geo::Position(a, c), geo::Position(b, d), &brg, &dist);
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
			if (pos.lon() < 0.0)
				adder = 360.0;
		}
	}

	// As a course test, use segment bounding box test
	if ((pos.lat() >= (fmin(a, b) - selectRadius)) && (pos.lat() <= (fmax(a, b) + selectRadius))
		&& ((pos.lon() + adder) >= (fmin(c, d) - selectRadius))
		&& ((pos.lon() + adder) <= (fmax(c, d) + selectRadius))) {
		// Use vectors to do hit test....
		util::Vector2D va;
		util::Vector2D vb;
		util::Vector2D vn;

		// Assuming a Mercator projection
		double ap;
		double cp;
		geo::toSM(geo::Position(a, c), geo::Position(0.0, 0.0), &cp, &ap);
		double bp;
		double dp;
		geo::toSM(geo::Position(b, d), geo::Position(0.0, 0.0), &dp, &bp);
		double slatp;
		double slonp;
		geo::toSM(geo::Position(pos.lat(), pos.lon() + adder), geo::Position(0.0, 0.0), &slonp,
				  &slatp);

		va.x = slonp - cp;
		va.y = slatp - ap;
		vb.x = dp - cp;
		vb.y = bp - ap;

		double delta = lengthOfNormal(va, vb, vn);
		if (fabs(delta) < (selectRadius * 1852 * 60))
			return true;
	}
	return false;
}

void Select::CalcSelectRadius()
{
	selectRadius = pixelRadius / (cc1->GetCanvasTrueScale() * 1852 * 60);
}

SelectItem* Select::FindSelection(const geo::Position& pos, unsigned long fseltype)
{
	CalcSelectRadius();

	for (SelectableItemList::iterator i = select_items.begin(); i != select_items.end(); ++i) {
		SelectItem* item = *i;
		if (item->type() == fseltype) {
			switch (fseltype) {
				case SelectItem::TYPE_ROUTEPOINT:
				case SelectItem::TYPE_TIDEPOINT:
				case SelectItem::TYPE_CURRENTPOINT:
				case SelectItem::TYPE_AISTARGET:
					if ((fabs(pos.lat()- item->pos1.lat()) < selectRadius)
						&& (fabs(pos.lon()- item->pos1.lon()) < selectRadius))
						return item;
					break;

				case SelectItem::TYPE_ROUTESEGMENT:
				case SelectItem::TYPE_TRACKSEGMENT:
					if (IsSegmentSelected(item->pos1.lat(), item->pos2.lat(), item->pos1.lon(),
										  item->pos2.lon(), pos))
						return item;
					break;

				default:
					break;
			}
		}
	}

	return NULL;
}

bool Select::IsSelectableSegmentSelected(const geo::Position& pos, SelectItem* item)
{
	CalcSelectRadius();
	return IsSegmentSelected(item->pos1.lat(), item->pos2.lat(), item->pos1.lon(), item->pos2.lon(), pos);
}

SelectableItemList Select::FindSelectionList(const geo::Position& pos, unsigned long fseltype)
{
	SelectableItemList ret_list;

	CalcSelectRadius();
	for (SelectableItemList::iterator i = select_items.begin(); i != select_items.end(); ++i) {
		SelectItem* item = *i;
		if (item->type() != fseltype)
			continue;

		switch (fseltype) {
			case SelectItem::TYPE_ROUTEPOINT:
			case SelectItem::TYPE_TIDEPOINT:
			case SelectItem::TYPE_CURRENTPOINT:
			case SelectItem::TYPE_AISTARGET:
				if ((fabs(pos.lat() - item->pos1.lat()) < selectRadius)
					&& (fabs(pos.lon() - item->pos1.lon()) < selectRadius))
					ret_list.push_back(item);
				break;

			case SelectItem::TYPE_ROUTESEGMENT:
			case SelectItem::TYPE_TRACKSEGMENT:
				if (IsSegmentSelected(item->pos1.lat(), item->pos2.lat(), item->pos1.lon(),
									  item->pos2.lon(), pos))
					ret_list.push_back(item);
				break;

			default:
				break;
		}
	}

	return ret_list;
}

