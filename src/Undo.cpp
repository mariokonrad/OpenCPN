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

#include "Undo.h"
#include <RouteManagerDialog.h>
#include <MarkInfo.h>
#include <Select.h>
#include <ChartCanvas.h>
#include <MainFrame.h>
#include <Config.h>

#include <global/OCPN.h>

#include <navigation/RouteManager.h>
#include <navigation/WaypointManager.h>

#include <wx/file.h>
#include <wx/datetime.h>
#include <wx/clipbrd.h>

extern Config* pConfig;
extern Select* pSelect;
extern RouteManagerDialog* pRouteManagerDialog;
extern ChartCanvas* cc1;
extern MainFrame* gFrame;
extern MarkInfoImpl* pMarkPropDialog;

Undo::Undo()
{
	depthSetting = 10;
	stackpointer = 0;
	isInsideUndoableAction = false;
	candidate = NULL;
}

Undo::~Undo()
{
	for (unsigned int i = 0; i < undoStack.size(); ++i) {
		if (undoStack[i]) {
			delete undoStack[i];
			undoStack[i] = NULL;
		}
	}
	undoStack.clear();
}

void Undo::doUndoMoveWaypoint(UndoAction* action)
{
	RoutePoint* currentPoint = reinterpret_cast<RoutePoint*>(action->after[0]);
	wxRealPoint* lastPoint = reinterpret_cast<wxRealPoint*>(action->before[0]);
	double lat = currentPoint->latitude();
	double lon = currentPoint->longitude();
	currentPoint->set_position(geo::Position(lastPoint->y, lastPoint->x));
	lastPoint->y = lat;
	lastPoint->x = lon;
	SelectItem* selectable = reinterpret_cast<SelectItem*>(action->selectable[0]);
	selectable->pos1 = currentPoint->get_position();

	if ((NULL != pMarkPropDialog) && (pMarkPropDialog->IsShown())) {
		if (currentPoint == pMarkPropDialog->GetRoutePoint())
			pMarkPropDialog->UpdateProperties(true);
	}

	using navigation::RouteManager;
	RouteManager::RouteArray routes
		= global::OCPN::get().routeman().GetRouteArrayContaining(currentPoint);
	for (RouteManager::RouteArray::iterator i = routes.begin(); i != routes.end(); ++i) {
		Route* route = *i;
		route->CalculateBBox();
		route->UpdateSegmentDistances();
		pConfig->UpdateRoute(route);
	}
}

void Undo::doUndoDeleteWaypoint(UndoAction * action)
{
	RoutePoint* point = reinterpret_cast<RoutePoint*>(action->before[0]);
	pSelect->AddSelectableRoutePoint(point->get_position(), point);
	pConfig->AddNewWayPoint(point, -1);
	global::OCPN::get().waypointman().push_back(point);
	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateWptListCtrl();
}

void Undo::doRedoDeleteWaypoint(UndoAction * action)
{
	RoutePoint* point = reinterpret_cast<RoutePoint*>(action->before[0]);
	pConfig->DeleteWayPoint(point);
	pSelect->DeleteSelectablePoint(point, SelectItem::TYPE_ROUTEPOINT);
	global::OCPN::get().waypointman().remove(point);
	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateWptListCtrl();
}

void Undo::doUndoAppendWaypoint(UndoAction* action)
{
	RoutePoint* point = reinterpret_cast<RoutePoint*>(action->before[0]);
	Route* route = reinterpret_cast<Route*>(action->after[0]);

	bool noRouteLeftToRedo = false;
	if ((route->GetnPoints() == 2) && (gFrame->nRoute_State == 0))
		noRouteLeftToRedo = true;

	cc1->RemovePointFromRoute(point, route);

	if (action->beforeType[0] == UndoAction::Undo_IsOrphanded) {
		pConfig->DeleteWayPoint(point);
		pSelect->DeleteSelectablePoint(point, SelectItem::TYPE_ROUTEPOINT);
		global::OCPN::get().waypointman().remove(point);
	}

	if (noRouteLeftToRedo) {
		cc1->invalidate_redo();
	}

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateWptListCtrl();

	if (gFrame->nRoute_State > 1) {
		gFrame->nRoute_State--;
		cc1->set_prev_mouse_point(route->GetLastPoint());
		route->m_lastMousePointIndex = route->GetnPoints();
	}
}

void Undo::doRedoAppendWaypoint(UndoAction* action)
{
	RoutePoint* point = reinterpret_cast<RoutePoint*>(action->before[0]);
	Route* route = reinterpret_cast<Route*>(action->after[0]);

	if (action->beforeType[0] == UndoAction::Undo_IsOrphanded) {
		pConfig->AddNewWayPoint(point, -1);
		pSelect->AddSelectableRoutePoint(point->get_position(), point);
	}

	RoutePoint* prevpoint = route->GetLastPoint();

	route->AddPoint(point);
	pSelect->AddSelectableRouteSegment(prevpoint->get_position(), point->get_position(), prevpoint,
									   point, route);

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateWptListCtrl();

	if (gFrame->nRoute_State > 1) {
		gFrame->nRoute_State++;
		cc1->set_prev_mouse_point(route->GetLastPoint());
		route->m_lastMousePointIndex = route->GetnPoints();
	}
}

bool Undo::AnythingToUndo() const
{
	return undoStack.size() > stackpointer;
}

bool Undo::AnythingToRedo() const
{
	return stackpointer > 0;
}

UndoAction * Undo::GetNextUndoableAction()
{
	return undoStack[stackpointer];
}

UndoAction * Undo::GetNextRedoableAction()
{
	return undoStack[stackpointer-1];
}

void Undo::InvalidateRedo()
{
	if (stackpointer == 0)
		return;

	// Make sure we are not deleting any objects pointed to by
	// potential redo actions.

	for (unsigned int i = 0; i < stackpointer; i++) {
		switch (undoStack[i]->type) {
			case UndoAction::Undo_DeleteWaypoint:
				undoStack[i]->before[0] = NULL;
				break;
			case UndoAction::Undo_CreateWaypoint:
			case UndoAction::Undo_MoveWaypoint:
			case UndoAction::Undo_AppendWaypoint:
				break;
		}
		delete undoStack[i];
	}

	undoStack.erase(undoStack.begin(), undoStack.begin() + stackpointer);
	stackpointer = 0;
}

void Undo::InvalidateUndo()
{
	undoStack.clear();
	stackpointer = 0;
}

bool Undo::UndoLastAction()
{
	if (!AnythingToUndo())
		return false;
	UndoAction* action = GetNextUndoableAction();

	switch (action->type) {

		case UndoAction::Undo_CreateWaypoint:
			doRedoDeleteWaypoint(action); // Same as delete but reversed.
			stackpointer++;
			break;

		case UndoAction::Undo_MoveWaypoint:
			doUndoMoveWaypoint(action);
			stackpointer++;
			break;

		case UndoAction::Undo_DeleteWaypoint:
			doUndoDeleteWaypoint(action);
			stackpointer++;
			break;

		case UndoAction::Undo_AppendWaypoint:
			stackpointer++;
			doUndoAppendWaypoint(action);
			break;
	}
	return true;
}

bool Undo::RedoNextAction()
{
	if (!AnythingToRedo())
		return false;
	UndoAction* action = GetNextRedoableAction();

	switch (action->type) {

		case UndoAction::Undo_CreateWaypoint:
			doUndoDeleteWaypoint(action); // Same as delete but reversed.
			stackpointer--;
			break;

		case UndoAction::Undo_MoveWaypoint:
			doUndoMoveWaypoint(action); // For Wpt move, redo is same as undo (swap lat/long);
			stackpointer--;
			break;

		case UndoAction::Undo_DeleteWaypoint:
			doRedoDeleteWaypoint(action);
			stackpointer--;
			break;

		case UndoAction::Undo_AppendWaypoint:
			doRedoAppendWaypoint(action);
			stackpointer--;
			break;
	}
	return true;
}

bool Undo::BeforeUndoableAction(
		UndoAction::Type type,
		UndoAction::ItemPointer before,
		UndoAction::BeforePointerType beforeType,
		UndoAction::ItemPointer selectable)
{
	if (CancelUndoableAction())
		return false;
	InvalidateRedo();

	candidate = new UndoAction;
	candidate->before.clear();
	candidate->beforeType.clear();
	candidate->selectable.clear();
	candidate->after.clear();

	candidate->type = type;
	UndoAction::ItemPointer subject = before;

	switch (beforeType) {
		case UndoAction::Undo_NeedsCopy:
			switch (candidate->type) {
				case UndoAction::Undo_MoveWaypoint: {
					wxRealPoint* point = new wxRealPoint;
					RoutePoint* rp = (RoutePoint*)before;
					point->x = rp->longitude();
					point->y = rp->latitude();
					subject = point;
					break;
				}
				case UndoAction::Undo_CreateWaypoint:
					break;
				case UndoAction::Undo_DeleteWaypoint:
					break;
				case UndoAction::Undo_AppendWaypoint:
					break;
			}
			break;
		case UndoAction::Undo_IsOrphanded:
			break;
		case UndoAction::Undo_HasParent:
			break;
	}

	candidate->before.push_back(subject);
	candidate->beforeType.push_back(beforeType);
	candidate->selectable.push_back(selectable);

	isInsideUndoableAction = true;
	return true;
}

bool Undo::AfterUndoableAction(UndoAction::ItemPointer after)
{
	if (!isInsideUndoableAction)
		return false;

	candidate->after.push_back(after);
	undoStack.push_front(candidate);

	if (undoStack.size() > depthSetting) {
		undoStack.pop_back();
	}

	isInsideUndoableAction = false;
	return true;
}

bool Undo::CancelUndoableAction(bool noDataDelete)
{
	if (isInsideUndoableAction) {
		if (noDataDelete) {
			for (unsigned int i = 0; i < candidate->beforeType.size(); i++) {
				if (candidate->beforeType[i] == UndoAction::Undo_IsOrphanded) {
					candidate->beforeType[i] = UndoAction::Undo_HasParent;
				}
			}
		}
		if (candidate)
			delete candidate;
		candidate = NULL;
		isInsideUndoableAction = false;
		return true;
	}
	return false;
}

bool Undo::InUndoableAction() const
{
	return isInsideUndoableAction;
}

