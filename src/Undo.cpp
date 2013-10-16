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
#include <Routeman.h>
#include <WayPointman.h>
#include <RouteManagerDialog.h>
#include <MarkInfo.h>
#include <Select.h>
#include <ChartCanvas.h>
#include <MainFrame.h>
#include <Config.h>

#include <tinyxml/tinyxml.h>

#include <wx/file.h>
#include <wx/datetime.h>
#include <wx/clipbrd.h>

extern Routeman * g_pRouteMan;
extern Config * pConfig;
extern Select * pSelect;
extern RouteManagerDialog * pRouteManagerDialog;
extern WayPointman * pWayPointMan;
extern ChartCanvas * cc1;
extern MainFrame * gFrame;
extern MarkInfoImpl * pMarkPropDialog;

Undo::Undo()
{
	depthSetting = 10;
	stackpointer = 0;
	isInsideUndoableAction = false;
	candidate = NULL;
}

Undo::~Undo()
{
	for( unsigned int i=0; i<undoStack.size(); i++ ) {
		if( undoStack[i] ) {
			delete undoStack[i];
			undoStack[i] = NULL;
		}
	}
	undoStack.clear();
}

void Undo::doUndoMoveWaypoint(UndoAction * action)
{
	double lat, lon;
	RoutePoint* currentPoint = (RoutePoint*) action->after[0];
	wxRealPoint* lastPoint = (wxRealPoint*) action->before[0];
	lat = currentPoint->m_lat;
	lon = currentPoint->m_lon;
	currentPoint->m_lat = lastPoint->y;
	currentPoint->m_lon = lastPoint->x;
	lastPoint->y = lat;
	lastPoint->x = lon;
	SelectItem* selectable = (SelectItem*) action->selectable[0];
	selectable->m_slat = currentPoint->m_lat;
	selectable->m_slon = currentPoint->m_lon;

	if ((NULL != pMarkPropDialog) && (pMarkPropDialog->IsShown())) {
		if (currentPoint == pMarkPropDialog->GetRoutePoint())
			pMarkPropDialog->UpdateProperties(true);
	}

	wxArrayPtrVoid* routeArray = g_pRouteMan->GetRouteArrayContaining( currentPoint );
	if (routeArray) {
		for (unsigned int ir = 0; ir < routeArray->GetCount(); ir++ ) {
			Route * route = static_cast<Route *>(routeArray->Item(ir));
			route->CalculateBBox();
			route->UpdateSegmentDistances();
			pConfig->UpdateRoute(route);
		}
		delete routeArray;
	}
}

void Undo::doUndoDeleteWaypoint(UndoAction * action)
{
	RoutePoint* point = (RoutePoint*) action->before[0];
	pSelect->AddSelectableRoutePoint( point->m_lat, point->m_lon, point );
	pConfig->AddNewWayPoint( point, -1 );
	if (NULL != pWayPointMan)
		pWayPointMan->push_back(point);
	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateWptListCtrl();
}

void Undo::doRedoDeleteWaypoint(UndoAction * action)
{
	RoutePoint* point = (RoutePoint*) action->before[0];
	pConfig->DeleteWayPoint( point );
	pSelect->DeleteSelectablePoint( point, Select::TYPE_ROUTEPOINT );
	if (NULL != pWayPointMan)
		pWayPointMan->remove(point);
	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateWptListCtrl();
}

void Undo::doUndoAppendWaypoint(UndoAction * action)
{
	RoutePoint* point = (RoutePoint*) action->before[0];
	Route* route = (Route*) action->after[0];

	bool noRouteLeftToRedo = false;
	if( (route->GetnPoints() == 2) && (gFrame->nRoute_State == 0) )
		noRouteLeftToRedo = true;

	cc1->RemovePointFromRoute( point, route );

	if( action->beforeType[0] == UndoAction::Undo_IsOrphanded ) {
		pConfig->DeleteWayPoint( point );
		pSelect->DeleteSelectablePoint( point, Select::TYPE_ROUTEPOINT );
		if (NULL != pWayPointMan)
			pWayPointMan->remove(point);
	}

	if( noRouteLeftToRedo ) {
		cc1->undo->InvalidateRedo();
	}

	if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
		pRouteManagerDialog->UpdateWptListCtrl();

	if( gFrame->nRoute_State > 1 ) {
		gFrame->nRoute_State--;
		cc1->m_prev_pMousePoint = route->GetLastPoint();
		cc1->m_prev_rlat = cc1->m_prev_pMousePoint->m_lat;
		cc1->m_prev_rlon = cc1->m_prev_pMousePoint->m_lon;
		route->m_lastMousePointIndex = route->GetnPoints();
	}
}

void Undo::doRedoAppendWaypoint(UndoAction * action)
{
	RoutePoint* point = (RoutePoint*) action->before[0];
	Route* route = (Route*) action->after[0];

	if( action->beforeType[0] == UndoAction::Undo_IsOrphanded ) {
		pConfig->AddNewWayPoint( point, -1 );
		pSelect->AddSelectableRoutePoint( point->m_lat, point->m_lon, point );
	}

	RoutePoint* prevpoint = route->GetLastPoint();

	route->AddPoint( point );
	pSelect->AddSelectableRouteSegment( prevpoint->m_lat, prevpoint->m_lon,
			point->m_lat, point->m_lon, prevpoint, point, route );

	if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();

	if( gFrame->nRoute_State > 1 ) {
		gFrame->nRoute_State++;
		cc1->m_prev_pMousePoint = route->GetLastPoint();
		cc1->m_prev_rlat = cc1->m_prev_pMousePoint->m_lat;
		cc1->m_prev_rlon = cc1->m_prev_pMousePoint->m_lon;
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
	if( stackpointer == 0 ) return;

	// Make sure we are not deleting any objects pointed to by
	// potential redo actions.

	for( unsigned int i=0; i<stackpointer; i++ ) {
		switch( undoStack[i]->type ) {
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

	undoStack.erase( undoStack.begin(), undoStack.begin() + stackpointer );
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

	switch( action->type ){

		case UndoAction::Undo_CreateWaypoint:
			doRedoDeleteWaypoint( action ); // Same as delete but reversed.
			stackpointer++;
			break;

		case UndoAction::Undo_MoveWaypoint:
			doUndoMoveWaypoint( action );
			stackpointer++;
			break;

		case UndoAction::Undo_DeleteWaypoint:
			doUndoDeleteWaypoint( action );
			stackpointer++;
			break;

		case UndoAction::Undo_AppendWaypoint:
			stackpointer++;
			doUndoAppendWaypoint( action );
			break;
	}
	return true;
}

bool Undo::RedoNextAction()
{
	if (!AnythingToRedo())
		return false;
	UndoAction* action = GetNextRedoableAction();

	switch( action->type ){

		case UndoAction::Undo_CreateWaypoint:
			doUndoDeleteWaypoint( action ); // Same as delete but reversed.
			stackpointer--;
			break;

		case UndoAction::Undo_MoveWaypoint:
			doUndoMoveWaypoint( action ); // For Wpt move, redo is same as undo (swap lat/long);
			stackpointer--;
			break;

		case UndoAction::Undo_DeleteWaypoint:
			doRedoDeleteWaypoint( action );
			stackpointer--;
			break;

		case UndoAction::Undo_AppendWaypoint:
			doRedoAppendWaypoint( action );
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
			switch( candidate->type ) {
				case UndoAction::Undo_MoveWaypoint: {
					wxRealPoint* point = new wxRealPoint;
					RoutePoint* rp = (RoutePoint*) before;
					point->x = rp->m_lon;
					point->y = rp->m_lat;
					subject = point;
					break;
					}
				case UndoAction::Undo_CreateWaypoint: break;
				case UndoAction::Undo_DeleteWaypoint: break;
				case UndoAction::Undo_AppendWaypoint: break;
			}
			break;
		case UndoAction::Undo_IsOrphanded: break;
		case UndoAction::Undo_HasParent: break;
	}

	candidate->before.push_back( subject );
	candidate->beforeType.push_back( beforeType );
	candidate->selectable.push_back( selectable );

	isInsideUndoableAction = true;
	return true;
}

bool Undo::AfterUndoableAction(UndoAction::ItemPointer after)
{
	if( !isInsideUndoableAction ) return false;

	candidate->after.push_back( after );
	undoStack.push_front( candidate );

	if( undoStack.size() > depthSetting ) {
		undoStack.pop_back();
	}

	isInsideUndoableAction = false;
	return true;
}

bool Undo::CancelUndoableAction(bool noDataDelete)
{
	if( isInsideUndoableAction ) {
		if( noDataDelete ) {
			for( unsigned int i = 0; i < candidate->beforeType.size(); i++ ) {
				if( candidate->beforeType[i] == UndoAction::Undo_IsOrphanded ) {
					candidate->beforeType[i] = UndoAction::Undo_HasParent;
				}
			}
		}
		if( candidate ) delete candidate;
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

