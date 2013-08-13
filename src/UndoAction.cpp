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

#include "UndoAction.h"
#include "RoutePoint.h"
#include <wx/gdicmn.h>

UndoAction::~UndoAction()
{
	assert(before.size() == beforeType.size());

	for( unsigned int i = 0; i < before.size(); i++ ) {
		switch (beforeType[i]) {
			case Undo_NeedsCopy:
				switch (type) {
					case Undo_MoveWaypoint:
						if( before[i] ) {
							delete (wxRealPoint*) before[i];
							before[i] = NULL;
						}
						break;
					case Undo_DeleteWaypoint: break;
					case Undo_CreateWaypoint: break;
					case Undo_AppendWaypoint: break;
				}
				break;

			case Undo_IsOrphanded:
				switch (type) {
					case Undo_DeleteWaypoint:
						if (before[i]) {
							delete (RoutePoint*) before[i];
						}
						break;
					case Undo_CreateWaypoint: break;
					case Undo_MoveWaypoint: break;
					case Undo_AppendWaypoint:
						if (before[i]) {
							delete (RoutePoint*) before[i];
							before[i] = NULL;
						}
						break;
				}
				break;

			case Undo_HasParent: break;
		}
	}
	before.clear();
}

wxString UndoAction::Description()
{
	wxString descr;
	switch( type ){
		case Undo_CreateWaypoint:
			descr = _("Create Waypoint");
			break;
		case Undo_DeleteWaypoint:
			descr = _("Delete Waypoint");
			break;
		case Undo_MoveWaypoint:
			descr = _("Move Waypoint");
			break;
		case Undo_AppendWaypoint:
			descr = _("Append Waypoint");
			break;
		default:
			descr = _T("");
			break;
	}
	return descr;
}

