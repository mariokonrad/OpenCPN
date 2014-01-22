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

#ifndef __UNDOACTION__H__
#define __UNDOACTION__H__

#include <vector>
#include <wx/string.h>

class UndoAction
{
public:
	typedef void* ItemPointer;

	enum Type {
		Undo_CreateWaypoint,
		Undo_DeleteWaypoint,
		Undo_AppendWaypoint,
		Undo_MoveWaypoint
	};

	enum BeforePointerType {
		Undo_IsOrphanded,
		Undo_NeedsCopy,
		Undo_HasParent
	};

public:
	~UndoAction();
	wxString Description();

	Type type;
	std::vector<ItemPointer> before;
	std::vector<BeforePointerType> beforeType;
	std::vector<ItemPointer> after;
	std::vector<ItemPointer> selectable;
};

#endif
